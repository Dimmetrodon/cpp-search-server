#include "search_server.h"
#include "string_processing.h"
#include "log_duration.h"
#include <cmath>

using namespace std;

//Конструктор для string стоп-слов
SearchServer::SearchServer(const string& stop_words)
{
    vector<string> stop_words_vector = SplitIntoWords(stop_words);
    for (const string& word : stop_words_vector)
    {
        if (!IsValidString(word))
        {
            throw invalid_argument("Forbidden symbols entered");
        }
    }
    for (const string& word : stop_words_vector)
    {
        SearchServer::stop_words_.insert(word);
    }
}

SearchServer::SearchServer(string_view stop_words)
{
    vector<string_view> stop_words_vector = SplitIntoWordsView(stop_words);
    for (string_view word : stop_words_vector)
    {
        if (!IsValidString(word))
        {
            throw invalid_argument("Forbidden symbols entered");
        }
    }
    for (string_view word : stop_words_vector)
    {
        SearchServer::stop_words_.insert(string(word));
    }
}

void SearchServer::SetStopWords(const string& text)
{
    for (const string& word : SplitIntoWords(text))
    {
        SearchServer::stop_words_.insert(word);
    }
}

void SearchServer::AddDocument(int document_id, const string_view document, DocumentStatus status, const vector<int>& ratings)
{
    if (documents_.count(document_id) == 1)
    {
        throw invalid_argument("id is taken");
    }
    else if (document_id < 0)
    {
        throw invalid_argument("id value should not be less than 0");
    }
    else if (!SearchServer::IsValidString(document))
    {
        throw invalid_argument("Forbidden symbols");
    }

    const auto [it, inserted] = documents_.emplace(document_id,
        DocumentData
        {
            ComputeAverageRating(ratings),
            status,
            string(document)
        });

    const vector<string_view> words = SearchServer::SplitIntoWordsNoStop(it->second.raw_text);
    const double inv_word_count = 1.0 / words.size();
    for (string_view word : words)
    {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        ids_to_word_to_freqs[document_id][word] += inv_word_count;
        ids_to_words_[document_id].insert(string(word));
    }
    documents_ids_.insert(document_id);
    
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query, DocumentStatus check_status) const
{
    return SearchServer::FindTopDocuments(raw_query, [check_status](int document_id, DocumentStatus status, int rating) { return status == check_status; });
}
std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query) const
{
    return SearchServer::FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

//Возвращает длину documents_
int SearchServer::GetDocumentCount() const
{
    return documents_.size();
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(string_view raw_query, int document_id) const
{
    const Query query = ParseQuery(raw_query);
    vector<string_view> matched_words;

    for (string_view word : query.plus_words)
    {
        if (word_to_document_freqs_.count(word) == 0)
        {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id))
        {
            matched_words.push_back(word);
        }
    }

    for (string_view word : query.minus_words)
    {
        if (word_to_document_freqs_.count(word) == 0)
        {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id))
        {
            matched_words.clear();
            break;
        }
    }

    return tuple{ matched_words, SearchServer::documents_.at(document_id).status };
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(std::execution::sequenced_policy policy, string_view raw_query, int document_id) const
{
    return MatchDocument(raw_query, document_id);
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(std::execution::parallel_policy policy, string_view raw_query, int document_id) const
{
    Query query = ParseQuery(raw_query);
    DocumentStatus status = documents_.at(document_id).status;

    vector<string_view> matched_words(query.plus_words.size());
    if (any_of(
        query.minus_words.begin(), query.minus_words.end(),
        [document_id, &itwtf = ids_to_word_to_freqs](const auto& word)
        {
            return itwtf.at(document_id).count(word);
        }))
    {
        return { {}, documents_.at(document_id).status };
    }

        auto it_end = copy_if(
            query.plus_words.begin(), query.plus_words.end(),
            matched_words.begin(),
            [&](string_view word)
            {
                return ids_to_word_to_freqs.at(document_id).count(word);
            });

        sort(matched_words.begin(), it_end);
        auto it_end1 = unique(matched_words.begin(), it_end);
        matched_words.erase(it_end1, matched_words.end());
        return { matched_words, status };
}


std::set<int>::const_iterator SearchServer::begin() const
{
    return documents_ids_.begin();
}

std::set<int>::const_iterator SearchServer::end() const
{
    return documents_ids_.end();
}

const map<string_view, double>& SearchServer::GetWordFrequencies(int document_id) const
{
    if (find(this->begin(), this->end(), document_id) == this->end())
    {
        static const map<string_view, double> empty{};
        return empty;
    }
    return ids_to_word_to_freqs.at(document_id);
}

void SearchServer::RemoveDocument(int document_id)
{
    for (auto& [word, TF] : ids_to_word_to_freqs.at(document_id))
    {
        if (word_to_document_freqs_.at(word).size() > 1)
        {
            auto pos = word_to_document_freqs_.at(word).find(document_id);
            word_to_document_freqs_.at(word).erase(pos);
        }
        else
        {
            word_to_document_freqs_.erase(word);
        }
    }

    ids_to_word_to_freqs.erase(document_id);
    documents_ids_.erase(find(documents_ids_.begin(), documents_ids_.end(), document_id));
    documents_.erase(document_id);
    ids_to_words_.erase(document_id);
}

void SearchServer::RemoveDocument(std::execution::parallel_policy policy, int document_id)
{
    if (ids_to_word_to_freqs.count(document_id) == 0) {
        return;
    }

    /* { {word, TF}, {word, TF}, ... } */
    std::vector< std::pair<std::string, double> > words_and_it_freqs((ids_to_word_to_freqs.at(document_id)).size());
    copy(policy,
        (ids_to_word_to_freqs.at(document_id)).begin(), (ids_to_word_to_freqs.at(document_id)).end(),
        words_and_it_freqs.begin());

    std::for_each(
        policy,
        words_and_it_freqs.begin(),
        words_and_it_freqs.end(),
        [&](const std::pair<std::string, double>& word_freq) {
            word_to_document_freqs_[word_freq.first].erase(document_id);
        }
    );

    ids_to_word_to_freqs.erase(document_id);
    documents_ids_.erase(find(documents_ids_.begin(), documents_ids_.end(), document_id));
    ids_to_words_.erase(document_id);
    documents_.erase(document_id);
}

void SearchServer::RemoveDocument(std::execution::sequenced_policy policy, int document_id)
{
    RemoveDocument(document_id);
}

bool SearchServer::IsValidString(string_view text)
{
    return (none_of(text.begin(), text.end(), [](char x)
        {
            if ((x > 0) && (x < 32))
                return true;
            return false;
        }));
}

bool SearchServer::IsStopWord(string_view word) const
{
    return stop_words_.count(string(word)) > 0;
}

vector<string_view> SearchServer::SplitIntoWordsNoStop(string_view text) const
{
    vector<string_view> words;
    for (string_view word : SplitIntoWordsView(text))
    {
        if (!IsStopWord(word))
        {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings)
{
    if (ratings.empty())
    {
        return 0;
    }
    int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string_view word) const
{
    if (!IsValidString(word))
    {
        throw invalid_argument("Forbidden symbols");
    }

    bool is_minus = false;
    // Word shouldn't be empty
    if (word[0] == '-')
    {
        is_minus = true;
        word = word.substr(1);
        if (word.empty())
        {
            throw invalid_argument("No text after minus");
        }
        if (word[0] == '-')
        {
            throw invalid_argument("Multiple minuses");
        }
    }
    return
    {
        word,
        is_minus,
        IsStopWord(word)
    };
}

SearchServer::Query SearchServer::ParseQuery(string_view text) const
{
    if (!IsValidString(text))
    {
        throw invalid_argument("Forbidden symbols");
    }

    Query query;
    vector<string_view> split_words = SplitIntoWordsView(text);

    for (string_view word : split_words)
    {
        if (!IsStopWord(word))
        {
            if (word[0] == '-')
            {
                word = word.substr(1);
                if (word.empty())
                {
                    throw invalid_argument("No text after minus");
                }
                if (word[0] == '-')
                {
                    throw invalid_argument("Multiple minuses");
                }
                query.minus_words.push_back(word);
                continue;
            }
            query.plus_words.push_back(word);
        }
    }


    sort(query.plus_words.begin(), query.plus_words.end());
    auto plus_words_end = unique(query.plus_words.begin(), query.plus_words.end());
    query.plus_words.resize(distance(query.plus_words.begin(), plus_words_end));

    return query;
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const
{
    return log(SearchServer::GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

ostream& operator<<(ostream& out, const Document doc)
{
    out << "{ document_id = "s << doc.id << ", relevance = "s << doc.relevance << ", rating = "s << doc.rating << " }"s;
    return out;
}
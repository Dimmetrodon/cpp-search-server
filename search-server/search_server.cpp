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

void SearchServer::SetStopWords(const string & text)
{
    for (const string& word : SplitIntoWords(text))
    {
        SearchServer::stop_words_.insert(word);
    }
}

//Добавляет в documents_ id, средний рейтинг, статус
void SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings)
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

    const vector<string> words = SearchServer::SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
    for (const string& word : words)
    {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        ids_to_word_to_freqs[document_id][word] += inv_word_count;
        ids_to_words_[document_id].insert(word);
    }
    if (words_to_ids_to_dublicate.count(document))
    {
        words_to_ids_to_dublicate[document][document_id] = true;
        id_duplicate[document_id] =  true;
    }
    else
    {
        words_to_ids_to_dublicate[document][document_id] = false;
        id_duplicate[document_id] = false;
    }
    documents_ids_.insert(document_id);
    documents_.emplace(document_id,
        DocumentData
        {
            ComputeAverageRating(ratings),
            status
        });
}

//vector<int> SearchServer::FindDuplicates() const
//{
//    vector<int> result;
//    for (auto [id, dublicate] :id_duplicate)
//    {
//        if (dublicate == true)
//        {
//            result.push_back(id);
//        }
//    }
//    return result;
//}

vector<int> SearchServer::FindDuplicates() const
{
    vector<set<string>> checked_docs;
    vector<int> result;
    for (auto& [id, words] : ids_to_words_)
    {
        if (count(checked_docs.begin(), checked_docs.end(), words))
        {
            result.push_back(id);
        }
        else
        {
            checked_docs.push_back(words);
        }
    }
    return result;
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus check_status) const
{
    return SearchServer::FindTopDocuments(raw_query, [check_status](int document_id, DocumentStatus status, int rating) { return status == check_status; });
}
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const
{
    return SearchServer::FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

//Возвращает длину documents_
int SearchServer::GetDocumentCount() const
{
    return documents_.size();
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query, int document_id) const
{
    const Query query = ParseQuery(raw_query);
    vector<string> matched_words;

    for (const string& word : query.plus_words)
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

    for (const string& word : query.minus_words)
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

std::set<int>::const_iterator SearchServer::begin() const
{
    return documents_ids_.begin();
}

std::set<int>::const_iterator SearchServer::end() const
{
    return documents_ids_.end();
}

const map<string, double>& SearchServer::GetWordFrequencies(int document_id) const
{
    if (find(this->begin(), this->end(), document_id) == this->end())
    {
        static const map<string, double> empty{};
        return empty;
    }
    return ids_to_word_to_freqs.at(document_id);
}

void SearchServer::RemoveDocument(int document_id)
{
    for (auto [word, TF] : ids_to_word_to_freqs.at(document_id))
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

bool SearchServer::IsValidString(const string& text)
{
    return (none_of(text.begin(), text.end(), [](char x)
        {
            if ((x > 0) && (x < 32))
                return true;
            return false;
        }));
}

bool SearchServer::IsStopWord(const string& word) const
{
    return stop_words_.count(word) > 0;
}

//Из строки в вектор слов, исключая стоп-слова
vector<string> SearchServer::SplitIntoWordsNoStop(const string& text) const
{
    vector<string> words;
    for (const string& word : SplitIntoWords(text))
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

SearchServer::QueryWord SearchServer::ParseQueryWord(string word) const
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

//Делает из строки множества плюс и минус слов
SearchServer::Query SearchServer::ParseQuery(const string& text) const
{
    Query query;
    for (const string& word : SplitIntoWords(text))
    {
        const QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop)
        {
            if (query_word.is_minus)
            {
                query.minus_words.insert(query_word.data);
            }
            else
            {
                query.plus_words.insert(query_word.data);
            }
        }
    }
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
#include "search_server.h"
#include "string_processing.h"
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

void SearchServer::SetStopWords(const string& text)
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
    }
    SearchServer::documents_ids_.push_back(document_id);
    SearchServer::documents_.emplace(document_id,
        DocumentData
        {
            ComputeAverageRating(ratings),
            status
        });
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

int SearchServer::GetDocumentId(int index) const
{
    if ((index < 0) || (index > SearchServer::GetDocumentCount()))
    {
        throw out_of_range("Invalid index");
    }
    return SearchServer::documents_ids_[index];
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
#pragma once
#include "document.h"
#include "log_duration.h"
#include<vector>
#include<string>
#include<string_view>
#include<iostream>
#include<set>
#include<map>
#include <algorithm>
#include <numeric>
#include <execution>

const double EPSILON = 1e-6; //Число для сравнения double чисел
const int MAX_RESULT_DOCUMENT_COUNT = 5;

enum class DocumentStatus
{
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED
};



class SearchServer
{
public:

    SearchServer() = default;

    //Конструктор для string стоп-слов
    explicit SearchServer(const std::string& stop_words);
    explicit SearchServer(std::string_view stop_words);

    //Конструктор для set и vector стоп-слов
    template <typename Container>
    explicit SearchServer(const Container& container)
    {
        for (std::string_view str : container)
        {
            if (!str.empty())
            {
                stop_words_.emplace(str);
            }
        }

        std::for_each(
            stop_words_.begin(), stop_words_.end(),
            [&](const auto& word)
            {
                if (!IsValidString(word))
                {
                    throw std::invalid_argument("Forbidden symbols");
                }
            });
    }

    void SetStopWords(const std::string& text);

    //Добавляет в documents_ id, средний рейтинг, статус
    //void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);
    void AddDocument(int document_id, std::string_view document, DocumentStatus status, const std::vector<int>& ratings);

    //Сортирует и возвращает 5 лучших по релевантности документа
    /*template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate predicate) const;
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus check_status) const;
    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;*/

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentPredicate predicate) const;
    std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentStatus check_status) const;
    std::vector<Document> FindTopDocuments(std::string_view raw_query) const;

    //Возвращает длину documents_
    int GetDocumentCount() const;

    /*std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;
    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(std::execution::parallel_policy, const std::string& raw_query, int document_id) const;
    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(std::execution::sequenced_policy, const std::string& raw_query, int document_id) const;*/

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view raw_query, int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::execution::parallel_policy, std::string_view raw_query, int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::execution::sequenced_policy, std::string_view raw_query, int document_id) const;

    std::set<int>::const_iterator begin() const;

    std::set<int>::const_iterator end() const;

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

    void RemoveDocument(int document_id);
    void RemoveDocument(std::execution::parallel_policy, int document_id);
    void RemoveDocument(std::execution::sequenced_policy, int document_id);

    std::map<int, std::set<std::string>> ids_to_words_;                     // Словарь id - все слова без повторов в документе


private:
    //Структура рейтинг, DocumentStatus(DocumentStatus::actuall,DocumentStatus::banned...)
    struct DocumentData
    {
        int rating = 0;
        DocumentStatus status = DocumentStatus::ACTUAL;
        std::string raw_text;
    };

    std::set<std::string, std::less<>> stop_words_;                                      //Множество стоп-слов
    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;   //Словарь: слово - словарь(id,частота встречи в документах(TF))
    std::map<int, DocumentData> documents_;                                 //Словарь: id - DocumentData(рейтинг, статус)
    std::set<int> documents_ids_;                                           // Идентификаторы
    std::map<int, std::map<std::string_view, double>> ids_to_word_to_freqs;      // Словарь: id - слова - частота

    //static bool IsValidString(const std::string& text);
    static bool IsValidString(std::string_view text);

    //bool IsStopWord(const std::string& word) const;
    bool IsStopWord(std::string_view word) const;

    //Из строки в вектор слов, исключая стоп-слова
    //std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;
    std::vector<std::string_view> SplitIntoWordsNoStop(std::string_view text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    //Структура слово - bool(минус-слово) - bool(плюс-слово)
    struct QueryWord
    {
        std::string_view data;
        bool is_minus = false;
        bool is_stop = false;
    };

    //QueryWord ParseQueryWord(std::string word) const;
    QueryWord ParseQueryWord(std::string_view word) const;

    //Структура множество плюс-слов - множество минус-слов
    struct Query
    {
        std::vector<std::string_view> plus_words;
        std::vector<std::string_view> minus_words;
    };

    //Делает из строки множества плюс и минус слов
    //Query ParseQuery(const std::string& text) const;
    Query ParseQuery(std::string_view text) const;

    // Existence required
    double ComputeWordInverseDocumentFreq(const std::string& word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate predicate) const
    {
        std::map<int, double> document_to_relevance;
        for (std::string_view word : query.plus_words)
        {
            if (word_to_document_freqs_.count(word) == 0)
            {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(std::string(word));
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word))
            {
                auto current_document = documents_.at(document_id);
                if (predicate(document_id, current_document.status, current_document.rating))
                {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (std::string_view word : query.minus_words)
        {
            if (word_to_document_freqs_.count(word) == 0)
            {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word))
            {
                document_to_relevance.erase(document_id);
            }
        }

        std::vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance)
        {
            matched_documents.push_back(
                Document
                {
                document_id,
                relevance,
                documents_.at(document_id).rating
                });
        }
        return matched_documents;
    }
};

std::ostream& operator<<(std::ostream& out, const Document doc);

//template <typename DocumentPredicate>
//std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentPredicate predicate) const
//{
//    const Query query = ParseQuery(raw_query);
//    auto matched_documents = FindAllDocuments(query, predicate);
//
//    sort(matched_documents.begin(), matched_documents.end(),
//        [](const Document& lhs, const Document& rhs)
//        {
//            if (std::abs(lhs.relevance - rhs.relevance) < EPSILON)
//            {
//                return lhs.rating > rhs.rating;
//            }
//            else
//            {
//                return lhs.relevance > rhs.relevance;
//            }
//        });
//    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
//    {
//        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
//    }
//    return matched_documents;
//}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query, DocumentPredicate predicate) const
{
    const Query query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query, predicate);

    sort(matched_documents.begin(), matched_documents.end(),
        [](const Document& lhs, const Document& rhs)
        {
            if (std::abs(lhs.relevance - rhs.relevance) < EPSILON)
            {
                return lhs.rating > rhs.rating;
            }
            else
            {
                return lhs.relevance > rhs.relevance;
            }
        });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
    {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
}
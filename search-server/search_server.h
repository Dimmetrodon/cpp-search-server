#pragma once
#include "document.h"
#include<vector>
#include<string>
#include<iostream>
#include<set>
#include<map>
#include <algorithm>
#include <numeric>

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

    //Конструктор для set и vector стоп-слов
    template <typename Container>
    explicit SearchServer(const Container& container)
    {
        using namespace std::literals;
        for (const std::string& word : container)
        {
            if (!IsValidString(word))
            {
                throw std::invalid_argument("Forbidden symbols entered"s);
            }
            if (word != ""s)
            {
                stop_words_.insert(word);
            }
        }
    }

    void SetStopWords(const std::string& text);

    //Добавляет в documents_ id, средний рейтинг, статус
    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);

    //Сортирует и возвращает 5 лучших по релевантности документа
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate predicate) const
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

    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus check_status) const
    {
        return SearchServer::FindTopDocuments(raw_query, [check_status](int document_id, DocumentStatus status, int rating) { return status == check_status; });
    }

    std::vector<Document> FindTopDocuments(const std::string& raw_query) const
    {
        return SearchServer::FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    //Возвращает длину documents_
    int GetDocumentCount() const;

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;

    int GetDocumentId(int index) const;

private:
    //Структура рейтинг, DocumentStatus(DocumentStatus::actuall,DocumentStatus::banned...)
    struct DocumentData
    {
        int rating;
        DocumentStatus status;
    };

    std::set<std::string> stop_words_; //Множество стоп-слов

    std::map<std::string, std::map<int, double>> word_to_document_freqs_; //Словарь: слово - словарь(id,частота встречи в документах)

    std::map<int, DocumentData> documents_; //Словарь: id - DocumentData(рейтинг, статус)

    std::vector<int> documents_ids_; // Вектор идентификаторов хранящий их по порядку добавления


    static bool IsValidString(const std::string& text);

    bool IsStopWord(const std::string& word) const;

    //Из строки в вектор слов, исключая стоп-слова
    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    //Структура слово - bool(минус-слово) - bool(плюс-слово)
    struct QueryWord
    {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(std::string word) const;

    //Структура множество плюс-слов - множество минус-слов
    struct Query
    {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    //Делает из строки множества плюс и минус слов
    Query ParseQuery(const std::string& text) const;

    // Existence required
    double ComputeWordInverseDocumentFreq(const std::string& word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate predicate) const
    {
        std::map<int, double> document_to_relevance;
        for (const std::string& word : query.plus_words)
        {
            if (word_to_document_freqs_.count(word) == 0)
            {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word))
            {
                auto current_document = documents_.at(document_id);
                if (predicate(document_id, current_document.status, current_document.rating))
                {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const std::string& word : query.minus_words)
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
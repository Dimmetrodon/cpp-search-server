#pragma once
#include <deque>
#include <vector>
#include "search_server.h"


class RequestQueue
{
public:
    explicit RequestQueue(const SearchServer& search_server);

    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate); 

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus check_status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

private:
    struct QueryResult
    {
        // определите, что должно быть в структуре
        int request_time;
        int results;
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    int current_mins_ = 0;
    const SearchServer& search_server_;
    bool day_ = false;

    void MinsCheck();

    void MoveDeque();

};


template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate)
{
    // напишите реализацию
    std::vector<Document> vec = search_server_.FindTopDocuments(raw_query, document_predicate);
    MinsCheck();
    MoveDeque();

    QueryResult result;
    result.request_time = current_mins_;
    result.results = vec.size();
    requests_.push_back(result);

    return vec;
}
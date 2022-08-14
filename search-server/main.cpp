// search_server_s1_t2_v2.cpp


#include <iostream>
#include "document.h"
#include "search_server.h"
#include "request_queue.h"
#include "paginator.h"
#include "read_input_functions.h"

using namespace std;


//string ReadLine()
//{
//    string s;
//    getline(cin, s);
//    return s;
//}
//
//int ReadLineWithNumber()
//{
//    int result;
//    cin >> result;
//    ReadLine();
//    return result;
//}
//
////Дробит строку в слова
//vector<string> SplitIntoWords(const string& text)
//{
//    vector<string> words;
//    string word;
//    for (const char c : text)
//    {
//        if (c == ' ')
//        {
//            if (!word.empty())
//            {
//                words.push_back(word);
//                word.clear();
//            }
//        }
//        else
//        {
//            word += c;
//        }
//    }
//    if (!word.empty())
//    {
//        words.push_back(word);
//    }
//
//    return words;
//}

////Структура id, релевантность, рейтинг
//struct Document
//{
//    Document()
//        :id(0), relevance(0), rating(0)
//    {
//    }
//
//    explicit Document(int in_id, double in_relevance, int in_rating)
//        :id(in_id), relevance(in_relevance), rating(in_rating)
//    {
//    }
//
//    int id;
//    double relevance;
//    int rating;
//};

//enum class DocumentStatus
//{
//    ACTUAL,
//    IRRELEVANT,
//    BANNED,
//    REMOVED
//};

//class SearchServer
//{
//public:
//
//    SearchServer() = default;
//
//    //Конструктор для string стоп-слов
//    explicit SearchServer(const string& stop_words)
//    {
//        vector<string> stop_words_vector = SplitIntoWords(stop_words);
//        for (const string& word : stop_words_vector)
//        {
//            if (!IsValidString(word))
//            {
//                throw invalid_argument("Forbidden symbols entered"s);
//            }
//        }
//        for (const string& word : stop_words_vector)
//        {
//            stop_words_.insert(word);
//        }
//    }
//
//    //Конструктор для set и vector стоп-слов
//    template <typename Container>
//    explicit SearchServer(const Container& container)
//    {
//        for (const string& word : container)
//        {
//            if (!IsValidString(word))
//            {
//                throw invalid_argument("Forbidden symbols entered"s);
//            }
//            if (word != ""s)
//            {
//                stop_words_.insert(word);
//            }
//        }
//    }
//
//    void SetStopWords(const string& text)
//    {
//        for (const string& word : SplitIntoWords(text))
//        {
//            stop_words_.insert(word);
//        }
//    }
//
//    //Добавляет в documents_ id, средний рейтинг, статус
//    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings)
//    {
//        if (documents_.count(document_id) == 1)
//        {
//            throw invalid_argument("id is taken"s);
//        }
//        else if (document_id < 0)
//        {
//            throw invalid_argument("id value should not be less than 0"s);
//        }
//        else if (!IsValidString(document))
//        {
//            throw invalid_argument("Forbidden symbols"s);
//        }
//        const vector<string> words = SplitIntoWordsNoStop(document);
//        const double inv_word_count = 1.0 / words.size();
//        for (const string& word : words)
//        {
//            word_to_document_freqs_[word][document_id] += inv_word_count;
//        }
//        documents_ids_.push_back(document_id);
//        documents_.emplace(document_id,
//            DocumentData
//            {
//                ComputeAverageRating(ratings),
//                status
//            });
//    }
//
//    //Сортирует и возвращает 5 лучших по релевантности документа
//    template <typename DocumentPredicate>
//    vector<Document> FindTopDocuments(const string& raw_query, DocumentPredicate predicate) const
//    {
//        const Query query = ParseQuery(raw_query);
//        auto matched_documents = FindAllDocuments(query, predicate);
//
//        sort(matched_documents.begin(), matched_documents.end(),
//            [](const Document& lhs, const Document& rhs)
//            {
//                if (abs(lhs.relevance - rhs.relevance) < EPSILON)
//                {
//                    return lhs.rating > rhs.rating;
//                }
//                else
//                {
//                    return lhs.relevance > rhs.relevance;
//                }
//            });
//        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
//        {
//            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
//        }
//        return matched_documents;
//    }
//
//    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus check_status) const
//    {
//        return FindTopDocuments(raw_query, [check_status](int document_id, DocumentStatus status, int rating) { return status == check_status; });
//    }
//
//    vector<Document> FindTopDocuments(const string& raw_query) const
//    {
//        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
//    }
//
//    //Возвращает длину documents_
//    int GetDocumentCount() const
//    {
//        return documents_.size();
//    }
//
//    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const
//    {
//        const Query query = ParseQuery(raw_query);
//        vector<string> matched_words;
//        for (const string& word : query.plus_words)
//        {
//            if (word_to_document_freqs_.count(word) == 0)
//            {
//                continue;
//            }
//            if (word_to_document_freqs_.at(word).count(document_id))
//            {
//                matched_words.push_back(word);
//            }
//        }
//        for (const string& word : query.minus_words)
//        {
//            if (word_to_document_freqs_.count(word) == 0)
//            {
//                continue;
//            }
//            if (word_to_document_freqs_.at(word).count(document_id))
//            {
//                matched_words.clear();
//                break;
//            }
//        }
//        return tuple{ matched_words, documents_.at(document_id).status };
//    }
//
//    int GetDocumentId(int index) const
//    {
//        if ((index < 0) || (index > GetDocumentCount()))
//        {
//            throw out_of_range("Invalid index"s);
//        }
//        return documents_ids_[index];
//    }
//
//private:
//    //Структура рейтинг, DocumentStatus(DocumentStatus::actuall,DocumentStatus::banned...)
//    struct DocumentData
//    {
//        int rating;
//        DocumentStatus status;
//    };
//
//    set<string> stop_words_; //Множество стоп-слов
//
//    map<string, map<int, double>> word_to_document_freqs_; //Словарь: слово - словарь(id,частота встречи в документах)
//
//    map<int, DocumentData> documents_; //Словарь: id - DocumentData(рейтинг, статус)
//
//    vector<int> documents_ids_; // Вектор идентификаторов хранящий их по порядку добавления
//
//    
//    static bool IsValidString(const string& text) 
//    {
//        return (none_of(text.begin(), text.end(), [](char x)
//            {
//                if ((x > 0) && (x < 32))
//                    return true;
//                return false;
//            }));        
//    }
//
//    bool IsStopWord(const string& word) const
//    {
//        return stop_words_.count(word) > 0;
//    }
//
//    //Из строки в вектор слов, исключая стоп-слова
//    vector<string> SplitIntoWordsNoStop(const string& text) const
//    {
//        vector<string> words;
//        for (const string& word : SplitIntoWords(text))
//        {
//            if (!IsStopWord(word))
//            {
//                words.push_back(word);
//            }
//        }
//        return words;
//    }
//
//    static int ComputeAverageRating(const vector<int>& ratings)
//    {
//        if (ratings.empty())
//        {
//            return 0;
//        }
//        int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
//        return rating_sum / static_cast<int>(ratings.size());
//    }
//
//    //Структура слово - bool(минус-слово) - bool(плюс-слово)
//    struct QueryWord
//    {
//        string data;
//        bool is_minus;
//        bool is_stop;
//    };
//
//    QueryWord ParseQueryWord(string word) const
//    {
//        if (!IsValidString(word))
//        {
//            throw invalid_argument("Forbidden symbols"s);
//        }
//
//        bool is_minus = false;
//        // Word shouldn't be empty
//        if (word[0] == '-')
//        {
//            is_minus = true;
//            word = word.substr(1);
//            if (word.empty())
//            {
//                throw invalid_argument("No text after minus"s);
//            }
//            if (word[0] == '-')
//            {
//                throw invalid_argument("Multiple minuses"s);
//            }
//        }
//        return
//        {
//            word,
//            is_minus,
//            IsStopWord(word)
//        };
//    }
//
//    //Структура множество плюс-слов - множество минус-слов
//    struct Query
//    {
//        set<string> plus_words;
//        set<string> minus_words;
//    };
//
//    //Делает из строки множества плюс и минус слов
//    Query ParseQuery(const string& text) const
//    {
//        Query query;
//        for (const string& word : SplitIntoWords(text))
//        {
//            const QueryWord query_word = ParseQueryWord(word);
//            if (!query_word.is_stop)
//            {
//                if (query_word.is_minus)
//                {
//                    query.minus_words.insert(query_word.data);
//                }
//                else
//                {
//                    query.plus_words.insert(query_word.data);
//                }
//            }
//        }
//        return query;
//    }
//
//    // Existence required
//    double ComputeWordInverseDocumentFreq(const string& word) const
//    {
//        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
//    }
//
//    template <typename DocumentPredicate>
//    vector<Document> FindAllDocuments(const Query& query, DocumentPredicate predicate) const
//    {
//        map<int, double> document_to_relevance;
//        for (const string& word : query.plus_words)
//        {
//            if (word_to_document_freqs_.count(word) == 0)
//            {
//                continue;
//            }
//            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
//            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word))
//            {
//                auto current_document = documents_.at(document_id);
//                if (predicate(document_id, current_document.status, current_document.rating))
//                {
//                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
//                }
//            }
//        }
//
//        for (const string& word : query.minus_words)
//        {
//            if (word_to_document_freqs_.count(word) == 0)
//            {
//                continue;
//            }
//            for (const auto [document_id, _] : word_to_document_freqs_.at(word))
//            {
//                document_to_relevance.erase(document_id);
//            }
//        }
//
//        vector<Document> matched_documents;
//        for (const auto [document_id, relevance] : document_to_relevance)
//        {
//            matched_documents.push_back(
//                Document
//                {
//                document_id,
//                relevance,
//                documents_.at(document_id).rating
//                });
//        }
//        return matched_documents;
//    }
//};

// ------------- Output changes -------------

//template <typename Key, typename Value>
//ostream& operator<<(ostream& out, const pair<Key, Value>& pair)
//{
//    out << pair.first << ": "s << pair.second;
//    return out;
//}
//
//template<typename Container>
//void Print(ostream& out, const Container& container)
//{
//    int i = 1;
//    for (const auto& element : container)
//    {
//        if (i != container.size())
//        {
//            out << element << ", "s;
//            ++i;
//        }
//        else
//        {
//            out << element;
//        }
//    }
//}
//
//template<typename T>
//ostream& operator<<(ostream& out, const vector<T>& container)
//{
//    out << "["s;
//    Print(out, container);
//    out << "]"s;
//    return out;
//}
//
//template<typename T>
//ostream& operator<<(ostream& out, const set<T>& container)
//{
//    out << "{"s;
//    Print(out, container);
//    out << "}"s;
//    return out;
//}
//
//template <typename Key, typename Value>
//ostream& operator<<(ostream& out, const map<Key, Value>& map)
//{
//    out << "{"s;
//    Print(out, map);
//    out << "}"s;
//    return out;
//}

// ------------- Output changes end -------------

// ------------- Assert -------------

//template <typename T, typename U>
//void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
//    const string& func, unsigned line, const string& hint)
//{
//    if (t != u)
//    {
//        cerr << boolalpha;
//        cerr << file << "("s << line << "): "s << func << ": "s;
//        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
//        cerr << t << " != "s << u << "."s;
//        if (!hint.empty())
//        {
//            cerr << " Hint: "s << hint;
//        }
//        cerr << endl;
//        abort();
//    }
//}
//
//#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)
//
//#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))
//
//void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
//    const string& hint)
//{
//    if (!value)
//    {
//        cerr << file << "("s << line << "): "s << func << ": "s;
//        cerr << "ASSERT("s << expr_str << ") failed."s;
//        if (!hint.empty())
//        {
//            cerr << " Hint: "s << hint;
//        }
//        cerr << endl;
//        abort();
//    }
//}
//
//#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)
//
//#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))
//
//
//template <typename Function>
//void RunTestImpl(Function function, const string& func_name)
//{
//    function();
//    cerr << func_name << " OK"s << endl;
//}
//
//#define RUN_TEST(function) RunTestImpl((function), #function)

// ------------- Assert end -------------

// -------- Начало модульных тестов поисковой системы ----------

//void TestMultipleMinusesQuery()
//{
//    SearchServer server;
//    vector<Document> result;
//    (void)server.AddDocument(1, "cat in the city"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
//
//    ASSERT_EQUAL(server.FindTopDocuments("--cat"s, result), false);
//    ASSERT_EQUAL(server.FindTopDocuments("    --cat"s, result), false);
//    ASSERT_EQUAL(server.FindTopDocuments("city --cat"s, result), false);
//    ASSERT_EQUAL(server.FindTopDocuments("cat-city"s, result), true);
//}
//
//
//// Функция TestSearchServer является точкой входа для запуска тестов
//
//void TestSearchServer()
//{
//    //RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
//}

// --------- Окончание модульных тестов поисковой системы -----------

//
//void PrintDocument(const Document& document) 
//{
//    cout << "{ "s
//        << "document_id = "s << document.id << ", "s
//        << "relevance = "s << document.relevance << ", "s
//        << "rating = "s << document.rating << " }"s << endl;
//}
//
//void PrintMatchDocumentResult(int document_id, const vector<string>& words, DocumentStatus status) 
//{
//    cout << "{ "s
//        << "document_id = "s << document_id << ", "s
//        << "status = "s << static_cast<int>(status) << ", "s
//        << "words ="s;
//    for (const string& word : words) 
//    {
//        cout << ' ' << word;
//    }
//    cout << "}"s << endl;
//}
//
//void AddDocument(SearchServer& search_server, int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) 
//{
//    try {
//        search_server.AddDocument(document_id, document, status, ratings);
//    }
//    catch (const exception& e) 
//    {
//        cout << "Ошибка добавления документа "s << document_id << ": "s << e.what() << endl;
//    }
//}
//
//void FindTopDocuments(const SearchServer& search_server, const string& raw_query) 
//{
//    cout << "Результаты поиска по запросу: "s << raw_query << endl;
//    try 
//    {
//        for (const Document& document : search_server.FindTopDocuments(raw_query)) 
//        {
//            PrintDocument(document);
//        }
//    }
//    catch (const exception& e) 
//    {
//        cout << "Ошибка поиска: "s << e.what() << endl;
//    }
//}
//
//void MatchDocuments(const SearchServer& search_server, const string& query) 
//{
//    try 
//    {
//        cout << "Матчинг документов по запросу: "s << query << endl;
//        const int document_count = search_server.GetDocumentCount();
//        for (int index = 0; index < document_count; ++index) 
//        {
//            const int document_id = search_server.GetDocumentId(index);
//            const auto [words, status] = search_server.MatchDocument(query, document_id);
//            PrintMatchDocumentResult(document_id, words, status);
//        }
//    }
//    catch (const exception& e) 
//    {
//        cout << "Ошибка матчинга документов на запрос "s << query << ": "s << e.what() << endl;
//    }
//}


//template <typename Iterator>
//class IteratorRange 
//{
//public:
//    IteratorRange(Iterator page_begin, Iterator page_end)
//        : page_begin_(page_begin)
//        , page_end_(page_end)
//    {
//    }
//
//    Iterator begin() const 
//    {
//        return page_begin_;
//    }
//
//    Iterator end() const 
//    {
//        return page_end_;
//    }
//
//private:
//    Iterator page_begin_;
//    Iterator page_end_;
//};

//template<typename Iterator>
//class Paginator
//{
//public:
//    Paginator(Iterator begin, Iterator end, size_t page_size)
//    {
//        if (page_size == 1)
//        {
//            Iterator it = begin;
//            int distance_ = distance(begin, end);
//            for (int i = 1; i < distance_; i += page_size)
//            {
//                elements.push_back({ it, it + 1 });
//                advance(it, page_size);
//            }
//            if (distance(begin, end) > page_size)
//            {
//                elements.push_back({ it , it + 1 });
//            }
//        }
//        else
//        {
//            Iterator it = begin;
//            int distance_ = distance(begin, end);
//            if (!(distance_ % page_size))
//            {
//                int i = 1;
//                do
//                {
//                    elements.push_back({ it, it + page_size - 1 });
//                    advance(it, page_size);
//                    i += page_size;
//                } while (i < distance_);
//            }
//            else
//            {
//                int i = 1;
//                do
//                {
//                    elements.push_back({ it, it + page_size - 1 });
//                    advance(it, page_size);
//                    i += page_size;
//                } while (i < distance_);
//                elements.push_back({end - 1, end - 1});
//            }
//            
//        }        
//    }
//
//    auto begin() const
//    {
//        return elements.begin();
//    }
//
//    auto end() const
//    {
//        return elements.end();
//    }
//
//private:
//    vector<IteratorRange<Iterator>> elements;
//};


//template <typename Iterator>
//ostream& operator<<(ostream& out, const IteratorRange<Iterator> iterators)
//{
//    for (auto it = iterators.begin(); it <= iterators.end(); ++it)
//    {
//        out << *it;
//    }
//    return out;
//}

//ostream& operator<<(ostream& out, const Document doc)
//{
//    out << "{ document_id = "s << doc.id << ", relevance = "s << doc.relevance << ", rating = "s << doc.rating << " }"s;
//    return out;
//}

//template <typename Container>
//auto Paginate(const Container& c, size_t page_size) 
//{
//    return Paginator(begin(c), end(c), page_size);
//}

//class RequestQueue 
//{
//public:
//    explicit RequestQueue(const SearchServer& search_server)
//        :search_server_(search_server)
//    {
//        // напишите реализацию
//    }
//    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
//    template <typename DocumentPredicate>
//    vector<Document> AddFindRequest(const string& raw_query, DocumentPredicate document_predicate) 
//    {
//        // напишите реализацию
//        vector<Document> vec = search_server_.FindTopDocuments(raw_query, document_predicate);
//        MinsCheck();
//        MoveDeque();
//
//        QueryResult result;
//        result.request_time = current_mins_;
//        result.results = vec.size();
//        requests_.push_back(result);
//
//        return vec;
//    }
//
//    vector<Document> AddFindRequest(const string& raw_query, DocumentStatus check_status) 
//    {
//        // напишите реализацию
//        return AddFindRequest(raw_query, [check_status](int document_id, DocumentStatus status, int rating) { return status == check_status; });
//    }
//
//    vector<Document> AddFindRequest(const string& raw_query) 
//    {
//        // напишите реализацию
//        return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
//    }
//
//    int GetNoResultRequests() const 
//    {
//        // напишите реализацию
//        int empty_results = 0;
//        for (auto it = requests_.begin(); it != requests_.end(); it = next(it))
//        {
//            if (it->results == 0)
//            {
//                empty_results += 1;
//            }
//        }
//        return empty_results;
//    }
//private:
//    struct QueryResult 
//    {
//        // определите, что должно быть в структуре
//        int request_time;
//        int results;
//    };
//    deque<QueryResult> requests_;
//    const static int min_in_day_ = 1440;
//    int current_mins_ = 0;
//    const SearchServer& search_server_;
//    bool day_ = false;
//
//    // возможно, здесь вам понадобится что-то ещё
//
//    void MinsCheck()
//    {
//        if (current_mins_ != (min_in_day_))
//        {
//            current_mins_ += 1;
//        }
//        else
//        {
//            day_ = true;
//            current_mins_ = 1;
//        }
//    }
//
//    void MoveDeque()
//    {
//        if (day_)
//        {
//            requests_.pop_front();
//        }
//    }
//
//};

int main()
{
    setlocale(LC_ALL, "Russian");
    SearchServer search_server;


    AddDocument(search_server, 1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    AddDocument(search_server, 1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, { 1, 2 });
    AddDocument(search_server, -1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, { 1, 2 });
    AddDocument(search_server, 3, "большой пёс скво\x12рец евгений"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
    AddDocument(search_server, 4, "большой пёс скворец евгений"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

    FindTopDocuments(search_server, "пушистый -пёс"s);
    FindTopDocuments(search_server, "пушистый --кот"s);
    FindTopDocuments(search_server, "пушистый -"s);
    FindTopDocuments(search_server, "пушистый\x12"s);

    MatchDocuments(search_server, "пушистый пёс"s);
    MatchDocuments(search_server, "модный -кот"s);
    MatchDocuments(search_server, "модный --пёс"s);
    MatchDocuments(search_server, "пушистый - хвост"s);
}
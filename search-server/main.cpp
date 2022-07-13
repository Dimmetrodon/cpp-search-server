// search_server_s1_t2_v2.cpp

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <numeric>
#include <cassert>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6; //Число для сравнения double чисел

string ReadLine()
{
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber()
{
    int result;
    cin >> result;
    ReadLine();
    return result;
}

//Дробит строку в слова
vector<string> SplitIntoWords(const string& text)
{
    vector<string> words;
    string word;
    for (const char c : text)
    {
        if (c == ' ')
        {
            if (!word.empty())
            {
                words.push_back(word);
                word.clear();
            }
        }
        else
        {
            word += c;
        }
    }
    if (!word.empty())
    {
        words.push_back(word);
    }

    return words;
}

//Структура id, релевантность, рейтинг
struct Document
{
    int id;
    double relevance;
    int rating;
};

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
    //Добавляет в stop_words_ стоп-слова
    void SetStopWords(const string& text)
    {
        for (const string& word : SplitIntoWords(text))
        {
            stop_words_.insert(word);
        }
    }
    //Добавляет в documents_ id, средний рейтинг, статус
    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings)
    {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words)
        {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id,
            DocumentData
            {
                ComputeAverageRating(ratings),
                status
            });
    }

    //Сортирует и возвращает 5 лучших по релевантности документа
    template <typename Suitability>
    vector<Document> FindTopDocuments(const string& raw_query, Suitability suitability) const
    {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, suitability);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs)
            {
                if (abs(lhs.relevance - rhs.relevance) < EPSILON)
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

    //Сортирует и возвращает 5 лучших по релевантности документа
    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus check_status = DocumentStatus::ACTUAL) const
    {
        return FindTopDocuments(raw_query, [check_status](int document_id, DocumentStatus status, int rating) { return status == check_status; });
    }

    //Возвращает длину documents_
    int GetDocumentCount() const
    {
        return documents_.size();
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const
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
        return { matched_words, documents_.at(document_id).status };
    }

private:
    //Структура рейтинг, DocumentStatus(DocumentStatus::actuall,DocumentStatus::banned...)
    struct DocumentData
    {
        int rating;
        DocumentStatus status;
    };

    //Множество стоп-слов
    set<string> stop_words_;
    //Словарь слово - словарь(id,частота встречи в документах)
    map<string, map<int, double>> word_to_document_freqs_;
    //Словарь id - DocumentData(рейтинг, статус)
    map<int, DocumentData> documents_;

    //Является ли стоп-словом
    bool IsStopWord(const string& word) const
    {
        return stop_words_.count(word) > 0;
    }

    //Из строки в вектор слов, исключая стоп-слова
    vector<string> SplitIntoWordsNoStop(const string& text) const
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
    //Возвращает средний рейтинг
    static int ComputeAverageRating(const vector<int>& ratings)
    {
        if (ratings.empty())
        {
            return 0;
        }
        int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
        return rating_sum / static_cast<int>(ratings.size());
    }

    //Структура слово - bool(минус-слово) - bool(плюс-слово)
    struct QueryWord
    {
        string data;
        bool is_minus;
        bool is_stop;
    };

    //Принимает слово. Возвращает структуру слово - bool(минус-слово) - bool(плюс-слово)
    QueryWord ParseQueryWord(string text) const
    {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-')
        {
            is_minus = true;
            text = text.substr(1);
        }
        return
        {
            text,
            is_minus,
            IsStopWord(text)
        };
    }

    //Структура множество плюс-слов - множество минус-слов
    struct Query
    {
        set<string> plus_words;
        set<string> minus_words;
    };

    //Делает из строки множества плюс и минус слов
    Query ParseQuery(const string& text) const
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
    double ComputeWordInverseDocumentFreq(const string& word) const
    {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template <typename Suitability>
    vector<Document> FindAllDocuments(const Query& query, Suitability suitability) const
    {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words)
        {
            if (word_to_document_freqs_.count(word) == 0)
            {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word))
            {
                auto current_document = documents_.at(document_id);
                if (suitability(document_id, current_document.status, current_document.rating))
                {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const string& word : query.minus_words)
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

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance)
        {
            matched_documents.push_back({
                document_id,
                relevance,
                documents_.at(document_id).rating
                });
        }
        return matched_documents;
    }
};

// ------------- Изменения вывода -------------
template <typename Key, typename Value>
ostream& operator<<(ostream& out, const pair<Key, Value>& pair)
{
    out << pair.first << ": "s << pair.second;
    return out;
}

template<typename Container>
void Print(ostream& out, const Container& container)
{
    int i = 1;
    for (const auto& element : container)
    {
        if (i != container.size())
        {
            out << element << ", "s;
            ++i;
        }
        else
        {
            out << element;
        }
    }
}

template<typename T>
ostream& operator<<(ostream& out, const vector<T>& container)
{
    out << "["s;
    Print(out, container);
    out << "]"s;
    return out;
}

template<typename T>
ostream& operator<<(ostream& out, const set<T>& container)
{
    out << "{"s;
    Print(out, container);
    out << "}"s;
    return out;
}

template <typename Key, typename Value>
ostream& operator<<(ostream& out, const map<Key, Value>& map)
{
    out << "{"s;
    Print(out, map);
    out << "}"s;
    return out;
}
// -------------------------------------------

// ------------- Assert -------------
template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
    const string& func, unsigned line, const string& hint)
{
    if (t != u)
    {
        cerr << boolalpha;
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cerr << t << " != "s << u << "."s;
        if (!hint.empty())
        {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
    const string& hint)
{
    if (!value)
    {
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty())
        {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))
// -------------------------------------------


template <typename Function>
void RunTestImpl(Function function, const string& func_name)
{
    function();
    cerr << func_name << " OK"s << endl;
}

#define RUN_TEST(function) RunTestImpl((function), #function)

// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents"s);
    }
}

void TestAddDocument()
{
    const int doc_id = 100;
    const string content = "big eared dog";
    const vector<int> ratings = { 5, 4, 5 };

    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);

    auto found_docs = server.FindTopDocuments("big"s);
    Document doc0 = found_docs[0];
    ASSERT_EQUAL(doc0.id, doc_id);

    found_docs = server.FindTopDocuments("eared"s);
    doc0 = found_docs[0];
    ASSERT_EQUAL(found_docs[0].id, doc_id);

    found_docs = server.FindTopDocuments("dog"s);
    doc0 = found_docs[0];
    ASSERT_EQUAL(found_docs[0].id, doc_id);
}

void TestStopWords()
{
    SearchServer server;

    const string stop_words = "in for that"s;
    server.SetStopWords(stop_words);

    const string content = "Cat in the city looking for house that can become his home"s;
    const int doc_id = 1;
    const vector<int> ratings = { 5, 5, 4 };
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    auto [words, status] = server.MatchDocument("Cat in house for become that"s, 1);
    const vector<string> estimated_outcome = { "Cat"s, "become"s, "house"s };
    ASSERT_EQUAL(words, estimated_outcome);
}

void TestMinusWordsOut()
{
    SearchServer server;
    server.AddDocument(1, "Cat in house"s, DocumentStatus::ACTUAL, { 4, 5, 1 });
    server.AddDocument(2, "Dog outside the house"s, DocumentStatus::ACTUAL, { 4, 5, 1 });
    server.AddDocument(3, "Bird sitting in the house"s, DocumentStatus::ACTUAL, { 4, 5, 1 });

    auto found_docs = server.FindTopDocuments("-in -on house"s);
    ASSERT_EQUAL(found_docs[0].id, 2);
}

void TestMatching()
{
    SearchServer server;
    server.AddDocument(1, "Cat in house"s, DocumentStatus::ACTUAL, { 4, 5, 1 });
    server.AddDocument(2, "Dog outside the house"s, DocumentStatus::ACTUAL, { 4, 5, 1 });
    server.AddDocument(3, "Bird sitting in the house"s, DocumentStatus::ACTUAL, { 4, 5, 1 });

    {
        auto [words, words_info] = server.MatchDocument("-Cat house"s, 1);
        ASSERT(words.empty());
    }
    {
        auto [words, words_info] = server.MatchDocument("Cat house"s, 1);
        ASSERT(!words.empty());
    }
    {
        auto [words, words_info] = server.MatchDocument("-outside house"s, 2);
        ASSERT(words.empty());
    }
    {
        auto [words, words_info] = server.MatchDocument("the"s, 2);
        ASSERT(!words.empty());
    }
    {
        auto [words, words_info] = server.MatchDocument("-Bird"s, 3);
        ASSERT(words.empty());
    }
}

void TestRelevanceSorting()
{
    SearchServer server;
    server.AddDocument(1, "Cat in house"s, DocumentStatus::ACTUAL, { 4, 5, 1 });
    server.AddDocument(2, "Dog outside the house"s, DocumentStatus::ACTUAL, { 4, 5, 1 });
    server.AddDocument(3, "Bird sitting in the house"s, DocumentStatus::ACTUAL, { 4, 5, 1 });

    {
        const vector<Document> found_documents_info = server.FindTopDocuments("Cat in house"s);
        ASSERT(found_documents_info[0].relevance > found_documents_info[1].relevance);
        ASSERT(found_documents_info[1].relevance > found_documents_info[2].relevance);
    }

    {
        const vector<Document> found_documents_info = server.FindTopDocuments("Bird in the house"s);
        ASSERT(found_documents_info[0].relevance > found_documents_info[1].relevance);
        ASSERT(found_documents_info[1].relevance > found_documents_info[2].relevance);
    }

}

void TestDocumentRatings()
{
    SearchServer server;
    server.AddDocument(1, "Cat in house"s, DocumentStatus::ACTUAL, { 4, 5, 1 });
    server.AddDocument(2, "Dog outside the house"s, DocumentStatus::ACTUAL, { 4, 5, 1, 3, 2, 1 });
    server.AddDocument(3, "Bird sitting in the house"s, DocumentStatus::ACTUAL, { 4, 5, 1, 4, 3, 4 });

    {
        const auto found_docs = server.FindTopDocuments("Cat in house"s);
        ASSERT_EQUAL(found_docs[0].rating, 3);
        ASSERT_EQUAL(found_docs[1].rating, 3);
        ASSERT_EQUAL(found_docs[2].rating, 2);
    }
}

void TestFilterSearchUsingPredicate()
{
    SearchServer server;
    server.AddDocument(1, "Cat in house"s, DocumentStatus::ACTUAL, { 1 });
    server.AddDocument(2, "Dog outside the house"s, DocumentStatus::ACTUAL, { 2 });
    server.AddDocument(3, "Bird sitting in the house"s, DocumentStatus::ACTUAL, { 0 });

    {
        const auto found_docs = server.FindTopDocuments("Cat laying in the house"s, [](int id, auto status, int rating) {return rating >= 1; });
        ASSERT_EQUAL(found_docs.size(), 2);
    }
}

void TestSeachingStatusDocuments()
{
    SearchServer server;
    server.AddDocument(1, "Cat in house"s, DocumentStatus::ACTUAL, { 1 });
    server.AddDocument(2, "Dog outside the house"s, DocumentStatus::IRRELEVANT, { 2 });
    server.AddDocument(3, "Bird sitting in the house"s, DocumentStatus::BANNED, { 0 });

    {
        const auto found_docs = server.FindTopDocuments("Cat laying in the house"s, DocumentStatus::BANNED);
        ASSERT_EQUAL(found_docs[0].id, 3);
    }

    {
        const auto found_docs = server.FindTopDocuments("Cat laying in the house"s, DocumentStatus::IRRELEVANT);
        ASSERT_EQUAL(found_docs[0].id, 2);
    }

    {
        const auto found_docs = server.FindTopDocuments("Cat laying in the house"s, DocumentStatus::ACTUAL);
        ASSERT_EQUAL(found_docs[0].id, 1);
    }
}

void TestRelevanceCalculation()
{
    SearchServer server;
    server.AddDocument(1, "Cat in house"s, DocumentStatus::ACTUAL, { 1 });
    server.AddDocument(2, "Dog outside the house"s, DocumentStatus::IRRELEVANT, { 2 });
    server.AddDocument(3, "Bird sitting in the house"s, DocumentStatus::BANNED, { 0 });

    {
        const auto found_docs = server.FindTopDocuments("Cat laying in the house"s, DocumentStatus::BANNED);
        ASSERT(found_docs[0].relevance - 0.162186 < EPSILON);
    }
}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestAddDocument);
    RUN_TEST(TestStopWords);
    RUN_TEST(TestMinusWordsOut);
    RUN_TEST(TestMatching);
    RUN_TEST(TestRelevanceSorting);
    RUN_TEST(TestDocumentRatings);
    RUN_TEST(TestFilterSearchUsingPredicate);
    RUN_TEST(TestSeachingStatusDocuments);
    RUN_TEST(TestRelevanceCalculation);

    // Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}
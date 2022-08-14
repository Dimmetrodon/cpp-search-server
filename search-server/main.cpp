// search_server_s1_t2_v2.cpp


#include <iostream>
#include "document.h"
#include "search_server.h"
#include "request_queue.h"
#include "paginator.h"
#include "read_input_functions.h"

using namespace std;


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
#include "string_processing.h"

using namespace std;

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
vector<string> SplitIntoWords(const std::string& text)
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

vector<string_view> SplitIntoWordsView(const string_view str) 
{
    vector<string_view> result;
    int64_t pos = str.find_first_not_of(" ");
    const int64_t pos_end = str.npos;

    while (pos != pos_end) 
    {
        int64_t space = str.find(' ', pos);
        result.push_back(space == pos_end ? str.substr(pos) : str.substr(pos, space - pos));
        pos = str.find_first_not_of(" ", space);
    }

    return result;
}

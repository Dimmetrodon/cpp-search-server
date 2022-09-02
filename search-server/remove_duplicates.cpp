#include "remove_duplicates.h"
#include "search_server.h"
using namespace std;

void RemoveDuplicates(SearchServer& search_server)
{
	
    vector<set<string>> checked_docs;
    vector<int> to_remove;
    for (auto& [id, words] : search_server.ids_to_words_)
    {
        if (count(checked_docs.begin(), checked_docs.end(), words))
        {
            to_remove.push_back(id);
        }
        else
        {
            checked_docs.push_back(words);
        }
    }

	for (auto it = to_remove.begin(); it != to_remove.end(); it = next(it))
	{
		cout << "Found duplicate document id "s << *it << "\n";
		search_server.RemoveDocument(*it);
	}
}

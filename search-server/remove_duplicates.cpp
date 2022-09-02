#include "remove_duplicates.h"
#include "search_server.h"
using namespace std;

void RemoveDuplicates(SearchServer& search_server)
{
	vector<int> to_remove = search_server.FindDuplicates();
	for (auto it = to_remove.begin(); it != to_remove.end(); it = next(it))
	{
		cout << "Found duplicate document id "s << *it << "\n";
		search_server.RemoveDocument(*it);
	}
}

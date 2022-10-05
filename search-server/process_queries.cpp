#include "process_queries.h"

using namespace std;

vector<vector<Document>> ProcessQueries(const SearchServer& search_server, const vector<string>& queries)
{
	vector<vector<Document>> result(queries.size());
	transform(execution::par,
  			  queries.begin(), queries.end(),
			  result.begin(),
			  [&search_server](const string& query){return search_server.FindTopDocuments(query);});
	return result;
}

deque<Document> ProcessQueriesJoined(const SearchServer& search_server, const vector<string>& queries)
{
	vector<vector<Document>> input_vector_vector = ProcessQueries(search_server, queries);
	deque<Document> result;
	//for (auto input_vector : input_vector_vector)
	//{
	//	//move(make_move_iterator(inputik.begin()), make_move_iterator(inputik.end()), result.end());
	//	for (auto input : input_vector)
	//	{
	//		result.push_back(move(input));
	//	}
	//}
	for (const auto& input_vector : input_vector_vector)
	{
		result.insert(result.end(), make_move_iterator(input_vector.begin()), make_move_iterator(input_vector.end()));
	}
	return result;
}
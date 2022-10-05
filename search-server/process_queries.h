#pragma once
#include "search_server.h"
#include "document.h"

#include<vector>
#include<execution>
#include<algorithm>
#include<string>
#include<deque>
#include<utility>

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, const std::vector<std::string>& queries);

std::deque<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries);
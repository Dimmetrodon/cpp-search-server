#pragma once
#include <string>
#include <iostream>
#include <vector>

std::string ReadLine();

int ReadLineWithNumber();

//Дробит строку в слова
std::vector<std::string> SplitIntoWords(const std::string& text);
std::vector<std::string_view> SplitIntoWordsView(std::string_view text);
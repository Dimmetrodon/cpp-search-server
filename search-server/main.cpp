// search_server_s1_t2_v2.cpp


#include <iostream>
#include "document.h"
#include "search_server.h"
#include "request_queue.h"
#include "paginator.h"
#include "read_input_functions.h"

using namespace std;

int main()
{
    setlocale(LC_ALL, "Russian");
    SearchServer search_server("� � ��"s);


    AddDocument(search_server, 1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    AddDocument(search_server, 1, "�������� �� � ������ �������"s, DocumentStatus::ACTUAL, { 1, 2 });
    AddDocument(search_server, -1, "�������� �� � ������ �������"s, DocumentStatus::ACTUAL, { 1, 2 });
    AddDocument(search_server, 3, "������� �� ����\x12��� �������"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
    AddDocument(search_server, 4, "������� �� ������� �������"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

    FindTopDocuments(search_server, "�������� -��"s);
    FindTopDocuments(search_server, "�������� --���"s);
    FindTopDocuments(search_server, "�������� -"s);
    FindTopDocuments(search_server, "��������\x12"s);

    MatchDocuments(search_server, "�������� ��"s);
    MatchDocuments(search_server, "������ -���"s);
    MatchDocuments(search_server, "������ --��"s);
    MatchDocuments(search_server, "�������� - �����"s);
}
# cpp-search-server
## Краткое описание
Проект представляет собой прообраз поисковой системы. Реализовывался с целью более глубокого изучения языка C++, отработки навыков написания нагруженных, эффективных, многопоточных приложений и явлется воплощением желания автора разработать нечто похожее на то, что в современных реалиях непрерывно сопровождает человека, приносит ощутимую пользу и обеспечивает комфорт. 

В процессе для удобной профилировки написанного и в целом для упражнения в написании RAII-кода был реализован класс [LogDuration](https://github.com/eugeneknvlv/cpp-search-server/blob/main/search-server/log_duration.h) с соответствующим набором макросов для удобного использования в разных частях программы.

Также с использованием мьютексов была реализована [обертка над контейнером std::map](https://github.com/eugeneknvlv/cpp-search-server/blob/main/search-server/concurrent_map.h), позволяющая избежать "состояния гонки" при работе в многопоточном режиме 

## Инструкция по развертыванию и пользованию
Требуемая версия языка - С++17 и выше. В остальном конкретных требований нет, компилятор подойдет любой: gcc, MinGW, Microsoft Visual C++.

Конструктор самого класса `SearchServer` принимает строку, содержащую стоп-слова, которые при добавлении документов в систему учитываться не должны. В файле `main.cpp` уже содержится генератор случайных документов, далее загружаемых в систему, и запросов, на которые система должна выдать ответ, отранжированный по релевантности, основанной на параметре [`TF-IDF`](https://en.wikipedia.org/wiki/Tf%E2%80%93idf) и рейтинге документа, строящимся на основе оценок пользователей. Запросы также могут содержать "минус-слова". Ответы на такие запросы не должны содержать этих слов (например, если потерялась собака в ошейнике, бессмысленно выдавать пользователю документы, сообщающие о находке животного без ошейника).

Добавление документов в систему осуществляется посредством метода `AddDocument`, принимающего id документа, сам текст документа в виде строки, его статус (актуальный, нерелевантный, забаненный, удаленный) и вектор целых чисел - оценок пользователей.

Обработку запроса производит метод `FindTopDocuments`, принимающий в качестве аргументов политику исполнения (`execution::seq`, `execution::par`) и сам запрос. Результатом работы является предоставление пользователю определенного числа наиболее релевантных документов.

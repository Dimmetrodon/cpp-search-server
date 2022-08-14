#pragma once
//Структура id, релевантность, рейтинг
struct Document
{
    Document();

    explicit Document(int in_id, double in_relevance, int in_rating);
    //Значения по умолчанию присваиваются в дефолтном конструкторе
    int id;
    double relevance;
    int rating;
};
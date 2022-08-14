#include "document.h"

    Document::Document()
        :id(0), relevance(0), rating(0)
    {
    }

    Document::Document(int in_id, double in_relevance, int in_rating)
        :id(in_id), relevance(in_relevance), rating(in_rating)
    {
    }


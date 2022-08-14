#pragma once
#include <vector>

template <typename Iterator>
class IteratorRange
{
public:
    IteratorRange(Iterator page_begin, Iterator page_end)
        : page_begin_(page_begin)
        , page_end_(page_end)
    {
    }

    Iterator begin() const
    {
        return page_begin_;
    }

    Iterator end() const
    {
        return page_end_;
    }

private:
    Iterator page_begin_;
    Iterator page_end_;
};


template<typename Iterator>
class Paginator
{
public:
    Paginator(Iterator begin, Iterator end, size_t page_size)
    {
        if (page_size == 1)
        {
            Iterator it = begin;
            int distance_ = std::distance(begin, end);
            for (int i = 1; i < distance_; i += page_size)
            {
                elements.push_back({ it, it + 1 });
                std::advance(it, page_size);
            }
            if (std::distance(begin, end) > page_size)
            {
                elements.push_back({ it , it + 1 });
            }
        }
        else
        {
            Iterator it = begin;
            int distance_ = std::distance(begin, end);
            if (!(distance_ % page_size))
            {
                int i = 1;
                do
                {
                    elements.push_back({ it, it + page_size - 1 });
                    std::advance(it, page_size);
                    i += page_size;
                } while (i < distance_);
            }
            else
            {
                int i = 1;
                do
                {
                    elements.push_back({ it, it + page_size - 1 });
                    std::advance(it, page_size);
                    i += page_size;
                } while (i < distance_);
                elements.push_back({ end - 1, end - 1 });
            }

        }
    }

    auto begin() const
    {
        return elements.begin();
    }

    auto end() const
    {
        return elements.end();
    }

private:
    std::vector<IteratorRange<Iterator>> elements;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size)
{
    return Paginator(begin(c), end(c), page_size);
}

template <typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator> iterators)
{
    for (auto it = iterators.begin(); it <= iterators.end(); ++it)
    {
        out << *it;
    }
    return out;
}

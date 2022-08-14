#include "request_queue.h"
using namespace std;

RequestQueue::RequestQueue(const SearchServer& search_server)
    :search_server_(search_server)
{
    // напишите реализацию
}

int RequestQueue::GetNoResultRequests() const
{
    // напишите реализацию
    int empty_results = 0;
    for (auto it = requests_.begin(); it != requests_.end(); it = next(it))
    {
        if (it->results == 0)
        {
            empty_results += 1;
        }
    }
    return empty_results;
}
void RequestQueue::MinsCheck()
{
    if (current_mins_ != (min_in_day_))
    {
        current_mins_ += 1;
    }
    else
    {
        day_ = true;
        current_mins_ = 1;
    }
}

void RequestQueue::MoveDeque()
{
    if (day_)
    {
        requests_.pop_front();
    }
}
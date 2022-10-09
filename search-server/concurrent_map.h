#pragma once
#include <vector>
#include <map>
#include <future>
#include <algorithm>

template <typename Key, typename Value>
class ConcurrentMap
{
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

    struct Access
    {
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;
    };

    explicit ConcurrentMap(size_t bucket_count)
        :bucket_count_(bucket_count)
        , maps(bucket_count)
        , mutex_values(bucket_count)
    {
    }

    Access operator[](const Key& key)
    {
        int map_num = static_cast<uint64_t>(key) % bucket_count_;
        return { std::lock_guard(mutex_values[map_num]), maps[map_num][key] };
    }

    std::map<Key, Value> BuildOrdinaryMap()
    {
        std::map<Key, Value> result_map;
        for (size_t i = 0; i != bucket_count_; ++i)
        {
            std::lock_guard guard(mutex_values[i]);
            result_map.insert(maps[i].begin(), maps[i].end());
        }
        return result_map;
    }

private:
    size_t bucket_count_;
    std::vector<std::map<Key, Value>> maps;
    std::vector<std::mutex> mutex_values;
};
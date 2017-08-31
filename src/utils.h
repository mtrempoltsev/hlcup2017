#pragma once

#include <chrono>

#include <boost/date_time/posix_time/ptime.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

boost::posix_time::ptime from_time_t(std::time_t value);
std::time_t my_to_time_t(boost::posix_time::ptime time);

bool to_uint(const std::string& text, uint32_t& result);
bool to_int(const std::string& text, int32_t& result);

struct timer final
{
public:
    timer();
    ~timer();

private:
    const std::chrono::high_resolution_clock::time_point start_;
};

template <class Key, class Value, class Map>
void erase_value_from_multimap(const Key& key, const Value& value_to_erase, Map& multimap)
{
    auto it = multimap.find(key);
    const auto end = multimap.end();
    while (it != end && it->first == key)
    {
        if (it->second == value_to_erase)
        {
            multimap.erase(it);
            break;
        }
        ++it;
    }
}

std::string percent_decode(const char* in);

template <class X, class Y>
void erase(std::vector<X>& data, const Y& value)
{
    auto& entity = data[value];
    const auto it = std::lower_bound(entity.begin(), entity.end(), value);
    entity.erase(it);
}

template <class X, class Y>
void insert(std::vector<X>& data, const Y& value)
{
    auto& entity = data[value];
    const auto it = std::upper_bound(entity.begin(), entity.end(), value);
    entity.insert(it, value);
}

template <class T, class It>
class sorted_slice
{
public:
    sorted_slice() = default;

    sorted_slice(It begin, It end)
    {
        while (begin != end)
            data_.push_back((begin++)->second);
        std::sort(data_.begin(), data_.end());
    }

    bool has(const T& x) const
    {
        return std::binary_search(data_.begin(), data_.end(), x);
    }

private:
    std::vector<T> data_;
};

template <class X, class Y>
class multidictionary final
{
    using pair = std::pair<X, Y>;
    using vector = std::vector<pair>;
    using iterator = typename vector::const_iterator;

    struct left_pair_camparer final
    {
        bool operator()(const pair& a, const X& b) const
        {
            return a.first < b;
        }
    };

    struct right_pair_camparer final
    {
        bool operator()(const X &a, const pair& b) const
        {
            return a < b.first;
        }
    };

public:
    iterator begin() const
    {
        return data_.begin();
    }

    iterator end() const
    {
        return data_.end();
    }

    iterator lower_bound(const X& x) const
    {
        return std::lower_bound(data_.cbegin(), data_.cend(), x, left_pair_camparer());
    }

    iterator upper_bound(const X& x) const
    {
        return std::upper_bound(data_.cbegin(), data_.cend(), x, right_pair_camparer());
    }

    void push_back(const X& x, const Y& y)
    {
        data_.push_back(std::make_pair(x, y));
    }

    void sort()
    {
        std::sort(data_.begin(), data_.end(), [](const pair& a, const pair& b) { return a.first < b.first; });
    }

    sorted_slice<Y, iterator> empty_slice() const
    {
        return sorted_slice<Y, iterator >();
    };

    sorted_slice<Y, iterator> slice_to_upper(const X& x) const
    {
        return sorted_slice<Y, iterator>(data_.cbegin(), upper_bound(x));
    }

    sorted_slice<Y, iterator> slice_of_equal(const X& x) const
    {
        const auto end = data_.cend();
        const auto first = std::lower_bound(data_.cbegin(), end, x, left_pair_camparer());
        auto last = first;
        while (last != end && last->first == x)
        {
            ++last;
        }
        return sorted_slice<Y, iterator>(first, last);
    }

    void insert(const X& key, const Y& value)
    {
        const auto end = data_.cend();
        auto it = std::upper_bound(data_.cbegin(), end, key, right_pair_camparer());
        data_.insert(it, std::make_pair(key, value));
    }

    void erase(const X& key, const Y& value_to_erase)
    {
        const auto end = data_.cend();
        auto it = std::lower_bound(data_.cbegin(), end, key, left_pair_camparer());

        while (it != end && it->first == key)
        {
            if (it->second == value_to_erase)
            {
                data_.erase(it);
                break;
            }
            ++it;
        }
    }

private:
    vector data_;
};

template <class T, class It>
sorted_slice<T, It> make_slice(It begin, It end)
{
    return sorted_slice<T, It>(begin, end);
};

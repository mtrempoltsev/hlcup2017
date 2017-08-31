#include <iostream>

#include "utils.h"

boost::posix_time::ptime from_time_t(std::time_t value)
{
    static const boost::posix_time::ptime start(boost::gregorian::date(1970, 1, 1));
    return start + boost::posix_time::seconds(value);
}

std::time_t my_to_time_t(boost::posix_time::ptime time)
{
    static const boost::posix_time::ptime start(boost::gregorian::date(1970, 1, 1));
    const auto duration = time - start;
    return duration.total_seconds();
}

bool to_uint(const std::string& text, uint32_t& result)
{
    if (text.empty())
        return false;

    int64_t number = 0;
    int size = 0;
    char c = 0;

    for (char c : text)
    {
        if (c < '0' || c > '9')
            return false;

        if (number == 0 && c == '0')
            continue;

        number = number * 10 + (c - '0');
        ++size;
        if (size > 10)
            return false;
    }

    if (number > static_cast<int64_t>(std::numeric_limits<uint32_t>::max()))
        return false;

    result = static_cast<uint32_t>(number);
    return true;
}

bool to_int(const std::string& text, int32_t& result)
{
    if (text.empty())
        return false;

    int64_t number = 0;
    int size = 0;
    char c = 0;

    const bool is_negative = text[0] == '-';

    for (size_t i = is_negative ? 1 : 0, length = text.length(); i < length; ++i)
    {
        const char c = text[i];

        if (c < '0' || c > '9')
            return false;

        if (number == 0 && c == '0')
            continue;

        number = number * 10 + (c - '0');
        ++size;
        if (size > 10)
            return false;
    }

    if (is_negative)
        number = -number;

    if (number > static_cast<int32_t>(std::numeric_limits<int32_t>::max()))
        return false;

    if (number < static_cast<int32_t>(std::numeric_limits<int32_t>::min()))
        return false;

    result = static_cast<int32_t>(number);
    return true;
}

timer::timer()
    : start_(std::chrono::high_resolution_clock::now())
{
}

timer::~timer()
{
    const auto finish = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(finish - start_).count() << "us" << std::endl;
}

std::string percent_decode(const char* in)
{
    static const char char_table[256] =
    {
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        0, 1, 2, 3, 4, 5, 6, 7,  8, 9,-1,-1,-1,-1,-1,-1,
        -1,10,11,12,13,14,15,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,10,11,12,13,14,15,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1
    };

    std::string result;

    if (in == nullptr)
        return result;

    char c = 0;
    while ((c = *in++) != '\0')
    {
        if (c == '+')
        {
            result += ' ';
            continue;
        }
        else if (c == '%')
        {
            char v1 = 0;
            char v2 = 0;
            if ((v1 = char_table[static_cast<unsigned char>(*in++)]) < 0
                || (v2 = char_table[static_cast<unsigned char>(*in++)]) < 0)
            {
                return result; // error
            }
            result += (v1 << 4) | v2;
        }
        else
            result += c;
    }

    return result;
}

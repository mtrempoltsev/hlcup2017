#include "utils.h"

#include "url_parser.h"

using key_value_list_t = std::unordered_map<std::string, std::string>;

#define PARSING_ERROR(n) { status_  = status::error_##n; return; }
#define CHECK_WHAT(x) if (*c++ != x) PARSING_ERROR(404)
#define GET_NUMBER \
    const auto n = get_number(c); \
    if (n == -1) PARSING_ERROR(404) \
    id_ = static_cast<uint32_t>(n);

#define CHECK_WHAT_QUERY_ID \
    ++c; \
    CHECK_WHAT('q') \
    CHECK_WHAT('u') \
    CHECK_WHAT('e') \
    CHECK_WHAT('r') \
    CHECK_WHAT('y') \
    CHECK_WHAT('_') \
    CHECK_WHAT('i') \
    CHECK_WHAT('d') \
    CHECK_WHAT('=') \
    while (char symbol = *c++) \
    { \
        if (symbol < '0' || symbol > '9') \
            PARSING_ERROR(400) \
    }

url_parser::url_parser(const char* url)
    : status_(status::ok)
    , id_(0)
{
    auto c = url;

    CHECK_WHAT('/')
    switch (*c++)
    {
        case 'u':
        {
            CHECK_WHAT('s')
            CHECK_WHAT('e')
            CHECK_WHAT('r')
            CHECK_WHAT('s')
            CHECK_WHAT('/')
            if (*c == 'n')
            {
                CHECK_WHAT('n')
                CHECK_WHAT('e')
                CHECK_WHAT('w')
                if (!*c)
                {
                    entity_ = entity::new_user;
                    return;
                }
                CHECK_WHAT_QUERY_ID
                entity_ = entity::new_user;
                return;
            }
            GET_NUMBER
            if (!*c)
            {
                entity_ = entity::user;
                return;
            }
            if (*c == '?')
            {
                CHECK_WHAT_QUERY_ID
                    entity_ = entity::user;
                return;
            }
            CHECK_WHAT('/')
            CHECK_WHAT('v')
            CHECK_WHAT('i')
            CHECK_WHAT('s')
            CHECK_WHAT('i')
            CHECK_WHAT('t')
            CHECK_WHAT('s')
            entity_ = entity::user_visits;
            if (!*c)
                return;
            CHECK_WHAT('?')
            if (!parse_query(c))
                PARSING_ERROR(400)
            return;
        }
        case 'l':
        {
            CHECK_WHAT('o')
            CHECK_WHAT('c')
            CHECK_WHAT('a')
            CHECK_WHAT('t')
            CHECK_WHAT('i')
            CHECK_WHAT('o')
            CHECK_WHAT('n')
            CHECK_WHAT('s')
            CHECK_WHAT('/')
            if (*c == 'n')
            {
                CHECK_WHAT('n')
                CHECK_WHAT('e')
                CHECK_WHAT('w')
                if (!*c)
                {
                    entity_ = entity::new_location;
                    return;
                }
                CHECK_WHAT_QUERY_ID
                entity_ = entity::new_location;
                return;
            }
            GET_NUMBER
            if (!*c)
            {
                entity_ = entity::location;
                return;
            }
            if (*c == '?')
            {
                CHECK_WHAT_QUERY_ID
                    entity_ = entity::location;
                return;
            }
            CHECK_WHAT('/')
            CHECK_WHAT('a')
            CHECK_WHAT('v')
            CHECK_WHAT('g')
            entity_ = entity::location_mark;
            if (!*c)
                return;
            CHECK_WHAT('?')
            if (!parse_query(c))
                PARSING_ERROR(400)
            return;
        }
        case 'v':
        {
            CHECK_WHAT('i')
            CHECK_WHAT('s')
            CHECK_WHAT('i')
            CHECK_WHAT('t')
            CHECK_WHAT('s')
            CHECK_WHAT('/')
            if (*c == 'n')
            {
                CHECK_WHAT('n')
                CHECK_WHAT('e')
                CHECK_WHAT('w')
                if (!*c)
                {
                    entity_ = entity::new_visit;
                    return;
                }
                CHECK_WHAT_QUERY_ID
                entity_ = entity::new_visit;
                return;
            }
            GET_NUMBER
            if (!*c)
            {
                entity_ = entity::visit;
                return;
            }
            if (*c == '?')
            {
                CHECK_WHAT_QUERY_ID
                entity_ = entity::visit;
                return;
            }
            PARSING_ERROR(404)
            break;
        }
    };

    status_ = status::error_404;
}

int64_t url_parser::get_number(const char*& c) const
{
    if (!*c)
        return -1;

    int64_t number = 0;
    int size = 0;
    char letter = 0;

    while ((letter = *c++) && (letter >= '0' && letter <= '9'))
    {
        if (number == 0 && letter == '0')
            continue;

        number = number * 10 + (letter - '0');
        ++size;
        if (size > 10)
            return -1;
    }

    --c;

    return number <= static_cast<int64_t>(std::numeric_limits<uint32_t>::max())
        ? number
        : -1;
}

bool url_parser::parse_query(const char* c)
{
    if (!*c)
        return false;

    const auto decoded = percent_decode(c);
    c = decoded.c_str();

    enum class state
    {
        key,
        value
    };

    state s = state::key;

    std::string key;
    std::string value;

    char symbol = 0;
    while (symbol = *c++)
    {
        switch (s)
        {
        case state::key:
            if (symbol == '=')
            {
                if (key.empty()) return false;
                s = state::value;
            }
            else key += symbol;
            break;
        case state::value:
            if (symbol == '&')
            {
                if (value.empty()) return false;
                if (!add_value(std::move(key), std::move(value)))
                    return false;
                key.clear();
                value.clear();
                s = state::key;
            }
            else value += symbol;
            break;
        };
    }

    if (s == state::key)
        return false;

    if (value.empty())
        return false;

    return add_value(std::move(key), std::move(value));
}

bool url_parser::add_value(std::string&& key, std::string&& value)
{
    auto it = arguments_.find(key);
    if (it != arguments_.end())
        return false;

    arguments_.emplace(key, value);
    return true;
}

#undef PARSING_ERROR
#undef CHECK_WHAT
#undef GET_NUMBER
#undef CHECK_WHAT_QUERY_ID

#pragma once

#include <unordered_map>

using key_value_list_t = std::unordered_map<std::string, std::string>;

struct url_parser final
{
public:
    explicit url_parser(const char* url);

    enum class status
    {
        ok,
        error_400,
        error_404
    };

    status status_;

    enum class entity
    {
        user,
        location,
        visit,
        user_visits,
        location_mark,
        new_user,
        new_location,
        new_visit
    };

    entity entity_;
    uint32_t id_;

    key_value_list_t arguments_;

private:
    int64_t get_number(const char*& c) const;
    bool parse_query(const char* c);
    bool add_value(std::string&& key, std::string&& value);
};

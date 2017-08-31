#pragma once

#include "types.h"

#include "rapidjson/encodings.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/reader.h"

struct one_user_handler final
{
    bool Null() { return false; }
    bool Bool(bool b) { return false; }
    bool Int64(int64_t i) { return false; }
    bool Uint64(uint64_t u) { return false; }
    bool Double(double d) { return false; }
    bool RawNumber(const char* str, rapidjson::SizeType length, bool copy) { return false; }
    bool StartArray() { return false; }
    bool EndArray(rapidjson::SizeType elementCount) { return false; }

    bool Int(int i);
    bool Uint(unsigned u);
    bool String(const char* str, rapidjson::SizeType length, bool copy);
    bool Key(const char* str, rapidjson::SizeType length, bool /*copy*/);
    bool StartObject();
    bool EndObject(rapidjson::SizeType memberCount);

    bool skip_id_ = false;

    user_t entity_;

private:
    enum class states
    {
        id,
        email,
        first_name,
        last_name,
        gender,
        birth_date
    };

    states state_;
};

struct one_location_handler final
{
    bool Null() { return false; }
    bool Bool(bool b) { return false; }
    bool Int64(int64_t i) { return false; }
    bool Uint64(uint64_t u) { return false; }
    bool Double(double d) { return false; }
    bool RawNumber(const char* str, rapidjson::SizeType length, bool copy) { return false; }
    bool Int(int i) { return false; }
    bool StartArray() { return false; }
    bool EndArray(rapidjson::SizeType elementCount) { return false; }

    bool Uint(unsigned u);
    bool String(const char* str, rapidjson::SizeType length, bool copy);
    bool Key(const char* str, rapidjson::SizeType length, bool copy);
    bool StartObject();
    bool EndObject(rapidjson::SizeType memberCount);

    bool skip_id_ = false;

    location_t entity_;

private:
    enum class states
    {
        id,
        place,
        country,
        city,
        distance
    };

    states state_;
};

struct one_visit_handler final
{
    bool Null() { return false; }
    bool Bool(bool b) { return false; }
    bool Int64(int64_t i) { return false; }
    bool Uint64(uint64_t u) { return false; }
    bool Double(double d) { return false; }
    bool RawNumber(const char* str, rapidjson::SizeType length, bool copy) { return false; }
    bool Int(int i) { return false; }
    bool String(const char* str, rapidjson::SizeType length, bool copy) { return false; }
    bool StartArray() { return false; }
    bool EndArray(rapidjson::SizeType elementCount) { return false; }

    bool Uint(unsigned u);
    bool Key(const char* str, rapidjson::SizeType length, bool copy);
    bool StartObject();
    bool EndObject(rapidjson::SizeType memberCount);

    bool skip_id_ = false;

    visit_t entity_;

private:
    enum class states
    {
        id,
        location,
        user,
        visited_at,
        mark
    };

    states state_;
};

template <class T, class HandlerT>
struct entity_list_handler
{
    bool Null() { return handler_.Null(); }
    bool Bool(bool b) { return handler_.Bool(b); }
    bool Int64(int64_t i) { return handler_.Int64(i); }
    bool Uint64(uint64_t u) { return handler_.Uint64(u); }
    bool Double(double d) { return handler_.Double(d); }
    bool RawNumber(const char* str, rapidjson::SizeType length, bool copy) { return handler_.RawNumber(str, length, copy); }
    bool Int(int i) { return handler_.Int(i); }
    bool Uint(unsigned u) { return handler_.Uint(u); }
    bool String(const char* str, rapidjson::SizeType length, bool copy) { return handler_.String(str, length, copy); }
    bool Key(const char* str, rapidjson::SizeType length, bool copy) { return handler_.Key(str, length, copy); }

    bool StartObject()
    {
        ++objects_;
        return handler_.StartObject();
    }

    bool EndObject(rapidjson::SizeType memberCount)
    {
        if (--objects_ < 1) // root
            return true;

        if (!handler_.EndObject(memberCount))
            return false;

        if (max_id_ < handler_.entity_.id_)
            max_id_ = handler_.entity_.id_;

        entities_[handler_.entity_.id_] = handler_.entity_;
        return true;
    }

    bool StartArray()
    {
        return true;
    }

    bool EndArray(rapidjson::SizeType elementCount)
    {
        return true;
    }

    using list_t = std::vector<T>;

    explicit entity_list_handler(list_t& entities, size_t& max_id)
        : entities_(entities)
        , max_id_(max_id)
        , objects_(0)
    {
    }

    list_t& entities_;
    size_t& max_id_;
    HandlerT handler_;
    int objects_;
};

using users_handler = entity_list_handler<user_t, one_user_handler>;
using locations_handler = entity_list_handler<location_t, one_location_handler>;
using visits_handler = entity_list_handler<visit_t, one_visit_handler>;

char* save(const user_t& user, char* buffer);
char* save(const location_t& location, char* buffer);
char* save(const visit_t& visit, char* buffer);
char* save(double average, char* buffer);
char* save(const visit_description_list& visits, char* buffer);

#include "in_place_writer.h"

#include "json.h"

#define BEGIN_CHECK { int n = 1;
#define CHECK_WHAT(c) if (*str++ != c) return false; else ++n;
#define END_CHECK if (n != length) return false; }

bool one_user_handler::Int(int i)
{
    if (state_ != states::birth_date) return false;
    entity_.birth_date_ = i;
    return true;
}

bool one_user_handler::Uint(unsigned u)
{
    if (state_ == states::id)
        entity_.id_ = u;
    else if (state_ == states::birth_date)
        entity_.birth_date_ = u;
    else
        return false;
    return true;
}

bool one_user_handler::String(const char* str, rapidjson::SizeType length, bool copy)
{
    if (state_ == states::first_name)
        entity_.first_name_.assign(str, length);
    else if (state_ == states::last_name)
        entity_.second_name_.assign(str, length);
    else if (state_ == states::email)
        entity_.email_.assign(str, length);
    else if (state_ == states::gender)
        entity_.gender_ = str[0] == 'm' ? gender_type::male : gender_type::female;
    else
        return false;
    return true;
}

bool one_user_handler::Key(const char* str, rapidjson::SizeType length, bool /*copy*/)
{
    switch (*str++)
    {
    case 'i':
        if (skip_id_) return false;
        BEGIN_CHECK
        CHECK_WHAT('d')
        END_CHECK
        state_ = states::id;
        break;
    case 'e':
        BEGIN_CHECK
        CHECK_WHAT('m')
        CHECK_WHAT('a')
        CHECK_WHAT('i')
        CHECK_WHAT('l')
        END_CHECK
        state_ = states::email;
        break;
    case 'f':
        BEGIN_CHECK
        CHECK_WHAT('i')
        CHECK_WHAT('r')
        CHECK_WHAT('s')
        CHECK_WHAT('t')
        CHECK_WHAT('_')
        CHECK_WHAT('n')
        CHECK_WHAT('a')
        CHECK_WHAT('m')
        CHECK_WHAT('e')
        END_CHECK
        state_ = states::first_name;
        break;
    case 'l':
        BEGIN_CHECK
        CHECK_WHAT('a')
        CHECK_WHAT('s')
        CHECK_WHAT('t')
        CHECK_WHAT('_')
        CHECK_WHAT('n')
        CHECK_WHAT('a')
        CHECK_WHAT('m')
        CHECK_WHAT('e')
        END_CHECK
        state_ = states::last_name;
        break;
    case 'g':
        BEGIN_CHECK
        CHECK_WHAT('e')
        CHECK_WHAT('n')
        CHECK_WHAT('d')
        CHECK_WHAT('e')
        CHECK_WHAT('r')
        END_CHECK
        state_ = states::gender;
        break;
    case 'b':
        BEGIN_CHECK
        CHECK_WHAT('i')
        CHECK_WHAT('r')
        CHECK_WHAT('t')
        CHECK_WHAT('h')
        CHECK_WHAT('_')
        CHECK_WHAT('d')
        CHECK_WHAT('a')
        CHECK_WHAT('t')
        CHECK_WHAT('e')
        END_CHECK
        state_ = states::birth_date;
        break;
    case 'u':
        // parent
        BEGIN_CHECK
        CHECK_WHAT('s')
        CHECK_WHAT('e')
        CHECK_WHAT('r')
        CHECK_WHAT('s')
        END_CHECK
        break;
    default:
        return false;
    }
    return true;
}

bool one_user_handler::StartObject()
{
    return true;
}

bool one_user_handler::EndObject(rapidjson::SizeType memberCount)
{
    if (skip_id_)
        return true;
    return memberCount == 6;
}

bool one_location_handler::Uint(unsigned u)
{
    if (state_ == states::id)
        entity_.id_ = u;
    else if (state_ == states::distance)
        entity_.distance_ = u;
    else
        return false;
    return true;
}

bool one_location_handler::String(const char* str, rapidjson::SizeType length, bool copy)
{
    if (state_ == states::place)
        entity_.place_ = str;
    else if (state_ == states::country)
        entity_.country_ = str;
    else if (state_ == states::city)
        entity_.city_ = str;
    else
        return true;
    return true;
}

bool one_location_handler::Key(const char* str, rapidjson::SizeType length, bool copy)
{
    switch (*str++)
    {
    case 'i':
        if (skip_id_) return false;
        BEGIN_CHECK
        CHECK_WHAT('d')
        END_CHECK
        state_ = states::id;
        break;
    case 'p':
        BEGIN_CHECK
        CHECK_WHAT('l')
        CHECK_WHAT('a')
        CHECK_WHAT('c')
        CHECK_WHAT('e')
        END_CHECK
        state_ = states::place;
        break;
    case 'c':
        switch (*str++)
        {
        case 'i':
            BEGIN_CHECK
            ++n;
            CHECK_WHAT('t')
            CHECK_WHAT('y')
            END_CHECK
            state_ = states::city;
            break;
        case 'o':
            BEGIN_CHECK
            ++n;
            CHECK_WHAT('u')
            CHECK_WHAT('n')
            CHECK_WHAT('t')
            CHECK_WHAT('r')
            CHECK_WHAT('y')
            END_CHECK
            state_ = states::country;
            break;
        default:
            return false;
        }
        break;
    case 'd':
        BEGIN_CHECK
        CHECK_WHAT('i')
        CHECK_WHAT('s')
        CHECK_WHAT('t')
        CHECK_WHAT('a')
        CHECK_WHAT('n')
        CHECK_WHAT('c')
        CHECK_WHAT('e')
        END_CHECK
        state_ = states::distance;
        break;
    case 'l':
        // parent
        BEGIN_CHECK
        CHECK_WHAT('o')
        CHECK_WHAT('c')
        CHECK_WHAT('a')
        CHECK_WHAT('t')
        CHECK_WHAT('i')
        CHECK_WHAT('o')
        CHECK_WHAT('n')
        CHECK_WHAT('s')
        END_CHECK
        break;
    default:
        return false;
    }
    return true;
}

bool one_location_handler::StartObject()
{
    return true;
}

bool one_location_handler::EndObject(rapidjson::SizeType memberCount)
{
    if (skip_id_)
        return true;
    return memberCount == 5;
}

bool one_visit_handler::Uint(unsigned u)
{
    if (state_ == states::id)
        entity_.id_ = u;
    else if (state_ == states::location)
        entity_.location_ = u;
    else if (state_ == states::user)
        entity_.user_ = u;
    else if (state_ == states::visited_at)
        entity_.visited_at_ = u;
    else if (state_ == states::mark)
        entity_.mark_ = u;
    else
        return false;
    return true;
}

bool one_visit_handler::Key(const char* str, rapidjson::SizeType length, bool copy)
{
    switch (*str++)
    {
    case 'i':
        if (skip_id_) return false;
        BEGIN_CHECK
        CHECK_WHAT('d')
        END_CHECK
        state_ = states::id;
        break;
    case 'l':
        BEGIN_CHECK
        CHECK_WHAT('o')
        CHECK_WHAT('c')
        CHECK_WHAT('a')
        CHECK_WHAT('t')
        CHECK_WHAT('i')
        CHECK_WHAT('o')
        CHECK_WHAT('n')
        END_CHECK
        state_ = states::location;
        break;
    case 'u':
        BEGIN_CHECK
        CHECK_WHAT('s')
        CHECK_WHAT('e')
        CHECK_WHAT('r')
        END_CHECK
        state_ = states::user;
        break;
    case 'v':
        BEGIN_CHECK
        CHECK_WHAT('i')
        CHECK_WHAT('s')
        CHECK_WHAT('i')
        CHECK_WHAT('t')
        if (*str == 's')
        {
            return true; // parent
        }
        CHECK_WHAT('e')
        CHECK_WHAT('d')
        CHECK_WHAT('_')
        CHECK_WHAT('a')
        CHECK_WHAT('t')
        END_CHECK
        state_ = states::visited_at;
        break;
    case 'm':
        // parent
        BEGIN_CHECK
        CHECK_WHAT('a')
        CHECK_WHAT('r')
        CHECK_WHAT('k')
        END_CHECK
        state_ = states::mark;
        break;
    default:
        return false;
    }
    return true;
}

bool one_visit_handler::StartObject()
{
    return true;
}

bool one_visit_handler::EndObject(rapidjson::SizeType memberCount)
{
    if (skip_id_)
        return true;
    return memberCount == 5;
}

char* save(const user_t& user, char* buffer)
{
    buffer = write(buffer, R"({"id":)");
    buffer = write(buffer, user.id_);
    buffer = write(buffer, R"(,"email":")");
    buffer = write(buffer, user.email_.c_str());
    buffer = write(buffer, R"(","first_name":")");
    buffer = write(buffer, user.first_name_.c_str());
    buffer = write(buffer, R"(","last_name":")");
    buffer = write(buffer, user.second_name_.c_str());
    buffer = write(buffer, R"(","gender":")");
    buffer = write(buffer, user.gender_ == gender_type::male ? 'm' : 'f');
    buffer = write(buffer, R"(","birth_date":)");
    buffer = write(buffer, user.birth_date_);
    buffer = write(buffer, '}');
    return buffer;
}

char* save(const location_t& location, char* buffer)
{
    buffer = write(buffer, R"({"id":)");
    buffer = write(buffer, location.id_);
    buffer = write(buffer, R"(,"place":")");
    buffer = write(buffer, location.place_.c_str());
    buffer = write(buffer, R"(","country":")");
    buffer = write(buffer, location.country_.c_str());
    buffer = write(buffer, R"(","city":")");
    buffer = write(buffer, location.city_.c_str());
    buffer = write(buffer, R"(","distance":)");
    buffer = write(buffer, location.distance_);
    buffer = write(buffer, '}');
    return buffer;
}

char* save(const visit_t& visit, char* buffer)
{
    buffer = write(buffer, R"({"id":)");
    buffer = write(buffer, visit.id_);
    buffer = write(buffer, R"(,"location":)");
    buffer = write(buffer, visit.location_);
    buffer = write(buffer, R"(,"user":)");
    buffer = write(buffer, visit.user_);
    buffer = write(buffer, R"(,"visited_at":)");
    buffer = write(buffer, visit.visited_at_);
    buffer = write(buffer, R"(,"mark":)");
    buffer = write(buffer, visit.mark_);
    buffer = write(buffer, '}');
    return buffer;
}

char* save(double average, char* buffer)
{
    buffer = write(buffer, R"({"avg":)");
    buffer = write(buffer, static_cast<int>(average * 100000 + 0.5) / 100000.0);
    buffer = write(buffer, '}');
    return buffer;
}

char* save(const visit_description_list& visits, char* buffer)
{
    buffer = write(buffer, R"({"visits":[)");
    bool first = true;
    for (const auto& visit : visits)
    {
        if (first)
            first = false;
        else
            buffer = write(buffer, ',');
        buffer = write(buffer, R"({"mark":)");
        buffer = write(buffer, visit.mark_);
        buffer = write(buffer, R"(,"visited_at":)");
        buffer = write(buffer, visit.visited_at_);
        buffer = write(buffer, R"(,"place":")");
        buffer = write(buffer, visit.place_.c_str());
        buffer = write(buffer, R"("})");
    }
    buffer = write(buffer, "]}");
    return buffer;
}

#undef BEGIN_CHECK
#undef CHECK_WHAT
#undef END_CHECK

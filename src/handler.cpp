#include "handler.h"

const_memory_block make_json_response(const_memory_block json, bool keep_alive, memory_block buffer)
{
    auto ptr = buffer.data_;

    const auto headers = make_response(response_template::default_headers, keep_alive);
    std::copy(headers.data_, headers.data_ + headers.size_, ptr);
    ptr += headers.size_;

    const auto length = std::to_string(json.size_);
    std::copy(length.c_str(), length.c_str() + length.size(), ptr);
    ptr += length.size();

    const uint32_t rnrn = 0x0a0d0a0d;
    *reinterpret_cast<uint32_t*>(ptr) = rnrn;
    ptr += sizeof(rnrn);

    std::copy(json.data_, json.data_ + json.size_, ptr);
    ptr += json.size_;

    return const_memory_block { buffer.data_, static_cast<size_t>(ptr - buffer.data_) };
}

template <class T>
const_memory_block save_json_and_make_response(const T& entity, bool keep_alive, memory_block buffer)
{
    const auto headers = make_response(response_template::default_headers, keep_alive);
    const auto headers_size = headers.size_;

    const auto length_size = 5;

    const uint32_t rnrn = 0x0a0d0a0d;
    const auto separartor_size = sizeof(rnrn);

    char* const json_begin = buffer.data_ + headers_size + length_size + separartor_size;
    char* const json_end = save(entity, json_begin);
    const auto json_size = json_end - json_begin;

    char* const separator_begin = json_begin - separartor_size;
    *reinterpret_cast<uint32_t*>(separator_begin) = rnrn;

    const auto length = std::to_string(json_size);
    char* const length_begin = separator_begin - length.size();
    std::copy(length.c_str(), length.c_str() + length.size(), length_begin);

    char* const header_begin = length_begin - headers_size;
    std::copy(headers.data_, headers.data_ + headers.size_, header_begin);

    return const_memory_block { header_begin, static_cast<size_t>(json_end - header_begin) };
}

template <class T>
const_memory_block get_entity(data_base* db, bool keep_alive, uint32_t id, memory_block buffer)
{
    T entity;
    if (!db->select(entity, id))
    {
        return make_response(response_template::error_404, keep_alive);
    }

    return save_json_and_make_response(entity, keep_alive, buffer);
}

template <class T, class Handler>
const_memory_block update_entity(data_base* db, bool keep_alive, uint32_t id, const_memory_block body)
{
    T entity;
    if (!db->select(entity, id))
    {
        return make_response(response_template::error_404, keep_alive);
    }

    Handler handler;
    handler.skip_id_ = true;
    handler.entity_ = entity;

    rapidjson::Reader reader;
    rapidjson::MemoryStream stream(body.data_, body.size_);
    reader.Parse(stream, handler);
    if (reader.HasParseError())
    {
        return make_response(response_template::error_400, keep_alive);
    }

    if (!db->update(id, handler.entity_))
    {
        return make_response(response_template::error_404, keep_alive);
    }

    return make_response(response_template::post_ok, keep_alive);
}

template <class T, class Handler>
const_memory_block add_entity(data_base* db, bool keep_alive, const_memory_block body)
{
    Handler handler;

    rapidjson::Reader reader;
    rapidjson::MemoryStream stream(body.data_, body.size_);
    reader.Parse(stream, handler);
    if (reader.HasParseError())
    {
        return make_response(response_template::error_400, keep_alive);
    }

    if (!db->add(handler.entity_))
    {
        return make_response(response_template::error_400, keep_alive);
    }

    return make_response(response_template::post_ok, keep_alive);
}

const_memory_block get_user_visits(data_base* db, bool keep_alive, uint32_t id, const key_value_list_t& arguments, memory_block buffer)
{
    auto from_it = arguments.find("fromDate");
    bool has_from = from_it != arguments.end();

    auto to_it = arguments.find("toDate");
    bool has_to = to_it != arguments.end();

    auto country_it = arguments.find("country");
    bool has_country = country_it != arguments.end();

    auto distance_it = arguments.find("toDistance");
    bool has_distance = distance_it != arguments.end();

    time32_t from = 0;
    time32_t to = 0;
    distance_t distance = 0;

    if (has_from && !to_int(from_it->second, from))
    {
        return make_response(response_template::error_400, keep_alive);
    }

    if (has_to && !to_int(to_it->second, to))
    {
        return make_response(response_template::error_400, keep_alive);
    }

    if (has_distance && !to_uint(distance_it->second, distance))
    {
        return make_response(response_template::error_400, keep_alive);
    }

    visit_description_list visits;
    if (!db->select_visits(visits, id,
        has_from ? &from : nullptr,
        has_to ? &to : nullptr,
        has_country ? country_it->second.c_str() : nullptr,
        has_distance ? &distance : nullptr))
    {
        return make_response(response_template::error_404, keep_alive);
    }

    return save_json_and_make_response(visits, keep_alive, buffer);
}

const_memory_block get_location_mark(data_base* db, bool keep_alive, uint32_t id, const key_value_list_t& arguments, memory_block buffer)
{
    auto from_it = arguments.find("fromDate");
    bool has_from = from_it != arguments.end();

    auto to_it = arguments.find("toDate");
    bool has_to = to_it != arguments.end();

    auto from_age_it = arguments.find("fromAge");
    bool has_from_age = from_age_it != arguments.end();

    auto to_age_it = arguments.find("toAge");
    bool has_to_age = to_age_it != arguments.end();

    auto gender_it = arguments.find("gender");
    bool has_gender = gender_it != arguments.end();

    time32_t from = 0;
    time32_t to = 0;
    year_t from_age = 0;
    year_t to_age = 0;

    if (has_from && !to_int(from_it->second, from))
    {
        return make_response(response_template::error_400, keep_alive);
    }

    if (has_to && !to_int(to_it->second, to))
    {
        return make_response(response_template::error_400, keep_alive);
    }

    if (has_from_age && !to_uint(from_age_it->second, from_age))
    {
        return make_response(response_template::error_400, keep_alive);
    }

    if (has_to_age && !to_uint(to_age_it->second, to_age))
    {
        return make_response(response_template::error_400, keep_alive);
    }

    gender_type gender = gender_type::male;

    if (has_gender)
    {
        if (gender_it->second == "f")
        {
            gender = gender_type::female;
        }
        else if (gender_it->second != "m")
        {
            return make_response(response_template::error_400, keep_alive);
        }
    }

    double result = 0;
    if (!db->count_average(result, id,
        has_from ? &from : nullptr,
        has_to ? &to : nullptr,
        has_from_age ? &from_age : nullptr,
        has_to_age ? &to_age : nullptr,
        has_gender ? &gender : nullptr))
    {
        return make_response(response_template::error_404, keep_alive);
    }

    return save_json_and_make_response(result, keep_alive, buffer);
}

request_handler::request_handler(data_base& db)
    : db_(db)
{
}

const_memory_block request_handler::process(request_context&& context)
{
    switch (context.entity_)
    {
        case url_parser::entity::user:
            if (context.method_ == http_method::get)
            {
                return get_entity<user_t>(&db_, context.keep_alive_, context.id_, context.buffer_);
            }
            else
            {
                return update_entity<user_t, one_user_handler>(&db_, context.keep_alive_, context.id_, context.body_);
            }
        case url_parser::entity::location:
            if (context.method_ == http_method::get)
            {
                return get_entity<location_t>(&db_, context.keep_alive_, context.id_, context.buffer_);
            }
            else
            {
                return update_entity<location_t, one_location_handler>(&db_, context.keep_alive_, context.id_, context.body_);
            }
        case url_parser::entity::visit:
            if (context.method_ == http_method::get)
            {
                return get_entity<visit_t>(&db_, context.keep_alive_, context.id_, context.buffer_);
            }
            else
            {
                return update_entity<visit_t, one_visit_handler>(&db_, context.keep_alive_, context.id_, context.body_);
            }
        case url_parser::entity::user_visits:
            return get_user_visits(&db_, context.keep_alive_, context.id_, context.arguments_, context.buffer_);
        case url_parser::entity::location_mark:
            return get_location_mark(&db_, context.keep_alive_, context.id_, context.arguments_, context.buffer_);
        case url_parser::entity::new_user:
            return add_entity<user_t, one_user_handler>(&db_, context.keep_alive_, context.body_);
        case url_parser::entity::new_location:
            return add_entity<location_t, one_location_handler>(&db_, context.keep_alive_, context.body_);
        case url_parser::entity::new_visit:
            return add_entity<visit_t, one_visit_handler>(&db_, context.keep_alive_, context.body_);
    };
}

#pragma once

#include <fstream>

#include "json.h"
#include "utils.h"

class data_base final
{
    static constexpr size_t users_max_size                  = 1050000;
    static constexpr size_t locations_max_size              = 780000;
    static constexpr size_t visits_max_size                 = 10100000;
    static constexpr size_t user_visits_max_size            = visits_max_size;
    static constexpr size_t marks_max_size                  = users_max_size;

public:
    explicit data_base(const std::string& root_dir);

    data_base(const data_base&) = delete;
    data_base& operator=(const data_base&) = delete;

    data_base(data_base&&) = delete;
    data_base& operator=(data_base&&) = delete;

    bool select(user_t& user, user_id id);
    bool update(uint32_t id, const user_t& user);
    bool add(const user_t& user);

    bool select(location_t& location, location_id id);
    bool update(uint32_t id, const location_t& location);
    bool add(const location_t& location);

    bool select(visit_t& visit, visit_id id);
    bool update(uint32_t id, const visit_t& visit);
    bool add(const visit_t& visit);

    bool count_average(double& result, location_id location, time32_t* from, time32_t* to, year_t* from_age, year_t* to_age, gender_type* gender);

    bool select_visits(visit_description_list& result, user_id user, time32_t* from, time32_t* to, const char* country, distance_t* distance);

private:
    template <class handler_t>
    bool load(const std::string& dir, const std::string& prefix, handler_t&& handler)
    {
        int i = 1;

        while (true)
        {
            const std::string file_name = dir + prefix + "_" + std::to_string(i) + ".json";
            std::ifstream file(file_name);
            if (!file)
                return true;

            //std::cout << "parsing " << file_name << "... " << std::flush;

            //timer t;

            rapidjson::Reader reader;
            rapidjson::IStreamWrapper stream(file);
            reader.Parse(stream, handler);
            if (reader.HasParseError())
            {
                std::cerr << "error while parsing " << file_name << '\n';
                return false;
            }

            ++i;
        }
    }

    void build_visits_cache();
    void build_marks_cache();
    void build_user_visits_cache();

    bool user_exists(uint32_t id) const;
    bool location_exists(uint32_t id) const;
    bool visit_exists(uint32_t id) const;
    bool mark_exists(uint32_t id) const;
    bool user_visit_exists(uint32_t id) const;

    void remove_from_cache(const user_t& user);
    void remove_from_location_marks_cache(uint32_t visit_id, const user_t& user);

    void add_to_cache(const user_t& user);
    void add_to_location_marks_cache(uint32_t visit_id, const user_t& user);

    void remove_from_cache(const location_t& location);
    void remove_from_user_visits_cache(uint32_t visit_id, const location_t& location);

    void add_to_cache(const location_t& location);
    void add_to_user_visits_cache(uint32_t visit_id, const location_t& location);

    void remove_from_cache(const visit_t& visit);

    void add_to_cache(const visit_t& visit);

    country_id create_country_id(const location_t& location);
    country_id get_country_id(const location_t& location);

private:
    user_list users_;
    size_t max_user_;

    location_list locations_;
    size_t max_location_;

    visit_list visits_;
    size_t max_visit_;

    country_id_map country_ids_;

    std::vector<std::vector<visit_id>> visit_by_user_;

    std::vector<std::vector<visit_id>> visit_by_location_;

    user_visits_list user_visits_;
    marks_list marks_;

    boost::posix_time::ptime now_;
};

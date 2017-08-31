#pragma once

#include <cstdint>
#include <ctime>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "utils.h"

using year_t = uint32_t;
using time32_t = int32_t;
using user_id = uint32_t;

enum class gender_type
{
    male,
    female
};

struct user_t
{
    user_id id_;
    std::string email_;
    std::string first_name_;
    std::string second_name_;
    gender_type gender_;
    time32_t birth_date_;

    user_t()
        : id_(0)
    {
    }

    user_t(const user_t& copied)
        : id_(copied.id_)
        , email_(copied.email_)
        , first_name_(copied.first_name_)
        , second_name_(copied.second_name_)
        , gender_(copied.gender_)
        , birth_date_(copied.birth_date_)
    {
    }

    user_t& operator=(const user_t& copied)
    {
        id_ = copied.id_;
        email_ = copied.email_;
        first_name_ = copied.first_name_;
        second_name_ = copied.second_name_;
        gender_ = copied.gender_;
        birth_date_ = copied.birth_date_;
        return *this;
    }

    user_t(user_t&&) = default;
    user_t& operator=(user_t&&) = default;
};

using user_list = std::vector<user_t>;

using location_id = uint32_t;

using distance_t = uint32_t;

struct location_t
{
    location_id id_;
    std::string place_;
    std::string country_;
    std::string city_;
    distance_t distance_;

    location_t()
        : id_(0)
    {
    }

    location_t(const location_t& copied)
        : id_(copied.id_)
        , place_(copied.place_)
        , country_(copied.country_)
        , city_(copied.city_)
        , distance_(copied.distance_)
    {
    }

    location_t& operator=(const location_t& copied)
    {
        id_ = copied.id_;
        place_ = copied.place_;
        country_ = copied.country_;
        city_ = copied.city_;
        distance_ = copied.distance_;
        return *this;
    }

    location_t(location_t&&) = default;
    location_t& operator=(location_t&&) = default;
};

using location_list = std::vector<location_t>;

using visit_id = uint32_t;

using mark_t = uint32_t;

struct visit_t
{
    visit_id id_;
    location_id location_;
    user_id user_;
    time32_t visited_at_;
    mark_t mark_;

    visit_t()
        : id_(0)
    {
    }

    visit_t(const visit_t& copied)
        : id_(copied.id_)
        , location_(copied.location_)
        , user_(copied.user_)
        , visited_at_(copied.visited_at_)
        , mark_(copied.mark_)
    {
    }

    visit_t& operator=(const visit_t& copied)
    {
        id_ = copied.id_;
        location_ = copied.location_;
        user_ = copied.user_;
        visited_at_ = copied.visited_at_;
        mark_ = copied.mark_;
        return *this;
    }

    visit_t(visit_t&&) = default;
    visit_t& operator=(visit_t&&) = default;
};

using visit_list = std::vector<visit_t>;

using country_id = size_t;
using country_id_map = std::unordered_map<std::string, country_id>;

struct user_visit_t
{
    multidictionary<time32_t, visit_id> visited_at_;
    multidictionary<country_id, visit_id> countries_;
    multidictionary<distance_t, visit_id> distances_;

    user_visit_t()
    {
    }

    user_visit_t(const user_visit_t& copied)
        : visited_at_(copied.visited_at_)
        , countries_(copied.countries_)
        , distances_(copied.distances_)
    {
    }

    user_visit_t& operator=(const user_visit_t& copied)
    {
        visited_at_ = copied.visited_at_;
        countries_ = copied.countries_;
        distances_ = copied.distances_;
        return *this;
    }

    user_visit_t(user_visit_t&&) = default;
    user_visit_t& operator=(user_visit_t&&) = default;
};

using user_visits_list = std::vector<user_visit_t>; // user_visit_id == user_id

struct location_marks
{
    multidictionary<time32_t, visit_id> visited_at_;
    multidictionary<time32_t, visit_id> ages_;
    std::unordered_set<visit_id> men_;
    std::unordered_set<visit_id> women_;

    location_marks()
    {
    }

    location_marks(const location_marks& copied)
        : visited_at_(copied.visited_at_)
        , ages_(copied.ages_)
        , men_(copied.men_)
        , women_(copied.women_)
    {
    }

    location_marks& operator=(const location_marks& copied)
    {
        visited_at_ = copied.visited_at_;
        ages_ = copied.ages_;
        men_ = copied.men_;
        women_ = copied.women_;
        return *this;
    }

    location_marks(location_marks&&) = default;
    location_marks& operator=(location_marks&&) = default;
};

using marks_list = std::vector<location_marks>; // location_id == mark_id

struct visit_description
{
    mark_t mark_;
    time32_t visited_at_;
    std::string place_;
};

using visit_description_list = std::vector<visit_description>;

#include "data_base.h"

data_base::data_base(const std::string& root_dir)
    : max_user_(0)
    , max_location_(0)
    , max_visit_(0)
{
    timer total_timer;

    std::cout << "root dir: " << root_dir << std::endl;

    {
        auto timestamp = std::ifstream("/tmp/data/options.txt");
        if (timestamp)
        {
            time32_t tmp;
            timestamp >> tmp;
            now_ = from_time_t(tmp);
            std::cout << "set timestamp from options: " << now_ << std::endl;
        }
        else
        {
            now_ = from_time_t(time(0));
            std::cout << "set timestamp from clock: " << now_ << std::endl;
        }
    }

    {
        std::cout << "data structures initialization... " << std::flush;

        timer init_data_timer;

        users_.resize(users_max_size);
        visit_by_user_.resize(users_max_size);
        locations_.resize(locations_max_size);
        visit_by_location_.resize(locations_max_size);
        visits_.resize(visits_max_size);
        marks_.resize(marks_max_size);
        user_visits_.resize(user_visits_max_size);
    }

    load(root_dir, "locations", locations_handler(locations_, max_location_));
    load(root_dir, "users", users_handler(users_, max_user_));
    load(root_dir, "visits", visits_handler(visits_, max_visit_));

    std::cout << "building users visits cache... " << std::flush;
    {
        timer t;
        build_user_visits_cache();
    }

    std::cout << "building marks cache... " << std::flush;
    {
        timer t;
        build_marks_cache();
    }

    std::cout << "building visits cache... " << std::flush;
    {
        timer t;
        build_visits_cache();
    }

    std::cout << "users:                    " << max_user_ << '\n';
    std::cout << "locations:                " << max_location_ << '\n';
    std::cout << "visits:                   " << max_visit_ << '\n';
    std::cout << "country ids:              " << country_ids_.size() << '\n';

    std::cout << "size of user:             " << sizeof(user_t) << std::endl;
    std::cout << "size of location:         " << sizeof(location_t) << std::endl;
    std::cout << "size of visit:            " << sizeof(visit_t) << std::endl;

    std::cout << "total time: ";
}

bool data_base::select(user_t& user, user_id id)
{
    if (!user_exists(id))
        return false;

    user = users_[id];
    return true;
}

bool data_base::update(uint32_t id, const user_t& user)
{
    if (!user_exists(id))
        return false;

    const user_t& old_user = users_[id];
    remove_from_cache(old_user);

    users_[id] = user;
    add_to_cache(user);

    return true;
}

bool data_base::add(const user_t& user)
{
    if (user_exists(user.id_))
        return false;

    assert(visit_by_user_[user.id_].empty());
    users_[user.id_] = user;
    return true;
}

bool data_base::select(location_t& location, location_id id)
{
    if (!location_exists(id))
        return false;

    location = locations_[id];
    return true;
}

bool data_base::update(uint32_t id, const location_t& location)
{
    if (!location_exists(id))
        return false;

    const location_t& old_location = locations_[id];
    remove_from_cache(old_location);

    locations_[id] = location;
    add_to_cache(location);
    return true;
}

bool data_base::add(const location_t& location)
{
    if (location_exists(location.id_))
        return false;

    assert(visit_by_location_[location.id_].empty());

    get_country_id(location); // create new country if needed
    locations_[location.id_] = location;
    return true;
}

bool data_base::select(visit_t& visit, visit_id id)
{
    if (!visit_exists(id))
        return false;

    visit = visits_[id];
    return true;
}

bool data_base::update(uint32_t id, const visit_t& visit)
{
    if (!visit_exists(id))
        return false;

    const visit_t& old_visit = visits_[id];
    remove_from_cache(old_visit);

    visits_[id] = visit;
    add_to_cache(visit);
    return true;
}

bool data_base::add(const visit_t& visit)
{
    if (visit_exists(visit.id_))
        return false;

    visits_[visit.id_] = visit;
    add_to_cache(visit);
    return true;
}

bool data_base::count_average(double& result, location_id location, time32_t* from, time32_t* to, year_t* from_age, year_t* to_age, gender_type* gender)
{
    if (!mark_exists(location))
        return false;

    if (to && from && *to < *from)
        return true;

    if (to_age && from_age && *to_age < *from_age)
        return true;

    const auto& info = marks_[location];

    auto first_visit = from == nullptr
        ? info.visited_at_.begin()
        : info.visited_at_.lower_bound(*from);

    auto end_visit = to == nullptr
        ? info.visited_at_.end()
        : info.visited_at_.upper_bound(*to);

    const auto ages = (from_age == nullptr && to_age == nullptr)
        ? info.ages_.empty_slice()
        : make_slice<uint32_t>(
            to_age == nullptr
                 ? info.ages_.begin()
                 : info.ages_.lower_bound(my_to_time_t(now_ - boost::gregorian::years(*to_age))),
            from_age == nullptr
                ? info.ages_.end()
                : info.ages_.upper_bound(my_to_time_t(now_ - boost::gregorian::years(*from_age))));

    result = 0;
    int n = 0;
    while (first_visit != end_visit)
    {
        const auto id = first_visit->second;

        const bool can_count_1 = (from_age == nullptr && to_age == nullptr) || ages.has(id);
        const bool can_count_2 = gender == nullptr || (*gender == gender_type::male ? info.men_.find(id) != info.men_.end() : info.women_.find(id) != info.women_.end());

        if (can_count_1 && can_count_2)
        {
            result += visits_[id].mark_;
            ++n;
        }

        ++first_visit;
    }

    if (n != 0)
        result /= n;

    return true;
}

bool data_base::select_visits(visit_description_list& result, user_id user, time32_t* from, time32_t* to, const char* country, distance_t* distance)
{
    if (!user_exists(user))
        return false;

    if (to && from && *to < *from)
        return true;

    if (distance && *distance == 0)
        return true;

    const auto& info = user_visits_[user];

    auto first_visit = from == nullptr
        ? info.visited_at_.begin()
        : info.visited_at_.lower_bound(*from);

    const auto end_visit = to == nullptr
        ? info.visited_at_.end()
        : info.visited_at_.upper_bound(*to);

    uint32_t country_id = 0;
    if (country != nullptr)
    {
        const auto it = country_ids_.find(std::string(country));
        if (it == country_ids_.end())
            return false;
        country_id = it->second;
    }

    const auto countries = (country == nullptr)
        ? info.countries_.empty_slice()
        : info.countries_.slice_of_equal(country_id);

    const auto distances = (distance == nullptr)
        ? info.distances_.empty_slice()
        : info.distances_.slice_to_upper(*distance - 1);

    while (first_visit != end_visit)
    {
        const auto id = first_visit->second;

        const bool can_insert_1 = country == nullptr || countries.has(id);
        const bool can_insert_2 = distance == nullptr || distances.has(id);

        if (can_insert_1 && can_insert_2)
        {
            const visit_t& visit = visits_[first_visit->second];
            result.push_back({ visit.mark_, visit.visited_at_, locations_[visit.location_].place_ });
        }

        ++first_visit;
    }

    return true;
}

void data_base::build_visits_cache()
{
    for (size_t id = 0, size = visits_.size(); id < size; ++id)
    {
        if (!visit_exists(id))
            continue;

        const auto& visit = visits_[id];

        visit_by_user_[visit.user_].push_back(id);
        visit_by_location_[visit.location_].push_back(id);
    }

    for (auto& visits : visit_by_user_)
    {
        std::sort(visits.begin(), visits.end());
    }

    for (auto& locations : visit_by_location_)
    {
        std::sort(locations.begin(), locations.end());
    }
}

void data_base::build_marks_cache()
{
    for (size_t id = 0, size = visits_.size(); id < size; ++id)
    {
        if (!visit_exists(id))
            continue;

        const auto& visit = visits_[id];

        auto& mark = marks_[visit.location_];

        mark.visited_at_.push_back(visit.visited_at_, id);

        const auto& user = users_[visit.user_];
        mark.ages_.push_back(user.birth_date_, id);

        if (user.gender_ == gender_type::male)
            mark.men_.insert(id);
        else
            mark.women_.insert(id);
    }

    for (auto& mark : marks_)
    {
        mark.ages_.sort();
        mark.visited_at_.sort();
    }
}

void data_base::build_user_visits_cache()
{
    for (size_t id = 0, size = locations_.size(); id < size; ++id)
    {
        if (!location_exists(id))
            continue;

        create_country_id(locations_[id]);
    }

    for (size_t id = 0, size = visits_.size(); id < size; ++id)
    {
        if (!visit_exists(id))
            continue;

        const auto& visit = visits_[id];
        auto& user_visits = user_visits_[visit.user_];

        const auto& location = locations_[visit.location_];
        const auto country_id = get_country_id(location);

        user_visits.visited_at_.push_back(visit.visited_at_, visit.id_);
        user_visits.countries_.push_back(country_id, visit.id_);
        user_visits.distances_.push_back(location.distance_, visit.id_);
    }

    for (size_t id = 0, size = visits_.size(); id < size; ++id)
    {
        const auto& visit = visits_[id];
        auto& user_visits = user_visits_[visit.user_];

        user_visits.visited_at_.sort();
        user_visits.countries_.sort();
        user_visits.distances_.sort();
    }
}

bool data_base::user_exists(uint32_t id) const
{
    return id < users_max_size && users_[id].id_ != 0;
}

bool data_base::location_exists(uint32_t id) const
{
    return id < locations_max_size && locations_[id].id_ != 0;
}

bool data_base::visit_exists(uint32_t id) const
{
    return id < visits_max_size && visits_[id].id_ != 0;
}

bool data_base::mark_exists(uint32_t id) const
{
    return location_exists(id);
}

bool data_base::user_visit_exists(uint32_t id) const
{
    return user_exists(id);
}

void data_base::remove_from_cache(const user_t& user)
{
    for (auto visit_id : visit_by_user_[user.id_])
    {
        remove_from_location_marks_cache(visit_id, user);
    }
}

void data_base::remove_from_location_marks_cache(uint32_t visit_id, const user_t& user)
{
    auto& visit = visits_[visit_id];

    const uint32_t location_id = visits_[visit_id].location_;
    auto& location_marks = marks_[location_id];

    location_marks.ages_.erase(user.birth_date_, visit_id);
    if (user.gender_ == gender_type::male)
        location_marks.men_.erase(visit_id);
    else
        location_marks.women_.erase(visit_id);
}

void data_base::add_to_cache(const user_t& user)
{
    for (auto visit_id : visit_by_user_[user.id_])
    {
        add_to_location_marks_cache(visit_id, user);
    }
}

void data_base::add_to_location_marks_cache(uint32_t visit_id, const user_t& user)
{
    const uint32_t location_id = visits_[visit_id].location_;
    auto& location_marks = marks_[location_id];

    location_marks.ages_.insert(user.birth_date_, visit_id);
    if (user.gender_ == gender_type::male)
        location_marks.men_.insert(visit_id);
    else
        location_marks.women_.insert(visit_id);
}

void data_base::remove_from_cache(const location_t& location)
{
    for (auto visit_id : visit_by_location_[location.id_])
    {
        remove_from_user_visits_cache(visit_id, location);
    }
}

void data_base::remove_from_user_visits_cache(uint32_t visit_id, const location_t& location)
{
    auto& visit = visits_[visit_id];

    const uint32_t user_id = visit.user_;
    auto& user_visits = user_visits_[user_id];

    const uint32_t country_id = get_country_id(location);
    user_visits.countries_.erase(country_id, visit_id);
    user_visits.distances_.erase(location.distance_, visit_id);
}

void data_base::add_to_cache(const location_t& location)
{
    for (auto visit_id : visit_by_location_[location.id_])
    {
        add_to_user_visits_cache(visit_id, location);
    }
}

void data_base::add_to_user_visits_cache(uint32_t visit_id, const location_t& location)
{
    const uint32_t user_id = visits_[visit_id].user_;
    auto& user_visits = user_visits_[user_id];

    const uint32_t country_id = get_country_id(location);
    user_visits.countries_.insert(country_id, visit_id);
    user_visits.distances_.insert(location.distance_, visit_id);
}

void data_base::remove_from_cache(const visit_t& visit)
{
    const user_t& user = users_[visit.user_];
    remove_from_location_marks_cache(visit.id_, user);

    const location_t& location = locations_[visit.location_];
    remove_from_user_visits_cache(visit.id_, location);

    erase(visit_by_user_, visit.user_);
    erase(visit_by_location_, visit.location_);

    auto& location_marks = marks_[visit.location_];
    location_marks.visited_at_.erase(visit.visited_at_, visit.id_);

    auto& user_visits = user_visits_[visit.user_];
    user_visits.visited_at_.erase(visit.visited_at_, visit.id_);
}

void data_base::add_to_cache(const visit_t& visit)
{
    insert(visit_by_user_, visit.user_);
    insert(visit_by_location_, visit.location_);

    const user_t& user = users_[visit.user_];
    add_to_location_marks_cache(visit.id_, user);

    const location_t& location = locations_[visit.location_];
    add_to_user_visits_cache(visit.id_, location);

    auto& location_marks = marks_[visit.location_];
    location_marks.visited_at_.insert(visit.visited_at_, visit.id_);

    auto& user_visits = user_visits_[visit.user_];
    user_visits.visited_at_.insert(visit.visited_at_, visit.id_);
}

country_id data_base::create_country_id(const location_t& location)
{
    const auto new_id = country_ids_.size();
    country_ids_.emplace(location.country_, new_id);
    return new_id;
}

country_id data_base::get_country_id(const location_t& location)
{
    auto it = country_ids_.find(location.country_);
    if (it != country_ids_.end())
        return it->second;

    return create_country_id(location);
}

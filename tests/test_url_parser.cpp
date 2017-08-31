#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "../src/in_place_writer.h"
#include "../src/json.h"
#include "../src/http_parser.h"
#include "../src/url_parser.h"
#include "../src/utils.h"

std::ostream& operator<<(std::ostream& out, http_parser::status status)
{
    out << (int) status;
    return out;
}

std::ostream& operator<<(std::ostream& out, http_method method)
{
    out << (int) method;
    return out;
}

std::ostream& operator<<(std::ostream& out, url_parser::entity operation)
{
    out << (int) operation;
    return out;
}

std::ostream& operator<<(std::ostream& out, url_parser::status status)
{
    out << (int) status;
    return out;
}

std::ostream& operator<<(std::ostream& out, const std::pair<std::string, std::string>& pair)
{
    out << '[' << pair.first << ", " << pair.second << ']';
    return out;
}

#define check(x, y) if ((x) != (y)) std::cout << __LINE__ << ": " << (x) << " != " << (y) << std::endl

template <class T>
bool parse_json(const char* json, T& handler)
{
    rapidjson::Reader reader;
    rapidjson::MemoryStream stream(json, strlen(json));
    reader.Parse(stream, handler);
    return !reader.HasParseError();
}

int main()
{
    {
        url_parser p("/users/new");
        check(p.entity_, url_parser::entity::new_user);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/users/new?query_id=5");
        check(p.entity_, url_parser::entity::new_user);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/locations/new");
        check(p.entity_, url_parser::entity::new_location);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/locations/new?query_id=5");
        check(p.entity_, url_parser::entity::new_location);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/visits/new");
        check(p.entity_, url_parser::entity::new_visit);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/visits/new?query_id=5");
        check(p.entity_, url_parser::entity::new_visit);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/");
        check(p.status_, url_parser::status::error_404);
    }

    {
        url_parser p("/usr/1/");
        check(p.status_, url_parser::status::error_404);
    }

    {
        url_parser p("/usr/90");
        check(p.status_, url_parser::status::error_404);
    }

    {
        url_parser p("/user/90k");
        check(p.status_, url_parser::status::error_404);
    }

    {
        url_parser p("/users/5000000000");
        check(p.status_, url_parser::status::error_404);
    }

    {
        url_parser p("/users/6/");
        check(p.status_, url_parser::status::error_404);
    }

    {
        url_parser p("/users");
        check(p.status_, url_parser::status::error_404);
    }

    {
        url_parser p("/users/");
        check(p.status_, url_parser::status::error_404);
    }

    {
        url_parser p("/users/90");
        check(p.id_, 90);
        check(p.entity_, url_parser::entity::user);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/users/09");
        check(p.id_, 9);
        check(p.entity_, url_parser::entity::user);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/users/0000000000009");
        check(p.id_, 9);
        check(p.entity_, url_parser::entity::user);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/users/0");
        check(p.id_, 0);
        check(p.entity_, url_parser::entity::user);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/users/000000000000");
        check(p.id_, 0);
        check(p.entity_, url_parser::entity::user);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/users/123/visits");
        check(p.id_, 123);
        check(p.entity_, url_parser::entity::user_visits);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/users/123/visits?");
        check(p.status_, url_parser::status::error_400);
    }

    {
        url_parser p("/users/123/visits?toDate=");
        check(p.status_, url_parser::status::error_400);
    }

    {
        url_parser p("/users/123/visits?toDate=856051200&fromDate=1553472000&country=%D0%9D%D0%BE%D0%B2%D0%B0%D1%8F+%D0%B7%D0%B5%D0%BB%D0%B0%D0%BD%D0%B4%D0%B8%D1%8F");
        check(p.id_, 123);
        check(p.entity_, url_parser::entity::user_visits);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 3);
        if (p.arguments_.size() == 3)
        {
            check(p.arguments_["toDate"], "856051200");
            check(p.arguments_["fromDate"], "1553472000");
            check(p.arguments_["country"], "Новая зеландия");
        }
    }

    {
        url_parser p("/locations/");
        check(p.status_, url_parser::status::error_404);
    }

    {
        url_parser p("/locations/7");
        check(p.id_, 7);
        check(p.entity_, url_parser::entity::location);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/locations/");
        check(p.status_, url_parser::status::error_404);
    }

    {
        url_parser p("/locations/345678/avg");
        check(p.id_, 345678);
        check(p.entity_, url_parser::entity::location_mark);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/locations/123/avg?");
        check(p.status_, url_parser::status::error_400);
    }

    {
        url_parser p("/locations/3329/avg?toDate=1574553600&gender=");
        check(p.status_, url_parser::status::error_400);
    }

    {
        url_parser p("/locations/3329/avg?toDate=1574553600&gender=f");
        check(p.id_, 3329);
        check(p.entity_, url_parser::entity::location_mark);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 2);
        if (p.arguments_.size() == 2)
        {
            check(p.arguments_["toDate"], "1574553600");
            check(p.arguments_["gender"], "f");
        }
    }

    {
        url_parser p("/visits/");
        check(p.status_, url_parser::status::error_404);
    }

    {
        url_parser p("/visits/8");
        check(p.id_, 8);
        check(p.entity_, url_parser::entity::visit);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/visits/8/");
        check(p.status_, url_parser::status::error_404);
    }

    {
        url_parser p("/locations/3329/avg?toDate=1574553600&toDate=1574553600");
        check(p.status_, url_parser::status::error_400);
    }

    {
        url_parser p("/users/81541?query_id=444");
        check(p.id_, 81541);
        check(p.entity_, url_parser::entity::user);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/locations/81541?query_id=444");
        check(p.id_, 81541);
        check(p.entity_, url_parser::entity::location);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        url_parser p("/visits/81541?query_id=444");
        check(p.id_, 81541);
        check(p.entity_, url_parser::entity::visit);
        check(p.status_, url_parser::status::ok);
        check(p.arguments_.size(), 0);
    }

    {
        one_user_handler handler;
        const char* json =
            "{ \"id\": 245, \"email\": \"foobar@mail.ru\", \"first_name\": \"Маша\", \"last_name\": \"Пушкина\", \"gender\": \"f\", \"birth_date\": 365299200 }";
        const auto success = parse_json(json, handler);
        check(success, true);

        const user_t& user = handler.entity_;
        check(user.id_, 245);
        check(user.email_, "foobar@mail.ru");
        check(user.first_name_, "Маша");
        check(user.second_name_, "Пушкина");
        check((int) user.gender_, 1);
        check(user.birth_date_, 365299200);
    }

    {
        one_user_handler handler;
        const char* json =
            "{ \"email\": \"foobar@mail.ru\", \"first_name\": \"Маша\", \"last_name\": \"Пушкина\", \"gender\": \"f\", \"birth_date\": 365299200 }";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_user_handler handler;
        const char* json =
            "{ \"id\": 245, \"first_name\": \"Маша\", \"last_name\": \"Пушкина\", \"gender\": \"f\", \"birth_date\": 365299200 }";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_user_handler handler;
        const char* json =
            "{ \"id\": 245, \"email\": \"foobar@mail.ru\", \"last_name\": \"Пушкина\", \"gender\": \"f\", \"birth_date\": 365299200 }";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_user_handler handler;
        const char* json =
            "{ \"id\": 245, \"email\": \"foobar@mail.ru\", \"first_name\": \"Маша\", \"gender\": \"f\", \"birth_date\": 365299200 }";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_user_handler handler;
        const char* json =
            "{ \"id\": 245, \"email\": \"foobar@mail.ru\", \"first_name\": \"Маша\", \"last_name\": \"Пушкина\", \"birth_date\": 365299200 }";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_user_handler handler;
        const char* json =
            "{ \"id\": 245, \"email\": \"foobar@mail.ru\", \"first_name\": \"Маша\", \"last_name\": \"Пушкина\", \"gender\": \"f\" }";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_user_handler handler;
        handler.skip_id_ = true;
        const char* json =
            "{ \"id\": 245, \"email\": \"foobar@mail.ru\", \"first_name\": \"Маша\", \"last_name\": \"Пушкина\", \"gender\": \"f\", \"birth_date\": 365299200 }";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_location_handler handler;
        const char* json =
            "{\"distance\": 15, \"city\": \"Ньюлёв\", \"place\": \"Ручей\", \"id\": 1, \"country\": \"Италия\"}";
        const auto success = parse_json(json, handler);
        check(success, true);

        const location_t& location = handler.entity_;
        check(location.id_, 1);
        check(location.city_, "Ньюлёв");
        check(location.country_, "Италия");
        check(location.distance_, 15);
        check(location.place_, "Ручей");
    }

    {
        one_location_handler handler;
        const char* json =
            "{\"city\": \"Ньюлёв\", \"place\": \"Ручей\", \"id\": 1, \"country\": \"Италия\"}";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_location_handler handler;
        const char* json =
            "{\"distance\": 15, \"place\": \"Ручей\", \"id\": 1, \"country\": \"Италия\"}";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_location_handler handler;
        const char* json =
            "{\"distance\": 15, \"city\": \"Ньюлёв\", \"id\": 1, \"country\": \"Италия\"}";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_location_handler handler;
        const char* json =
            "{\"distance\": 15, \"city\": \"Ньюлёв\", \"place\": \"Ручей\", \"country\": \"Италия\"}";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_location_handler handler;
        const char* json =
            "{\"distance\": 15, \"city\": \"Ньюлёв\", \"place\": \"Ручей\", \"id\": 1}";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_location_handler handler;
        handler.skip_id_ = true;
        const char* json =
            "{\"distance\": 15, \"city\": \"Ньюлёв\", \"place\": \"Ручей\", \"id\": 1, \"country\": \"Италия\"}";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_visit_handler handler;
        const char* json =
            "{\"user\": 84, \"location\": 54, \"visited_at\": 957879823, \"id\": 1, \"mark\": 2}";
        const auto success = parse_json(json, handler);
        check(success, true);

        const visit_t& location = handler.entity_;
        check(location.id_, 1);
        check(location.location_, 54);
        check(location.mark_, 2);
        check(location.user_, 84);
        check(location.visited_at_, 957879823);
    }

    {
        one_visit_handler handler;
        const char* json =
            "{\"location\": 54, \"visited_at\": 957879823, \"id\": 1, \"mark\": 2}";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_visit_handler handler;
        const char* json =
            "{\"user\": 84, \"visited_at\": 957879823, \"id\": 1, \"mark\": 2}";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_visit_handler handler;
        const char* json =
            "{\"user\": 84, \"location\": 54, \"id\": 1, \"mark\": 2}";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_visit_handler handler;
        const char* json =
            "{\"user\": 84, \"location\": 54, \"visited_at\": 957879823, \"mark\": 2}";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_visit_handler handler;
        const char* json =
            "{\"user\": 84, \"location\": 54, \"visited_at\": 957879823, \"id\": 1}";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        one_visit_handler handler;
        handler.skip_id_ = true;
        const char* json =
            "{\"user\": 84, \"location\": 54, \"visited_at\": 957879823, \"id\": 1, \"mark\": 2}";
        const auto success = parse_json(json, handler);
        check(success, false);
    }

    {
        int i = -1;

        check(to_int("0", i), true);
        check(i, 0);

        check(to_int("-10", i), true);
        check(i, -10);

        check(to_int("10", i), true);
        check(i, 10);

        check(to_int("-0", i), true);
        check(i, 0);

        check(to_int("005", i), true);
        check(i, 5);

        check(to_int("2147483647", i), true);
        check(i, 2147483647);

        check(to_int("-2147483648", i), true);
        check(i, -2147483648);

        check(to_int("2147483648", i), false);
        check(to_int("-2147483649", i), false);
        check(to_int("a0", i), false);
        check(to_int("10a", i), false);
        check(to_int("abc", i), false);
    }

    {
        uint32_t i = 1;

        check(to_uint("0", i), true);
        check(i, 0);

        check(to_uint("10", i), true);
        check(i, 10);

        check(to_uint("005", i), true);
        check(i, 5);

        check(to_uint("4294967295", i), true);
        check(i, 4294967295);

        check(to_uint("-0", i), false);
        check(to_uint("4294967296", i), false);
        check(to_uint("-2147483649", i), false);
        check(to_uint("a0", i), false);
        check(to_uint("10a", i), false);
        check(to_uint("abc", i), false);
    }

    {
        const std::string request1 =
            "POST //path HTTP//1.1\r\n"
            "Host: rve.org.uk\r\n"
            "Connection: keep-alive\r\n"
            "Content-Length: 12\r\n"
            "Accept-Language: ru,en;q=0.8\r\n"
            "\r\n"
            "1234567890ab"
            "\r\n";

        http_parser parser;

        for (size_t i = 0; i < request1.size() - 1; ++i)
        {
            auto n = parser.process(request1.c_str() + i, 1);
            check(n, 1);
            check(parser.current_status(), i < request1.size() -2 - 1 ? http_parser::status::incomplete : http_parser::status::complete);
        }

        auto n = parser.process(request1.c_str() + request1.size() - 1, 1);
        check(n, 1);
        check(parser.current_status(), http_parser::status::complete);

        check(parser.method(), http_method::post);
        check(strcmp(parser.url(), "//path"), 0);
        check(std::string(parser.body(), parser.body_size()), "1234567890ab");
        check(parser.keep_alive(), true);

        parser.reset();

        const std::string request2 =
            "GET / HTTP//1.1\r\n"
            "Host: rve.org.uk\r\n"
            "\r\n";

        n = parser.process(request2.c_str(), request2.size());
        check(n, request2.size());
        check(parser.current_status(), http_parser::status::complete);

        check(parser.method(), http_method::get);
        check(strcmp(parser.url(), "/"), 0);
        check(parser.body(), static_cast<const char*>(nullptr));
        check(parser.body_size(), 0);
        check(parser.keep_alive(), false);
    }

    {
        char buf[10];
        auto n = write(buf, -123);
        check(n - buf, 4);
        check(std::string(buf, n), "-123");
    }

    {
        char buf[10];
        auto n = write(buf, (int) 123);
        check(n - buf, 3);
        check(std::string(buf, n), "123");
    }

    {
        char buf[10];
        auto n = write(buf, (int) 0);
        check(n - buf, 1);
        check(std::string(buf, n), "0");
    }

    {
        char buf[10];
        auto n = write(buf, (unsigned) 0);
        check(n - buf, 1);
        check(std::string(buf, n), "0");
    }

    {
        char buf[10];
        auto n = write(buf, (unsigned) 123456);
        check(n - buf, 6);
        check(std::string(buf, n), "123456");
    }

    {
        char buf[10];
        auto n = write(buf, 123.456);
        check(n - buf, 7);
        check(std::string(buf, n), "123.456");
    }

    {
        multidictionary<char, int> dict;

        dict.push_back(3, 1004);
        dict.push_back(2, 1002);
        dict.push_back(1, 1001);
        dict.push_back(4, 1006);
        dict.push_back(3, 1005);
        dict.push_back(3, 1003);
        dict.push_back(5, 1007);

        dict.sort();

        auto s1 = dict.slice_to_upper(3);

        check(s1.has(1004), true);
        check(s1.has(1002), true);
        check(s1.has(1001), true);
        check(s1.has(1005), true);
        check(s1.has(1003), true);
        check(s1.has(1006), false);
        check(s1.has(1007), false);

        dict.erase(3, 1005);

        auto s2 = dict.slice_to_upper(3);

        check(s2.has(1004), true);
        check(s2.has(1002), true);
        check(s2.has(1001), true);
        check(s2.has(1005), false);
        check(s2.has(1003), true);
        check(s2.has(1006), false);
        check(s2.has(1007), false);

        dict.insert(3, 1005);

        auto s3 = dict.slice_to_upper(3);

        check(s3.has(1004), true);
        check(s3.has(1002), true);
        check(s3.has(1001), true);
        check(s3.has(1005), true);
        check(s3.has(1003), true);
        check(s3.has(1006), false);
        check(s3.has(1007), false);

        auto s4 = dict.slice_of_equal(3);

        check(s4.has(1004), true);
        check(s4.has(1002), false);
        check(s4.has(1001), false);
        check(s4.has(1005), true);
        check(s4.has(1003), true);
        check(s4.has(1006), false);
        check(s4.has(1007), false);

        auto s5 = dict.slice_of_equal(1);

        check(s5.has(1004), false);
        check(s5.has(1002), false);
        check(s5.has(1001), true);
        check(s5.has(1005), false);
        check(s5.has(1003), false);
        check(s5.has(1006), false);
        check(s5.has(1007), false);

        auto s6 = dict.slice_of_equal(55);

        check(s6.has(1004), false);
        check(s6.has(1002), false);
        check(s6.has(1001), false);
        check(s6.has(1005), false);
        check(s6.has(1003), false);
        check(s6.has(1006), false);
        check(s6.has(1007), false);
    }

    return 0;
}

#undef check

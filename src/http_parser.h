#pragma once

#include <string>

enum class http_method
{
    unknown,
    get,
    post
};

class http_parser final
{
public:
    enum class status
    {
        incomplete,
        complete,
        error
    };

    http_parser();

    size_t process(const char* str, size_t size);

    status current_status() const;

    http_method method() const;

    const char* url() const;
    size_t url_size() const;

    const char* body() const;
    size_t body_size() const;

    bool keep_alive() const;

    void reset();

private:
    enum class state
    {
        method,
        get_e,
        get_t,
        post_o,
        post_s,
        post_t,
        space,
        start_url,
        url,
        content_length_o,
        content_length_n,
        content_length_t,
        content_length_e1,
        content_length_n1,
        content_length_t1,
        content_length_minus,
        content_length_l,
        content_length_e2,
        content_length_n2,
        content_length_g,
        content_length_t2,
        content_length_h,
        content_length_colon,
        content_length_space,
        content_length_number,
        keep_alive_e1,
        keep_alive_e2,
        keep_alive_p,
        keep_alive_minus,
        keep_alive_a,
        keep_alive_l,
        keep_alive_i,
        keep_alive_v,
        keep_alive_e3,
        keep_alive_rc,
        header_r1,
        header_n1,
        header_r2,
        header_n2,
        body,
        body_r,
        body_n,
        end
    };

    state state_;

    status status_;

    http_method method_;

    const char* url_;
    size_t url_size_;

    const char* body_;
    size_t body_size_;
    size_t body_counter_;

    bool keep_alive_;
};

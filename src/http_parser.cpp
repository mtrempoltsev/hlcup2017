#include "http_parser.h"

#define RETURN_ERROR { status_ = status::error; return i; }
#define RETURN_END { status_ = status::complete; state_ = state::end; return i; }
#define CHAR_MUST_BE(x, next_state) if (c != x) { RETURN_ERROR } else state_ = next_state;
#define CHECK_WHAT(x, next_state) if (c != x) state_ = state::header_r1; else state_ = next_state; break;

http_parser::http_parser()
{
    reset();
}

size_t http_parser::process(const char* str, size_t size)
{
    size_t i = 0;

    while (i < size)
    {
        const char c = str[i++];

        switch (state_)
        {
        case state::method:
            if (c == 'G')
                state_ = state::get_e;
            else if (c == 'P')
                state_ = state::post_o;
            else
                RETURN_ERROR
            break;
        case state::get_e:
            CHAR_MUST_BE('E', state::get_t)
            break;
        case state::get_t:
            CHAR_MUST_BE('T', state::space)
            method_ = http_method::get;
            break;
        case state::post_o:
            CHAR_MUST_BE('O', state::post_s)
            break;
        case state::post_s:
            CHAR_MUST_BE('S', state::post_t)
            break;
        case state::post_t:
            CHAR_MUST_BE('T', state::space)
            method_ = http_method::post;
            break;
        case state::space:
            if (c == ' ')
                state_ = state::start_url;
            else
                RETURN_ERROR
            break;
        case state::start_url:
            if (c != '/')
            {
                RETURN_ERROR
            }
            state_ = state::url;
            url_ = str + i - 1;
            url_size_ = 1;
            break;
        case state::url:
            if (c == ' ')
            {
                state_ = state::header_r1;
                const_cast<char*>(url_)[url_size_] = '\0';
            }
            else
            {
                ++url_size_;
            }
            break;
        case state::content_length_o:
            CHECK_WHAT('o', state::content_length_n)
        case state::content_length_n:
            CHECK_WHAT('n', state::content_length_t)
        case state::content_length_t:
            CHECK_WHAT('t', state::content_length_e1)
        case state::content_length_e1:
            CHECK_WHAT('e', state::content_length_n1)
        case state::content_length_n1:
            CHECK_WHAT('n', state::content_length_t1)
        case state::content_length_t1:
            CHECK_WHAT('t', state::content_length_minus)
        case state::content_length_minus:
            CHECK_WHAT('-', state::content_length_l)
        case state::content_length_l:
            CHECK_WHAT('L', state::content_length_e2)
        case state::content_length_e2:
            CHECK_WHAT('e', state::content_length_n2)
        case state::content_length_n2:
            CHECK_WHAT('n', state::content_length_g)
        case state::content_length_g:
            CHECK_WHAT('g', state::content_length_t2)
        case state::content_length_t2:
            CHECK_WHAT('t', state::content_length_h)
        case state::content_length_h:
            CHECK_WHAT('h', state::content_length_colon)
        case state::content_length_colon:
            CHECK_WHAT(':', state::content_length_space)
        case state::content_length_space:
            CHAR_MUST_BE(' ', state::content_length_number)
            break;
        case state::content_length_number:
            if (c >= '0' && c <= '9')
                body_size_ = body_size_ * 10 + (c - '0');
            else
                state_ = state::header_n1;
            break;
        case state::keep_alive_e1:
            if (c == '\r')
                state_ = state::header_n1;
            else
                CHECK_WHAT('e', state::keep_alive_e2)
            break;
        case state::keep_alive_e2:
            CHECK_WHAT('e', state::keep_alive_p)
        case state::keep_alive_p:
            CHECK_WHAT('p', state::keep_alive_minus)
        case state::keep_alive_minus:
            CHECK_WHAT('-', state::keep_alive_a)
        case state::keep_alive_a:
            CHECK_WHAT('a', state::keep_alive_l)
        case state::keep_alive_l:
            CHECK_WHAT('l', state::keep_alive_i)
        case state::keep_alive_i:
            CHECK_WHAT('i', state::keep_alive_v)
        case state::keep_alive_v:
            CHECK_WHAT('v', state::keep_alive_e3)
        case state::keep_alive_e3:
            CHECK_WHAT('e', state::keep_alive_rc)
        case state::keep_alive_rc:
            if (c == '\r')
            {
                keep_alive_ = true;
                state_ = state::header_n1;
            }
            else
            {
                state_ = state::header_r1;
            }
            break;
        case state::header_r1:
            if (c == '\r')
                state_ = state::header_n1;
            else if (c == 'k')
                state_ = state::keep_alive_e1;
            break;
        case state::header_n1:
            if (c == '\n')
                state_ = state::header_r2;
            else
                state_ = state::header_r1;
            break;
        case state::header_r2:
            if (c == '\r')
                state_ = state::header_n2;
            else if (body_size_ == 0 && c == 'C')
                state_ = state::content_length_o;
            else
                state_ = state::header_r1;
            break;
        case state::header_n2:
            if (c == '\n')
            {
                if (body_size_ == 0)
                {
                    RETURN_END
                }
                else
                {
                    state_ = state::body;
                    body_ = str + i;
                }
            }
            else
            {
                RETURN_ERROR
            }
            break;
        case state::body:
            if (++body_counter_ == body_size_)
            {
                state_ = state::body_r;
                status_ = status::complete;
            }
            break;
        case state::body_r:
            CHAR_MUST_BE('\r', state::body_n)
                break;
        case state::body_n:
            if (c == '\n')
                RETURN_END
            else
                RETURN_ERROR
            break;
        default:
            RETURN_ERROR
        }
    }

    return i;
}

http_parser::status http_parser::current_status() const
{
    return status_;
}

http_method http_parser::method() const
{
    return method_;
}

const char* http_parser::url() const
{
    return url_;
}

size_t http_parser::url_size() const
{
    return url_size_;
}

const char* http_parser::body() const
{
    return body_;
}

size_t http_parser::body_size() const
{
    return body_size_;
}

bool http_parser::keep_alive() const
{
    return keep_alive_;
}

void http_parser::reset()
{
    state_ = state::method;

    status_ = status::incomplete;

    method_ = http_method::unknown;

    url_ = nullptr;
    url_size_ = 0;

    body_ = nullptr;
    body_size_ = 0;
    body_counter_ = 0;

    keep_alive_ = false;
}

#undef RETURN_ERROR
#undef RETURN_END
#undef CHAR_MUST_BE
#undef CHECK_WHAT

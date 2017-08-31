#include <cassert>

#include "responses.h"

static const std::string error_400 =
    "HTTP/1.1 400 OK\r\n"
    "Server: mt1\r\n"
    "Connection: close\r\n"
    "Content-Length: 0\r\n"
    "\r\n";

static const std::string error_400_keep_alive =
    "HTTP/1.1 400 OK\r\n"
    "Server: mt1\r\n"
    "Connection: keep-alive\r\n"
    "Content-Length: 0\r\n"
    "\r\n";

static const std::string error_404 =
    "HTTP/1.1 404 OK\r\n"
    "Server: mt1\r\n"
    "Connection: close\r\n"
    "Content-Length: 0\r\n"
    "\r\n";

static const std::string error_404_keep_alive =
    "HTTP/1.1 404 OK\r\n"
    "Server: mt1\r\n"
    "Connection: keep-alive\r\n"
    "Content-Length: 0\r\n"
    "\r\n";

static const std::string post_ok =
    "HTTP/1.1 200 OK\r\n"
    "Server: mt1\r\n"
    "Connection: close\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 2\r\n"
    "\r\n"
    "{}";

static const std::string post_ok_keep_alive =
    "HTTP/1.1 200 OK\r\n"
    "Server: mt1\r\n"
    "Connection: keep-alive\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 2\r\n"
    "\r\n"
    "{}";

static const std::string default_headers =
    "HTTP/1.1 200 OK\r\n"
    "Server: mt1\r\n"
    "Connection: close\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: ";

static const std::string default_headers_keep_alive =
    "HTTP/1.1 200 OK\r\n"
    "Server: mt1\r\n"
    "Connection: keep-alive\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: ";

const_memory_block make_response(response_template response, bool keep_alive)
{
    switch (response)
    {
    case response_template::error_400:
        return const_memory_block
            {
                keep_alive
                    ? error_400_keep_alive.c_str()
                    : error_400.c_str(),
                keep_alive
                    ? error_400_keep_alive.size()
                    : error_400.size()
            };
    case response_template::error_404:
        return const_memory_block
            {
                keep_alive
                    ? error_404_keep_alive.c_str()
                    : error_404.c_str(),
                keep_alive
                    ? error_404_keep_alive.size()
                    : error_404.size()
            };
    case response_template::post_ok:
        return const_memory_block
            {
                keep_alive
                    ? post_ok_keep_alive.c_str()
                    : post_ok.c_str(),
                keep_alive
                    ? post_ok_keep_alive.size()
                    : post_ok.size()
            };
    case response_template::default_headers:
        return const_memory_block
            {
                keep_alive
                    ? default_headers_keep_alive.c_str()
                    : default_headers.c_str(),
                keep_alive
                    ? default_headers_keep_alive.size()
                    : default_headers.size()
            };
    }
}

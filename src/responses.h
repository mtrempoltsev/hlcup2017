#pragma once

#include <string>

struct memory_block final
{
    char* data_;
    size_t size_;
};

struct const_memory_block final
{
    const char* data_;
    size_t size_;
};

enum class response_template
{
    error_400,
    error_404,
    post_ok,
    default_headers
};

const_memory_block make_response(response_template response, bool keep_alive);

#pragma once

#include <future>
#include <mutex>
#include <queue>

#include "http_parser.h"

#include "data_base.h"
#include "responses.h"
#include "url_parser.h"

struct request_context
{
    http_method method_;
    bool keep_alive_;
    uint32_t id_;
    url_parser::entity entity_;
    key_value_list_t arguments_;
    const_memory_block body_;
    memory_block buffer_;
};

class request_handler final
{
public:
    explicit request_handler(data_base& db);

    const_memory_block process(request_context&& context);

private:
    data_base& db_;
};

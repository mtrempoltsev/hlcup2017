#pragma once

#include <stack>
#include <thread>

#include <sys/epoll.h>

#include "http_parser.h"
#include "handler.h"

class listener;
class server_worker;

class connection final
{
public:
    connection();

    connection(const connection&) = delete;
    connection& operator=(const connection&) = delete;

    enum class action
    {
        next,
        stop
    };

    void init(server_worker* worker, int epoll, int sock);
    void reset();

    void handle(int events);

    void write_complete();

    memory_block read_buffer();
    const_memory_block write_buffer() const;

    action process_read(size_t bytes_received);
    action process_write(size_t bytes_sent);

    void close();

    bool keep_alive() const;

private:
    void process_request();
    void write_response(const_memory_block data_to_send);

private:
    server_worker* worker_;

    int epoll_;
    int socket_;

    http_parser http_parser_;

    static constexpr size_t max_buffer_size = 10 * 1024;
    std::array<char, max_buffer_size> buffer_;

    memory_block received_data_;
    const_memory_block data_to_send_;

    bool keep_alive_;
};

class http_server final
{
public:
    explicit http_server(request_handler& handler);

    http_server(const http_server&) = delete;
    http_server& operator=(const http_server&) = delete;

    void start(uint16_t port);

private:
    request_handler& handler_;

    static constexpr auto worker_num = 4;

    std::vector<std::unique_ptr<server_worker>> workers_;
    std::vector<std::unique_ptr<std::thread>> worker_threads_;
};

class server_worker final
{
public:
    server_worker(http_server& server, request_handler& handler);

    server_worker(const server_worker&) = delete;
    server_worker& operator=(const server_worker&) = delete;

    const_memory_block process_request(request_context&& context);

    void start(int listen_socket);

    connection* get_connection();
    void release_connection(connection* c);

private:
    http_server& server_;
    request_handler& handler_;

    static constexpr size_t max_events = 1024;
    std::array<struct epoll_event, max_events> events_;

    static constexpr size_t max_connections = max_events;

    std::stack<connection*> free_connections_;
    std::vector<std::unique_ptr<connection>> connections_;
};

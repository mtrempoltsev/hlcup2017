#include <sys/eventfd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#include "responses.h"

#include "http_server.h"

connection::connection()
    : worker_(nullptr)
{
}

void connection::init(server_worker* worker, int epoll, int sock)
{
    worker_ = worker;
    epoll_ = epoll;
    socket_ = sock;
    keep_alive_ = false;
    reset();
}

void connection::reset()
{
    received_data_ = { buffer_.data(), 0 };
    data_to_send_ = { nullptr, 0 };
    http_parser_.reset();
}

void connection::handle(int events)
{
    if (events & EPOLLIN)
    {
        while (true)
        {
            const auto buffer = read_buffer();
            const auto bytes_received = read(socket_, buffer.data_, buffer.size_);

            if (bytes_received == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK) // all read
                    return;

                std::cerr << "read error\n";
                close();
                return;
            }

            if (bytes_received > 0)
            {
                const auto action = process_read(bytes_received);
                if (action == connection::action::stop)
                    return;
            }
            else
            {
                close();
                return;
            }
        }
    }

    if (events & EPOLLOUT)
    {
        while (true)
        {
            const auto buffer = write_buffer();
            const auto bytes_received = write(socket_, buffer.data_, buffer.size_);

            if (bytes_received == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return; // all sent

                std::cerr << "write error\n";
                close();
                return;
            }

            if (bytes_received > 0)
            {
                const auto action = process_write(bytes_received);
                if (action == connection::action::stop)
                {
                    write_complete();
                    return;
                }
            }
        }
    }
}

void connection::write_complete()
{
    struct epoll_event event = {};
    event.data.ptr = this;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_, EPOLL_CTL_MOD, socket_, &event) == -1)
    {
        std::cerr << "can't reset write event\n";
    }
}

memory_block connection::read_buffer()
{
    return memory_block { received_data_.data_, max_buffer_size - received_data_.size_ };
}

const_memory_block connection::write_buffer() const
{
    return data_to_send_;
}

connection::action connection::process_read(size_t bytes_received)
{
    const auto processed = http_parser_.process(received_data_.data_, bytes_received);

    if (processed != bytes_received)
        std::cerr << "maybe next packet received\n";

    switch (http_parser_.current_status())
    {
    case http_parser::status::incomplete:
        received_data_.data_ += processed;
        received_data_.size_ += processed;
        assert(received_data_.size_ < max_buffer_size);
        return action::next;
    case http_parser::status::complete:
        process_request();
        return action::stop;
    case http_parser::status::error:
        std::cerr << "parsing error\n";
        close();
        return action::stop;
    }
}

connection::action connection::process_write(size_t bytes_sent)
{
    assert(data_to_send_.size_ >= bytes_sent);

    data_to_send_.data_ += bytes_sent;
    data_to_send_.size_ -= bytes_sent;

    if (data_to_send_.size_ > 0)
        return action::next;

    keep_alive_ = http_parser_.keep_alive();

    if (http_parser_.keep_alive())
        reset();
    else
        close();

    return action::stop;
}

void connection::process_request()
{
    url_parser parser(http_parser_.url());

    if (parser.status_ == url_parser::status::error_400)
    {
        write_response(make_response(response_template::error_400, http_parser_.keep_alive()));
        return;
    }

    if (parser.status_ == url_parser::status::error_404)
    {
        write_response(make_response(response_template::error_404, http_parser_.keep_alive()));
        return;
    }

    request_context context =
        {
            http_parser_.method(),
            http_parser_.keep_alive(),
            parser.id_,
            parser.entity_,
            std::move(parser.arguments_),
            const_memory_block { http_parser_.body(), http_parser_. body_size() },
            memory_block { buffer_.data(), max_buffer_size }
        };

    write_response(worker_->process_request(std::move(context)));
}

void connection::write_response(const_memory_block data_to_send)
{
    assert(data_to_send.size_ < max_buffer_size);

    data_to_send_ = data_to_send;

    while (true)
    {
        const auto buffer = write_buffer();
        const auto bytes_received = write(socket_, buffer.data_, buffer.size_);

        if (bytes_received == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // full buffer
                struct epoll_event event = {};
                event.data.ptr = this;
                event.events = EPOLLIN | EPOLLOUT | EPOLLET;
                if (epoll_ctl(epoll_, EPOLL_CTL_MOD, socket_, &event) == -1)
                {
                    std::cerr << "can't modify socket in epoll\n";
                }
                return;
            }

            std::cerr << "write error\n";
            close();
            return;
        }

        if (bytes_received > 0)
        {
            const auto action = process_write(bytes_received);
            if (action == connection::action::stop)
                return;
        }
    }
}

void connection::close()
{
    ::close(socket_);
    worker_->release_connection(this);
}

bool connection::keep_alive() const
{
    return keep_alive_;
}

http_server::http_server(request_handler& handler)
    : handler_(handler)
{
}

void http_server::start(uint16_t port)
{
    std::cout << "start server on port " << port << std::endl;

    const int listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket == -1)
    {
        std::cerr << "can't create socket\n";
        return;
    }

    const int enable = 1;
    int result = setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(enable));
    if (result == -1)
    {
        std::cerr << "can't set socket option\n";
        return;
    }

    result = setsockopt(listen_socket, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
    if (result == -1)
    {
        std::cerr << "can't set listener TCP_NODELAY\n";
        return;
    }

    result = setsockopt(listen_socket, SOL_TCP, TCP_DEFER_ACCEPT,(char *) &enable, sizeof(enable));
    if (result == -1)
    {
        std::cerr << "can't set listener TCP_DEFER_ACCEPT\n";
        return;
    }

    struct sockaddr_in addr = {};
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    result = bind(listen_socket, (struct sockaddr *) &addr, sizeof(addr));
    if (result < 0)
    {
        std::cerr << "can't bind socket\n";
        return;
    }

    const int flags = fcntl(listen_socket, F_GETFL, 0);
    if (flags == -1)
    {
        std::cerr << "can't get flags\n";
        return;
    }

    result = fcntl(listen_socket, F_SETFL, flags | O_NONBLOCK);
    if (result == -1)
    {
        std::cerr << "can't set O_NONBLOCK\n";
        return;
    }

    result = ::listen(listen_socket, SOMAXCONN);
    if (result < 0)
    {
        std::cerr << "listen error\n";
        return;
    }

    for (size_t i = 0; i < worker_num; ++i)
    {
        workers_.push_back(std::make_unique<server_worker>(*this, handler_));
        worker_threads_.push_back(std::make_unique<std::thread>(&server_worker::start, workers_.back().get(), listen_socket));
    }

    for (auto& thread : worker_threads_)
    {
        thread->join();
    }
}

server_worker::server_worker(http_server& server, request_handler& handler)
    : server_(server)
    , handler_(handler)
{
    connections_.reserve(max_connections);
    for (size_t i = 0; i < max_connections; ++i)
    {
        connections_.push_back(std::make_unique<connection>());
        free_connections_.push(connections_.back().get());
    }
}

const_memory_block server_worker::process_request(request_context&& context)
{
    return handler_.process(std::forward<request_context>(context));
}

void server_worker::start(int listen_socket)
{
    std::cout << "start worker: " << std::this_thread::get_id() << std::endl;

    const int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        std::cerr << "can't create epoll descriptor\n";
        return;
    }

    struct epoll_event listen_event = {};
    listen_event.data.fd = listen_socket;
    listen_event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_socket, &listen_event) == -1)
    {
        std::cerr << "can't set epoll listen socket\n";
        return;
    }

    bool keep_working = true;
    while (keep_working)
    {
        const int num_events = epoll_wait(epoll_fd, events_.data(), max_events, 0);

        for (int i = 0; i < num_events; ++i)
        {
            const auto& event = events_[i];

            if (event.data.fd != listen_socket)
            {
                auto conn = static_cast<connection*>(event.data.ptr);
                conn->handle(event.events);
            }

            while (true)
            {
                struct sockaddr in_addr = {};
                socklen_t in_addr_len = sizeof(in_addr);

                const int client = accept4(listen_socket, &in_addr, &in_addr_len, SOCK_NONBLOCK);

                struct epoll_event listen_event = {};
                listen_event.data.fd = listen_socket;
                listen_event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

                if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, listen_socket, &listen_event) == -1)
                {
                    std::cerr << "can't set epoll listen socket\n";
                    return;
                }

                if (client == -1)
                {
                    if (errno != EAGAIN && errno != EWOULDBLOCK)
                        std::cerr << "accept error\n";

                    break;
                }

                const int enable = 1;
                int result = setsockopt(client, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
                if (result == -1)
                    std::cerr << "can't set TCP_NODELAY\n";

                auto new_conn = get_connection();
                new_conn->init(this, epoll_fd, client);

                while (true)
                {
                    const auto buffer = new_conn->read_buffer();
                    const auto bytes_received = read(client, buffer.data_, buffer.size_);

                    if (bytes_received == -1)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) // all read
                        {
                            struct epoll_event new_event = {};
                            new_event.data.ptr = new_conn;
                            new_event.events = EPOLLIN | EPOLLET;

                            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client, &new_event) == -1)
                                std::cerr << "epoll add descriptor error 1\n";

                            break;
                        }

                        std::cerr << "read error\n";
                        new_conn->close();

                        break;
                    }

                    if (bytes_received > 0)
                    {
                        const auto action = new_conn->process_read(bytes_received);
                        if (action == connection::action::stop)
                        {
                            if (new_conn->keep_alive())
                            {
                                struct epoll_event new_event = {};
                                new_event.data.ptr = new_conn;
                                new_event.events = EPOLLIN | EPOLLET;

                                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client, &new_event) == -1)
                                    std::cerr << "epoll add descriptor error 2\n";
                            }

                            break;
                        }
                    }
                    else
                    {
                        new_conn->close();
                        break;
                    }
                }
            }
        }
    }
}

connection* server_worker::get_connection()
{
    if (free_connections_.empty())
    {
        for (int i = 0; i < 128; ++i)
        {
            connections_.push_back(std::make_unique<connection>());
            free_connections_.push(connections_.back().get());
        }
        std::cerr << std::this_thread::get_id() << " " << connections_.size() << '\n';
    }
    auto c = free_connections_.top();
    free_connections_.pop();
    return c;
}

void server_worker::release_connection(connection* c)
{
    free_connections_.push(c);
}

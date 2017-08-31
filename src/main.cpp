#include <execinfo.h>
#include <csignal>

#include "data_base.h"
#include "handler.h"
#include "http_server.h"

void signal_handler(int sig)
{
    const size_t depth = 20;
    void* array[depth];

    const auto size = backtrace(array, depth);

    std::cerr << "signal: " << sig << '\n';
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

int main(int argc, char* argv[])
{
    signal(SIGSEGV, signal_handler);

    //data_base db("/home/mt/work/hlcupdocs/data/TRAIN/data/");
    data_base db("./");

    request_handler handler(db);

    http_server server(handler);
    server.start(80);

    return 0;
}

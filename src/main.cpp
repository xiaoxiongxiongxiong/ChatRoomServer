#include "libos.h"
#include "cmdline.h"
#include <limits.h>
#include <mutex>
#include <condition_variable>
#include "impl/ChatRoomServerImpl.h"

// 解析命令行
static bool parse_cmdline(int argc, const char * const argv[], std::string & ip, int & port);

// 主线程锁
static std::mutex g_main_mtx;
// 主线程变量
static std::condition_variable g_main_cond;
// 是否在运行中
static bool g_is_running = true;

int main(int argc, char * argv[])
{
    int port = 0;
    std::string ip;
    if (!parse_cmdline(argc, argv, ip, port))
        return EXIT_FAILURE;

    log_msg_init("ChatRoomServer.log", LOG_LEVEL_INFO);
    atexit(log_msg_uninit);

    log_msg_info("server ip:%s, port:%d ...", ip.c_str(), port);

    CTcpServerImpl * server_impl = new (std::nothrow)CTcpServerImpl();
    if (nullptr == server_impl)
    {
        int code = errno;
        log_msg_error("new CTcpServerImpl failed, err code:%d msg:%s", code, strerror(code));
        return EXIT_FAILURE;
    }
    if (!server_impl->create(ip.c_str(), port))
    {
        log_msg_warn("create tcp server failed ...");
        delete server_impl;
        server_impl = nullptr;
        return EXIT_FAILURE;
    }

    log_msg_info("start tcp server ...");
    while (g_is_running)
    {
        std::unique_lock<std::mutex> lck(g_main_mtx);
        g_main_cond.wait_for(lck, std::chrono::milliseconds(100));
        // TODO
    }

    server_impl->destroy();
    delete server_impl;
    server_impl = nullptr;

    log_msg_info("stop tcp server ...");

    return EXIT_SUCCESS;
}

bool parse_cmdline(int argc, const char * const argv[], std::string & ip, int & port)
{
    if (argc < 1 || nullptr == argv || nullptr == argv[0])
    {
        std::cout << "Invalid param" << std::endl;
        return false;
    }

    // 初始化cmdline
    cmdline::parser p;
    p.add<std::string>("host", '\0', "ChatRoom Server ip address", false, "0.0.0.0");
    p.add<int>("port", '\0', "ChatRoom Server port", false, 0);
    p.add("version", 'v', "app version");
    p.add("help", 'h', "app help");

    if (!p.parse(argc, argv))
    {
        std::cout << p.usage();
        return false;
    }

    if (p.exist("help"))
    {
        std::cout << p.usage();
        return false;
    }

    if (p.exist("version"))
    {
        std::cout << "debug 0.0" << std::endl;
        return false;
    }

    if (p.exist("host") && p.exist("port"))
    {
        ip = p.get<std::string>("host");
        port = p.get<int>("port");
    }
    else
    {
        std::cout << p.usage();
        return false;
    }

    // 简单对参数有效性进行判断
    if (ip.empty() || 0 == port || port > USHRT_MAX)
    {
        std::cerr << "Input param is invalid." << std::endl;
        return false;
    }

    return true;
}

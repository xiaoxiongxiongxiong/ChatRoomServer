#include "libos.h"
#include "cmdline.h"
#include <limits.h>
#include <mutex>
#include <condition_variable>

// 解析命令行
static bool parse_cmdline(int argc, const char * const argv[], std::string & ip, uint16_t & port);

// 主线程锁
static std::mutex g_main_mtx;
// 主线程变量
static std::condition_variable g_main_cond;
// 是否在运行中
static bool g_is_running = true;

int main(int argc, char * argv[])
{
    uint16_t port = 0;
    std::string ip;
    if (!parse_cmdline(argc, argv, ip, port))
        return EXIT_FAILURE;

    log_msg_init("ChatRoomServer.log", LOG_LEVEL_INFO);
    atexit(log_msg_uninit);

    log_msg_info("server ip:%s, port:%d", ip.c_str(), port);
    // TODO 启动TCP服务器

    while (g_is_running)
    {
        std::unique_lock<std::mutex> lck(g_main_mtx);
        g_main_cond.wait_for(lck, std::chrono::milliseconds(100));
        // TODO
    }

    return EXIT_SUCCESS;
}

bool parse_cmdline(int argc, const char * const argv[], std::string & ip, uint16_t & port)
{
    if (argc < 1 || nullptr == argv || nullptr == argv[0])
    {
        std::cout << "Invalid param" << std::endl;
        return false;
    }

    // 初始化cmdline
    cmdline::parser p;
    p.add<std::string>("host", '\0', "ChatRoom Server ip address", false, "");
    p.add<uint16_t>("port", '\0', "ChatRoom Server port", false, 0);
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
        port = p.get<uint16_t>("port");
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

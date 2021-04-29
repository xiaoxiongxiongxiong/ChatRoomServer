#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "MessageQueue.h"

class CTcpServerImpl
{
public:
    CTcpServerImpl() = default;
    ~CTcpServerImpl() = default;

    // 创建TCP服务器
    bool create(const char * ip, int port);

    // 销毁TCP服务器
    void destroy();

protected:
    // 处理客户端请求线程
    void dealRequestThr();
    // 接收连接处理线程
    void acceptMessageThr();
    // 接收消息处理线程
    void recvMessageThr();
    // 发送消息处理线程
    void sendMessageThr();

private:
    // 是否已初始化
    bool _is_init = false;

    // 服务器IP
    std::string _ip;
    // 服务器端口号
    uint16_t _port = 0;
    // 服务器监听句柄
    int _fd = -1;
    // epoll 句柄
    int _epoll_fd = -1;

    // 处理客户端请求线程句柄
    std::thread _server_thr;

    // 处理连接请求线程句柄
    std::thread _accpet_thr;
    // 处理连接请求互斥锁
    std::mutex _accpet_mtx;
    // 处理连接请求条件变量
    std::condition_variable _accpet_cond;

    // 处理接收消息线程句柄
    std::thread _recv_thr;
    // 处理接收消息锁
    std::thread _recv_mtx;
    // 处理接收消息条件变量
    std::condition_variable _recv_cond;

    // 发送消息线程句柄
    std::thread _send_thr;
    // 发送消息线程锁
    std::thread _send_mtx;
    // 发送消息线程条件变量
    std::condition_variable _send_cond;
};

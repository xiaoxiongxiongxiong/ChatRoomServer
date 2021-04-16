#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string>

class ChatRoomServerImpl
{
public:
    ChatRoomServerImpl() = default;
    ~ChatRoomServerImpl() = default;

    // 创建TCP服务器
    bool create(const char * ip, int port);

    // 销毁TCP服务器
    void destroy();

private:
    // 服务器IP
    std::string _ip;
    // 服务器端口号
    uint16_t _port = 0;
};

#include "ChatRoomServerImpl.h"
#include "libos.h"
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

bool CTcpServerImpl::create(const char * ip, const int port)
{
    if (nullptr == ip || 0 == strcmp(ip, "") || port <= 0 || port >= USHRT_MAX)
    {
        log_msg_warn("invalid param, ip is nullptr/empty or port:%d is out of range!", port);
        return false;
    }

    if (_is_init)
    {
        log_msg_warn("server has already initialized!");
        return false;
    }

    _ip = ip;
    _port = static_cast<uint16_t>(port);

    // 创建socket
    _fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_fd < 0)
    {
        int code = errno;
        log_msg_error("socket failed, err code:%d msg:%s", code, strerror(code));
        return false;
    }

    int flags = fcntl(_fd, F_GETFL, 0);
    if (::fcntl(_fd, F_SETFL, flags | O_NONBLOCK) < 0)  // 设置为非阻塞模式
    {
        int code = errno;
        log_msg_error("fcntl F_SETFL O_NONBLOCK failed, err code:%d msg:%s", code, strerror(code));
    }

    int flag = 1;
    if (::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0) // 设置端口可复用
    {
        int code = errno;
        log_msg_error("setsockopt SO_REUSEADDR failed, err code:%d msg:%s", code, strerror(code));
    }

    // 绑定ip和端口号
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(_port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) < 0)
    {
        int code = errno;
        log_msg_error("inet_pton %s failed, err code:%d msg:%s", ip, code, strerror(code));
        destroy();
        return false;
    }
    if (::bind(_fd, reinterpret_cast<const struct sockaddr *>(&server_addr), sizeof(server_addr)) < 0)
    {
        int code = errno;
        log_msg_error("bind %s:%d failed, err code:%d msg:%s", ip, port, code, strerror(code));
        destroy();
        return false;
    }

    // 开始监听
    if (::listen(_fd, 1024) < 0)
    {
        int code = errno;
        log_msg_error("listen %s:%d failed, err code:%d msg:%s", ip, port, code, strerror(code));
        destroy();
        return false;
    }

    _epoll_fd = ::epoll_create(1);
    if (_epoll_fd < 0)
    {
        int code = errno;
        log_msg_error("epoll_create failed, err code:%d msg:%s", code, strerror(code));
        destroy();
        return false;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = _fd;
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _fd, &ev) < 0)
    {
        int code = errno;
        log_msg_error("epoll_ctl failed, err code:%d msg:%s", code, strerror(code));
        destroy();
        return false;
    }

    _is_init = true;
    _server_thr = std::thread{ &CTcpServerImpl::dealRequestThr, this };

    return true;
}

void CTcpServerImpl::destroy()
{
    _is_init = false;

    if (_epoll_fd > 0)
    {
        ::close(_epoll_fd);
        _epoll_fd = -1;
    }

    if (_fd > 0)
    {
        ::close(_fd);
        _fd = -1;
    }

    _port = 0;
    _ip.clear();
}

void CTcpServerImpl::dealRequestThr()
{
    struct epoll_event evs[64] = { 0 };
    while (_is_init)
    {
        int number = epoll_wait(_epoll_fd, evs, 64, 1000);
        if (number < 0)
        {
            int code = errno;
            if (EINTR == code)
                continue;
            log_msg_error("epoll_wait failed, err code:%d msg:%s", code, strerror(code));
            return;
        }

        for (int i = 0; i < number; i++)
        {
            if (evs[i].data.fd == _fd)
            {
                struct sockaddr_in client_addr = { 0 };
                socklen_t len = sizeof(client_addr);
                int client_fd = accept(_fd, (struct sockaddr *)&client_addr, &len);
                if (client_fd == -1)
                {
                    log_msg_error("accept failed, err:%s", strerror(errno));
                    return;
                }

                char host[INET_ADDRSTRLEN] = { 0 };
                inet_ntop(AF_INET, &client_addr.sin_addr, host, INET_ADDRSTRLEN);
                uint16_t port = ntohs(client_addr.sin_port);

                char buff[128] = { 0 };
                snprintf(buff, 128, "%s %d online ...", host, port);
                log_msg_info("%s", buff);
                strcat(buff, "\n");
            }
            else if (evs[i].events & EPOLLIN)
            {
                if (evs[i].data.fd < 0)
                    continue;

                char buff[1024] = { 0 };
                auto ret = recv(evs[i].data.fd, buff, 1024, 0);
                if ((ret < 0 && errno == ECONNRESET) || ret == 0)
                {

                    //delClientInfo(evs[i].data.fd);

                    char msg[128] = { 0 };
                    //snprintf(msg, 128, "%s %d offline ...", client_info.host, client_info.port);
                    log_msg_info("%s", msg);
                    strcat(msg, "\n");
                    //broadcast(msg, strlen(msg), -1);
                    continue;
                }

                char send_buff[1200] = { 0 };
                //snprintf(send_buff, 1200, "%s %d says: %s", client_info.host, client_info.port, buff);
                log_msg_info("%s", send_buff);
               // broadcast(send_buff, strlen(send_buff), evs[i].data.fd);
            }
        }
    }
}

void CTcpServerImpl::acceptMessageThr()
{

}

void CTcpServerImpl::recvMessageThr()
{

}

void CTcpServerImpl::sendMessageThr()
{

}

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

// 客户端管理类
class CServerManager final
{
public:
    static CServerManager & getInstance()
    {
        static CServerManager instance;
        return instance;
    }

    CServerManager(const CServerManager &) = delete;
    CServerManager & operator=(const CServerManager &) = delete;
    CServerManager(CServerManager &&) = delete;
    CServerManager & operator=(CServerManager &&) = delete;

public:


private:
    CServerManager() = default;
    ~CServerManager() = default;
};

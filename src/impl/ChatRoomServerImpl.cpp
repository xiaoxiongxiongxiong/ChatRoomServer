#include "ChatRoomServerImpl.h"
#include "libos.h"
#include <limits.h>
#include <string.h>

bool ChatRoomServerImpl::create(const char * ip, const int port)
{
    if (nullptr == ip || 0 == strcmp(ip, "") || port<0 || port>USHRT_MAX)
    {
        log_msg_warn("invalid param!");
        return false;
    }

    _ip = ip;
    _port = static_cast<uint16_t>(port);

    return true;
}

void ChatRoomServerImpl::destroy()
{

}

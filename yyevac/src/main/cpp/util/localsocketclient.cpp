//
// Created by asus on 2024/11/8.
//

#include "localsocketclient.h"

void LocalSocketClient::sendMessage(int type, string tag, string msg) {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = '\0';
    strcpy(addr.sun_path + 1, SOCKET_PATH);
    const char* name = "eva_local_socket";
    auto len = 1 + offsetof(struct sockaddr_un, sun_path) + strlen(name);

    if (connect(sock, (struct sockaddr*)&addr, len) == -1) {
        return;
    }

    if(send(sock, msg.c_str(), msg.size(), 0) == -1) {
        return;
    }
    close(sock);
}

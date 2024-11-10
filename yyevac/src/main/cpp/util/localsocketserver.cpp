//
// Created by asus on 2024/11/10.
//

#include "localsocketserver.h"

//需要新開線程
void LocalSocketServer::startServer() {
    int sock = socket(PF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = '\0';
    const char* name = "eva_local_socket";
    strcpy(addr.sun_path + 1, name);
    auto len = 1 + offsetof(struct sockaddr_un, sun_path) + strlen(name);

    bind(sock, (struct sockaddr *)&addr, len);

    if (listen(sock, SOMAXCONN) < 0) {
        close(sock);
        return;
    }

    int conn;

    while (1) {
        conn = accept(sock, nullptr, nullptr);
        if (conn == -1) {
            if (conn == EINTR)
                continue;
        }

        char buffer[1024];
        ssize_t bytesRead = recv(sock, buffer, sizeof(buffer) - 1 , 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
        }
        if (string(buffer) == "end") {
            close(sock);
            exit(EXIT_SUCCESS);
            close(conn);
        }
    }
}


//
// Created by asus on 2024/11/8.
//

#pragma once
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <string>

#define SOCKET_PATH "eva_local_socket" // 确保路径与Java层一致

using namespace std;
class LocalSocketClient {
public:
    void sendMessage(int type, string tag, string msg);
};


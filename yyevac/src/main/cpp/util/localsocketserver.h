//
// Created by asus on 2024/11/10.
//

#pragma once
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <string>

using namespace std;
class LocalSocketServer {
    void startServer();
};

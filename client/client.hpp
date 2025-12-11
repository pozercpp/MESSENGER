#pragma once

#include <string>
#include <sstream>
#include <pthread.h>

extern volatile bool running;

class Client {
private:
    std::string login;
    std::string client_pipe;
    pthread_t receive_thread;
    void send_command(const std::string& cmd);
    void receive_messages();
    static void* static_receive_func(void* arg);
public:
    Client(const std::string& _login);
    ~Client();
    void run();
};
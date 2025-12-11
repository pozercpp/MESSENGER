#pragma once

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <pthread.h>

extern volatile bool running;

class Server {
public:
    std::map<std::string, std::string> online;
    std::map<std::string, std::vector<std::string>> pending;
    pthread_mutex_t lock;
    void process_command(const std::string& cmd);
    void process_message(const std::string& from_login, const std::string& to_login, const std::string& msg);
    void send_pending(const std::string& login);
    Server();
    ~Server();
    void run();
};
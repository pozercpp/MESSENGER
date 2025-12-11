#include "server.hpp"

#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

const std::string SERVER_PIPE = "/tmp/msg_server";
const std::string CLIENT_PIPE_PREFIX = "/tmp/msg_";

struct DelayedArgs {
    Server* self;
    std::string from_login;
    std::string to_login;
    std::string msg;
    int delay;
};

static void* delayed_func(void* arg) {
    DelayedArgs* args = static_cast<DelayedArgs*>(arg);
    sleep(args->delay);
    args->self->process_message(args->from_login, args->to_login, args->msg);
    delete args;
    return nullptr;
}

Server::Server() {
    pthread_mutex_init(&lock, nullptr);
}

Server::~Server() {
    pthread_mutex_destroy(&lock);
}

void Server::process_command(const std::string& cmd) {
    std::istringstream iss(cmd);
    std::string action;
    iss >> action;
    if (action == "join") {
        std::string login;
        iss >> login;
        if (login.empty()) return;
        std::string client_pipe = CLIENT_PIPE_PREFIX + login;
        pthread_mutex_lock(&lock);
        if (online.find(login) != online.end()) {
            pthread_mutex_unlock(&lock);
            return;
        }
        online[login] = client_pipe;
        if (pending.find(login) == pending.end()) {
            pending[login] = {};
        }
        send_pending(login);
        pthread_mutex_unlock(&lock);
    } else if (action == "exit") {
        std::string login;
        iss >> login;
        if (login.empty()) return;
        pthread_mutex_lock(&lock);
        auto it = online.find(login);
        if (it != online.end()) {
            online.erase(it);
        }
        pthread_mutex_unlock(&lock);
    } else if (action == "send") {
        std::string from_login, to_login, msg;
        iss >> from_login >> to_login;
        std::getline(iss, msg);
        if (from_login.empty() || to_login.empty() || msg.empty()) return;
        msg = msg.substr(1);
        process_message(from_login, to_login, msg);
    } else if (action == "delayed") {
        std::string from_login, to_login, delay_str, msg;
        iss >> from_login >> to_login >> delay_str;
        std::getline(iss, msg);
        if (from_login.empty() || to_login.empty() || delay_str.empty() || msg.empty()) return;
        msg = msg.substr(1);
        int delay;
        try {
            delay = std::stoi(delay_str);
        } catch (...) {
            return;
        }
        DelayedArgs* args = new DelayedArgs{this, from_login, to_login, msg, delay};
        pthread_t tid;
        if (pthread_create(&tid, nullptr, delayed_func, args) == 0) {
            pthread_detach(tid);
        } else {
            delete args;
        }
    }
}

void Server::process_message(const std::string& from_login, const std::string& to_login, const std::string& msg) {
    std::string formatted_msg = from_login + ": " + msg;
    pthread_mutex_lock(&lock);
    if (pending.find(to_login) == pending.end()) {
        pending[to_login] = {};
    }
    pending[to_login].push_back(formatted_msg);
    if (online.find(to_login) != online.end()) {
        send_pending(to_login);
    }
    pthread_mutex_unlock(&lock);
}

void Server::send_pending(const std::string& login) {
    auto it = online.find(login);
    if (it == online.end()) return;
    std::string client_pipe = it->second;
    int fd = open(client_pipe.c_str(), O_WRONLY);
    if (fd == -1) {
        online.erase(login);
        return;
    }
    for (const auto& msg : pending[login]) {
        std::string line = msg + "\n";
        write(fd, line.c_str(), line.size());
    }
    close(fd);
    pending[login].clear();
}

void Server::run() {
    if (mkfifo(SERVER_PIPE.c_str(), 0666) == -1 && errno != EEXIST) {
        std::cerr << "Failed to create server pipe" << std::endl;
        return;
    }
    while (running) {
        int fd = open(SERVER_PIPE.c_str(), O_RDONLY);
        if (fd == -1) {
            std::cerr << "Failed to open server pipe" << std::endl;
            return;
        }
        char buffer[1024];
        std::string line;
        size_t bytes;
        while ((bytes = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes] = '\0';
            line += buffer;
            size_t pos;
            while ((pos = line.find('\n')) != std::string::npos) {
                std::string cmd = line.substr(0, pos);
                process_command(cmd);
                line.erase(0, pos + 1);
            }
        }
        close(fd);
    }
    unlink(SERVER_PIPE.c_str());
}
#include "client.hpp"

#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <error.h>

const std::string SERVER_PIPE = "/tmp/msg_server";
const std::string CLIENT_PIPE_PREFIX = "/tmp/msg_";

Client::Client(const std::string& _login) : login(_login), client_pipe(CLIENT_PIPE_PREFIX + login) {
    unlink(client_pipe.c_str());
    if (mkfifo(client_pipe.c_str(), 0666) == -1) {
        std::cerr << "Failed to create client pipe" << std::endl;
        return;
    }
    send_command("join " + login);
    if (pthread_create(&receive_thread, nullptr, static_receive_func, this) == 0) {
        pthread_detach(receive_thread);
    }
}

Client::~Client() {
    send_command("exit " + login);
    unlink(client_pipe.c_str());
}

void* Client::static_receive_func(void* arg) {
    Client* self = static_cast<Client*>(arg);
    self->receive_messages();
    return nullptr;
}

void Client::send_command(const std::string& cmd) {
    int fd = open(SERVER_PIPE.c_str(), O_WRONLY);
    if (fd == -1) {
        std::cerr << "Failed to open server pipe" << std::endl;
        return;
    }
    std::string line = cmd + "\n";
    write(fd, line.c_str(), line.size());
    close(fd);
}

void Client::receive_messages() {
    //std::cout << "DEBUG: Starting receive_messages for " << login << std::endl;
    //std::cout << "DEBUG: Pipe: " << client_pipe << std::endl;
    
    int fd = open(client_pipe.c_str(), O_RDWR);
    if (fd == -1) {
      //  std::cout << "DEBUG: Failed to open pipe: " << strerror(errno) << std::endl;
        return;
    }
    
    //std::cout << "DEBUG: Pipe opened successfully" << std::endl;
    
    char buffer[1024];
    while (true) {
      //  std::cout << "DEBUG: Waiting to read from pipe..." << std::endl;
        ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
        //std::cout << "DEBUG: read() returned: " << bytes << std::endl;
        
        if (bytes > 0) {
            buffer[bytes] = '\0';
          //  std::cout << "DEBUG: Received data: " << buffer << std::endl;
            std::cout << "\nReceived: " << buffer << "\n> " << std::flush;
        } else if (bytes == 0) {
            //std::cout << "DEBUG: EOF on pipe" << std::endl;
            break;
        } else {
            //std::cout << "DEBUG: read error: " << strerror(errno) << std::endl;
            break;
        }
    }
    
    close(fd);
    //std::cout << "DEBUG: receive_messages exiting" << std::endl;
}

void Client::run() {
    std::cout << "Logged in as " << login << ". Commands: send <to> <msg>, delayed <to> <delay> <msg>, exit" << std::endl;
    while (running) {
        std::cout << "> " << std::flush;
        std::string inp;
        std::getline(std::cin, inp);
        if (inp.empty()) continue;
        std::istringstream iss(inp);
        std::string action;
        iss >> action;
        if (action == "send") {
            std::string to_login, msg;
            iss >> to_login;
            std::getline(iss, msg);
            if (to_login.empty() || msg.empty()) {
                std::cout << "Usage: send <to> <msg>" << std::endl;
                continue;
            }
            msg = msg.substr(1);
            send_command("send " + login + " " + to_login + " " + msg);
        } else if (action == "delayed") {
            std::string to_login, delay_str, msg;
            iss >> to_login >> delay_str;
            std::getline(iss, msg);
            if (to_login.empty() || delay_str.empty() || msg.empty()) {
                std::cout << "Usage: delayed <to> <delay> <msg>" << std::endl;
                continue;
            }
            msg = msg.substr(1);
            send_command("delayed " + login + " " + to_login + " " + delay_str + " " + msg);
        } else if (action == "exit") {
            running = false;
            break;
        } else {
            std::cout << "Unknown command" << std::endl;
        }
    }
}
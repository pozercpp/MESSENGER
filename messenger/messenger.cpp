#include"messenger.hpp"
#include"client.hpp"
#include"server.hpp"

#include <iostream>
#include <signal.h>
#include <cstdlib>

volatile bool running = true;

void signal_handler(int signum) {
    running = false;
}

Messenger::Messenger(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " server" << " or " << argv[0] << " client <login>" << std::endl;
        std::exit(1);
    }
    mode = argv[1];
    if (mode == "client") {
        if (argc != 3) {
            std::cout << "Usage for client: " << argv[0] << " client <login>" << std::endl;
            std::exit(1);
        }
        login = argv[2];
    } else if (mode != "server") {
        std::cout << "Invalid mode. Use 'server' or 'client <login>'" << std::endl;
        std::exit(1);
    }
}

void Messenger::run() {
    signal(SIGINT, signal_handler);
    if (mode == "server") {
        Server s;
        s.run();
    } else if (mode == "client") {
        Client c(login);
        c.run();
    }
}
#pragma once

#include <string>

void signal_handler(int signum);

extern volatile bool running;

class Messenger {
private:
    std::string mode;
    std::string login;

public:
    Messenger(int argc, char* argv[]);
    void run();
};
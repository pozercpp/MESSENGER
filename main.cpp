#include "messenger.hpp"

int main(int argc, char* argv[]) {
    Messenger messenger(argc, argv);
    messenger.run();
    return 0;
}
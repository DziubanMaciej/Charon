#include "charon/user_interface/console_user_interface.h"

#include <iostream>
#include <string>

void ConsoleUserInterface::runImpl() {
    std::string line{};
    while (true) {
        std::getline(std::cin, line);

        if (line == "q") {
            charon.stop();
            break;
        }
    }
}

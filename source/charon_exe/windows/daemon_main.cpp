#include "charon/charon/charon_main.h"

#pragma comment(linker, "/ENTRY:mainCRTStartup")

int main(int argc, char **argv) {
    return charonMain(argc, argv, true);
}

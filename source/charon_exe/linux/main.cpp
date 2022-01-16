#include "charon/charon/charon_main.h"
#include "charon/util/argument_parser.h"

int main(int argc, char **argv) {
    ArgumentParser argParser{argc, argv};
    ArgNames daemonNames{"-d", "--daemon"};
    const bool runAsDaemon = argParser.getArgumentValue<bool>(daemonNames, false);
    return charonMain(argc, argv, runAsDaemon);
}

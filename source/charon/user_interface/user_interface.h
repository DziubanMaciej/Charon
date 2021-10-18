#pragma once

#include "charon/charon/charon.h"

class UserInterface {
public:
    UserInterface(Charon &charon) : charon(charon) {}
    virtual ~UserInterface() {}

    virtual void run() = 0;

protected:
    Charon &charon;
};

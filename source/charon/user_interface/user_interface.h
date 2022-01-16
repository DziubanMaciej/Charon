#pragma once

#include "charon/charon/charon.h"

#include <mutex>

class UserInterface {
public:
    UserInterface(Charon &charon) : charon(charon) {}
    virtual ~UserInterface() {}

    void run() {
        std::lock_guard lock{mutex};
        instance = this;
        runImpl();
        instance = nullptr;
    }

    virtual void runImpl() = 0;
    virtual void kill() {}

protected:
    Charon &charon;
    static inline UserInterface *instance = nullptr;

private:
    static inline std::mutex mutex = {};
};

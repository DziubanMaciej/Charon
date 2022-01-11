#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

class Notification {
public:
    void signal();
    void reset();
    void wait(bool desiredState = true);

    bool isSignalled() const;

private:
    std::mutex mutex = {};
    std::condition_variable conditionVariable = {};
    std::atomic_bool signalled = false;
};

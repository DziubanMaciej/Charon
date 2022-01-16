#include "charon/util/notification.h"

void Notification::signal() {
    std::lock_guard lock{this->mutex};
    this->signalled = true;
    this->conditionVariable.notify_all();
}

void Notification::reset() {
    std::lock_guard lock{this->mutex};
    this->signalled = false;
    this->conditionVariable.notify_all();
}

void Notification::wait(bool desiredState) {
    std::unique_lock lock{this->mutex};
    while (this->signalled != desiredState) {
        this->conditionVariable.wait(lock);
    }
}

bool Notification::isSignalled() const {
    return this->signalled;
}

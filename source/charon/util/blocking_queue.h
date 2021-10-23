#pragma once

#include "charon/util/class_traits.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class BlockingQueue : NonCopyableAndMovable {
public:
    BlockingQueue() = default;
    BlockingQueue(const BlockingQueue &) = delete;
    BlockingQueue(BlockingQueue &&) = delete;

    void push(const T &value) {
        auto lock = this->lock();
        conditionVariable.notify_one();
        queue.push(value);
    }

    void push(T &&value) {
        auto lock = this->lock();
        conditionVariable.notify_one();
        queue.push(std::move(value));
    }

    bool blockingPop(T &result) {
        const auto infiniteTimeout = std::chrono::duration<double>(std::numeric_limits<double>::infinity());
        return blockingPop(result, infiniteTimeout);
    }

    template <class Rep, class Period>
    bool blockingPop(T &result, const std::chrono::duration<Rep, Period> &timeout) {
        auto lock = this->lock();

        // Wait for push notification
        while (empty() && !blockingPopInterrupted) {
            const auto waitResult = conditionVariable.wait_for(lock, timeout);
            if (waitResult == std::cv_status::timeout) {
                return false;
            }
        }
        if (blockingPopInterrupted) {
            blockingPopInterrupted = false;
            return false;
        }

        // Get the element
        result = std::move(queue.front());
        queue.pop();
        return true;
    }

    bool nonBlockingPop(T &result) {
        auto lock = this->lock();
        if (!empty()) {
            result = std::move(queue.front());
            queue.pop();
            return true;
        }
        return false;
    }

    bool empty() {
        return queue.empty();
    }

    bool size() const {
        return queue.size();
    }

    void interruptBlockingPop() {
        auto lock = this->lock();
        blockingPopInterrupted = true;
        conditionVariable.notify_all();
    }

    void clear() {
        auto lock = this->lock();
        std::queue<T> empty;
        std::swap(queue, empty);
        conditionVariable.notify_all();
    }

private:
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable conditionVariable;
    bool blockingPopInterrupted = false;

    std::unique_lock<std::mutex> lock() {
        return std::unique_lock<std::mutex>{this->mutex};
    }
};

#pragma once

struct NonCopyable {
    NonCopyable() = default;
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
    NonCopyable(NonCopyable &&) = default;
    NonCopyable &operator=(NonCopyable &&) = default;
};

struct NonMovable {
    NonMovable() = default;
    NonMovable(const NonMovable &) = default;
    NonMovable &operator=(const NonMovable &) = default;
    NonMovable(NonMovable &&) = delete;
    NonMovable &operator=(NonMovable &&) = delete;
};

struct NonCopyableAndMovable {
    NonCopyableAndMovable() = default;
    NonCopyableAndMovable(const NonCopyableAndMovable &) = delete;
    NonCopyableAndMovable &operator=(const NonCopyableAndMovable &) = delete;
    NonCopyableAndMovable(NonCopyableAndMovable &&) = delete;
    NonCopyableAndMovable &operator=(NonCopyableAndMovable &&) = delete;
};

struct NonInstantiatable {
    NonInstantiatable() = delete;
    NonInstantiatable(const NonInstantiatable &) = delete;
    NonInstantiatable &operator=(const NonInstantiatable &) = delete;
    NonInstantiatable(NonInstantiatable &&) = delete;
    NonInstantiatable &operator=(NonInstantiatable &&) = delete;
};

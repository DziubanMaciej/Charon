#pragma once

#include <ostream>

struct Time {
    virtual void writeCurrentTime(std::ostream &out) const = 0;
};

struct TimeImpl : Time {
    void writeCurrentTime(std::ostream &out) const override;
};

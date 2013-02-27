#ifndef CSIMTIME_H
#define CSIMTIME_H

#include <stdint.h>

enum time_metric {
    SECONDS =0,
    MILLISECONDS,
    NANOSECONDS,
    UNKNOWN
};



typedef uint64_t TIME;

extern TIME Infinity;

#endif
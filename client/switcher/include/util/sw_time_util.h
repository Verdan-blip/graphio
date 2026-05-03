#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

long get_current_time_millis() {
    struct timespec ts;
    
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        return 0;
    }

    return (long) ts.tv_sec * 1000L + (long)ts.tv_nsec / 1000000L;
}
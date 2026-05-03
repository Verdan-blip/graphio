#ifndef SW_EASING_H
#define SW_EASING_H

#include <math.h>

typedef double (*sw_easing_func)(double t);

static inline double sw_ease_linear(double t) {
    return t;
}

static inline double sw_ease_out_cubic(double t) {
    return 1.0 - pow(1.0 - t, 3.0);
}

static inline double sw_ease_out_back(double t) {
    const double c1 = 1.70158;
    const double c3 = c1 + 1.0;
    return 1.0 + c3 * pow(t - 1.0, 3.0) + c1 * pow(t - 1.0, 2.0);
}

#endif
#include <math.h>

#include "include/math/sw_vec2.h"

double sw_vec2_length(struct sw_vec2 v) {
    return sqrt(v.x * v.x + v.y * v.y);
}

double sw_vec2_dist(struct sw_vec2 a, struct sw_vec2 b) {
    struct sw_vec2 diff = sw_vec2_sub(a, b);
    return sw_vec2_length(diff);
}
#ifndef SW_VEC2_H
#define SW_VEC2_H

struct sw_vec2 {
    double x;
    double y;
};

static inline struct sw_vec2 sw_vec2_create(double x, double y) {
    struct sw_vec2 v = {x, y};
    return v;
}

static inline struct sw_vec2 sw_vec2_empty() {
    struct sw_vec2 v = {0, 0};
    return v;
}

static inline struct sw_vec2 sw_vec2_add(struct sw_vec2 a, struct sw_vec2 b) {
    struct sw_vec2 res = {a.x + b.x, a.y + b.y};
    return res;
}

static inline struct sw_vec2 sw_vec2_sub(struct sw_vec2 a, struct sw_vec2 b) {
    struct sw_vec2 res = {a.x - b.x, a.y - b.y};
    return res;
}

static inline struct sw_vec2 sw_vec2_translate(struct sw_vec2 a, double s) {
    struct sw_vec2 res = {a.x + s, a.y + s};
    return res;
}

static inline struct sw_vec2 sw_vec2_mul(struct sw_vec2 a, double s) {
    struct sw_vec2 res = {a.x * s, a.y * s};
    return res;
}

static inline struct sw_vec2 sw_vec2_lerp(struct sw_vec2 a, struct sw_vec2 b, double t) {
    return sw_vec2_add(a, sw_vec2_mul(sw_vec2_sub(b, a), t));
}

double sw_vec2_length(struct sw_vec2 v);
double sw_vec2_dist(struct sw_vec2 a, struct sw_vec2 b);

#endif
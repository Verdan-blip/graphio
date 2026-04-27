#ifndef SW_COLOR_H
#define SW_COLOR_H

struct sw_color {
    union {
        struct {
            double r;
            double g;
            double b;
            double a;
        };
        double channels[4];
    };
};

static inline struct sw_color sw_color_create(double r, double g, double b, double a) {
    struct sw_color c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    return c;
}

static inline struct sw_color sw_color_lerp(struct sw_color start, struct sw_color end, double t) {
    struct sw_color res;
    res.r = start.r + (end.r - start.r) * t;
    res.g = start.g + (end.g - start.g) * t;
    res.b = start.b + (end.b - start.b) * t;
    res.a = start.a + (end.a - start.a) * t;
    return res;
}

#endif
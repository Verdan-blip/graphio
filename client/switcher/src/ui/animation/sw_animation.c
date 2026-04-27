#include <stdlib.h>

#include "include/ui/animation/sw_animation.h"

static void update_double(struct sw_animation *a, double p) {
    *(double*)a->target = a->values.d.start + (a->values.d.end - a->values.d.start) * p;
}

static void update_vec2(struct sw_animation *a, double p) {
    *(struct sw_vec2*)a->target = sw_vec2_lerp(a->values.v.start, a->values.v.end, p);
}

static void update_color(struct sw_animation *a, double p) {
    *(struct sw_color*)a->target = sw_color_lerp(a->values.c.start, a->values.c.end, p);
}

struct sw_animation* sw_animation_create_double(double *target, double end, int ms, sw_easing_func e) {
    struct sw_animation *a = calloc(1, sizeof(struct sw_animation));
    a->target = target;
    a->values.d.start = *target;
    a->values.d.end = end;
    a->duration = ms;
    a->easing = e;
    a->update_cb = update_double;
    return a;
}

struct sw_animation* sw_animation_create_vec2(struct sw_vec2 *target, struct sw_vec2 end, int ms, sw_easing_func e) {
    struct sw_animation *a = calloc(1, sizeof(struct sw_animation));
    a->target = target;
    a->values.v.start = *target;
    a->values.v.end = end;
    a->duration = ms;
    a->easing = e;
    a->update_cb = update_vec2;
    return a;
}

void sw_animation_destroy(struct sw_animation *self) {
    free(self);
}
#ifndef SW_ANIMATION_H
#define SW_ANIMATION_H

#include <wayland-util.h>
#include "include/math/sw_vec2.h"
#include "include/math/sw_color.h"

#include "include/ui/animation/sw_easing.h"

struct sw_animation;

typedef void (*sw_animation_update_pt)(struct sw_animation *self, double progress);

struct sw_animation {
    struct wl_list link;
    
    void *target;
    double elapsed;
    int duration;
    sw_easing_func easing;
    sw_animation_update_pt update_cb;

    union {
        struct { 
            double start; 
            double end; 
        } d;

        struct { 
            struct sw_vec2 start; 
            struct sw_vec2 end; 
        } v;

        struct { 
            struct sw_color start; 
            struct sw_color end; 
        } c;

        struct {
            struct sw_vec2 start;
            struct sw_vec2 end;
            double max_squash;
            struct sw_vec2 *target_size;
            struct sw_vec2 base_size;
        } squash_move;

    } values;
};

struct sw_animation* sw_animation_create_double(
    double *target, 
    double end, 
    int ms, 
    sw_easing_func e
);

struct sw_animation* sw_animation_create_vec2(
    struct sw_vec2 *target, 
    struct sw_vec2 end, 
    int ms, 
    sw_easing_func e
);

struct sw_animation* sw_animation_create_color(
    struct sw_color *target, 
    struct sw_color end, 
    int ms, 
    sw_easing_func e
);

struct sw_animation* sw_animation_create_squash_move(
    struct sw_vec2 *target_pos, 
    struct sw_vec2 *target_size, 
    struct sw_vec2 base_size,
    struct sw_vec2 end_pos, 
    double max_squash, 
    int ms, 
    sw_easing_func e
);

void sw_animation_destroy(struct sw_animation *self);

#endif
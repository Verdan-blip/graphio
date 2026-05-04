#include <stdlib.h>

#include "include/ui/animation/sw_animation.h"
#include "glib.h"

static void update_double(struct sw_animation *a, double p) {
    *(double*)a->target = a->values.d.start + (a->values.d.end - a->values.d.start) * p;
}

static void update_vec2(struct sw_animation *a, double p) {
    *(struct sw_vec2*)a->target = sw_vec2_lerp(a->values.v.start, a->values.v.end, p);
}

static void update_color(struct sw_animation *a, double p) {
    *(struct sw_color*)a->target = sw_color_lerp(a->values.c.start, a->values.c.end, p);
}

static void update_squash_move(struct sw_animation *a, double p) {
    struct sw_vec2 *pos = (struct sw_vec2 *)a->target;
    struct sw_vec2 lerped_pos = sw_vec2_lerp(a->values.squash_move.start, a->values.squash_move.end, p);

    double effect = sin(G_PI * p);
    
    struct sw_vec2 *size = a->values.squash_move.target_size;
    struct sw_vec2 base = a->values.squash_move.base_size;

    double dx = fabs(a->values.squash_move.end.x - a->values.squash_move.start.x);
    double dy = fabs(a->values.squash_move.end.y - a->values.squash_move.start.y);
    double dist = sqrt(dx * dx + dy * dy);

    if (dist < 0.001) {
        *size = base;
        *pos = lerped_pos;
        return;
    }

    double weight_x = dx / dist;
    double weight_y = dy / dist;

    double squash_x = 1.0 - (weight_y * effect * a->values.squash_move.max_squash);
    double squash_y = 1.0 - (weight_x * effect * a->values.squash_move.max_squash);

    size->x = base.x * squash_x;
    size->y = base.y * squash_y;

    struct sw_vec2 offset = {
        .x = (base.x - size->x) / 2.0,
        .y = (base.y - size->y) / 2.0
    };

    pos->x = lerped_pos.x + offset.x;
    pos->y = lerped_pos.y + offset.y;
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

struct sw_animation* sw_animation_create_squash_move(
    struct sw_vec2 *target_pos, 
    struct sw_vec2 *target_size, 
    struct sw_vec2 base_size,
    struct sw_vec2 end_pos, 
    double max_squash, 
    int ms, 
    sw_easing_func e
) {
    struct sw_animation *a = calloc(1, sizeof(struct sw_animation));
    a->target = target_pos;
    
    a->values.squash_move.target_size = target_size;
    a->values.squash_move.base_size = base_size;
    
    a->values.squash_move.start = *target_pos;
    a->values.squash_move.end = end_pos;
    a->values.squash_move.max_squash = max_squash;
    
    a->duration = ms;
    a->easing = e;
    a->update_cb = update_squash_move;
    
    return a;
}

void sw_animation_destroy(struct sw_animation *self) {
    free(self);
}
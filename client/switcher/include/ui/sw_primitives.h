#ifndef SW_PRIMITIVES_H
#define SW_PRIMITIVES_H

#include "cairo.h"

#include "include/math/sw_vec2.h"
#include "include/math/sw_color.h"

void sw_draw_filled_round_corner_rect(
    struct sw_vec2 pos,
    struct sw_vec2 size,
    double r,
    struct sw_color color,
    cairo_t *cr
);

void sw_draw_dashed_round_corner_rect(
    struct sw_vec2 pos,
    struct sw_vec2 size,
    double r,
    double stroke,
    struct sw_color color,
    cairo_t *cr
);

void sw_draw_outlined_round_corner_rect(
    struct sw_vec2 pos,
    struct sw_vec2 size,
    double r,
    double stroke,
    struct sw_color color,
    cairo_t *cr
);

#endif
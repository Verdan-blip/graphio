#ifndef SW_PRIMITIVES_H
#define SW_PRIMITIVES_H

#include "cairo.h"

void sw_draw_filled_round_corner_rect(
    double x, double y,
    double width, double height,
    double corner_radius,
    float* color,
    cairo_t *cr
);

void sw_draw_dashed_round_corner_rect(
    double x, double y,
    double w, double h,
    double r,
    double stroke,
    float* color,
    cairo_t *cr
);

void sw_draw_outlined_round_corner_rect(
    double x, double y,
    double w, double h,
    double r,
    double stroke,
    float* color,
    cairo_t *cr
);

#endif
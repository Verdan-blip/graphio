#include "glib.h"

#include "include/ui/sw_primitives.h"
#include "include/math/sw_color.h"

#include "include/math/sw_vec2.h"

void sw_draw_filled_round_corner_rect(
    struct sw_vec2 pos,
    struct sw_vec2 size,
    double r,
    struct sw_color color,
    cairo_t *cr
) {
    if (r > size.x / 2) r = size.x / 2;
    if (r > size.y / 2) r = size.y / 2;
    if (r < 0) r = 0;

    cairo_save(cr);

    cairo_set_source_rgba(
        cr, 
        color.r,
        color.g,
        color.b,
        color.a
    );

    cairo_new_sub_path(cr);

    if (r == 0) {
        cairo_rectangle(cr, pos.x, pos.y, size.x, size.y);
    } else {
        cairo_move_to(cr, pos.x + r, pos.y);
        
        cairo_line_to(cr, pos.x + size.x - r, pos.y);
        cairo_arc(cr, pos.x + size.x - r, pos.y + r, r, -G_PI / 2, 0);
        
        cairo_line_to(cr, pos.x + size.x, pos.y + size.y - r);
        cairo_arc(cr, pos.x + size.x - r, pos.y + size.y - r, r, 0, G_PI / 2);
        

        cairo_line_to(cr, pos.x + r, pos.y + size.y);
        cairo_arc(cr, pos.x + r, pos.y + size.y - r, r, G_PI / 2, G_PI);
        
        cairo_line_to(cr, pos.x, pos.y + r);
        cairo_arc(cr, pos.x + r, pos.y + r, r, G_PI, 3 * G_PI / 2);
        
        cairo_close_path(cr);
    }

    cairo_fill(cr);
    cairo_restore(cr);
}

void sw_draw_dashed_round_corner_rect(
    struct sw_vec2 pos,
    struct sw_vec2 size,
    double r,
    double stroke,
    struct sw_color color,
    cairo_t *cr
) {
    cairo_set_source_rgba(
        cr, 
        color.r,
        color.g,
        color.b,
        color.a
    );

    double dashes[] = { 10.0, 10.0 }; 
    int num_dashes = 2;
    double offset = 0;

    cairo_set_dash(cr, dashes, num_dashes, offset);

    cairo_new_sub_path(cr);
    cairo_arc(cr, pos.x + size.x - r, pos.y + r,     r, -G_PI / 2, 0);
    cairo_arc(cr, pos.x + size.x - r, pos.y + size.y - r, r, 0, G_PI / 2);
    cairo_arc(cr, pos.x + r,     pos.y + size.y - r, r, G_PI / 2, G_PI);
    cairo_arc(cr, pos.x + r,     pos.y + r,     r, G_PI, 3 * G_PI / 2);
    cairo_close_path(cr);

    cairo_set_line_width(cr, stroke);
    cairo_stroke(cr);
    cairo_set_dash(cr, NULL, 0, 0);
}

void sw_draw_outlined_round_corner_rect(
    struct sw_vec2 pos,
    struct sw_vec2 size,
    double r,
    double stroke,
    struct sw_color color,
    cairo_t *cr
) {
    cairo_set_source_rgba(
        cr, 
        color.r,
        color.g,
        color.b,
        color.a
    );

    cairo_new_sub_path(cr);
    cairo_arc(cr, pos.x + size.x - r, pos.y + r,     r, -G_PI / 2, 0);
    cairo_arc(cr, pos.x + size.x - r, pos.y + size.y - r, r, 0, G_PI / 2);
    cairo_arc(cr, pos.x + r,     pos.y + size.y - r, r, G_PI / 2, G_PI);
    cairo_arc(cr, pos.x + r,     pos.y + r,     r, G_PI, 3 * G_PI / 2);
    cairo_close_path(cr);

    cairo_set_line_width(cr, stroke);
    cairo_stroke(cr);
}
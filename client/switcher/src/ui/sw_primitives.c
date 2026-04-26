#include "include/ui/sw_primitives.h"
#include "glib.h"

void sw_draw_filled_round_corner_rect(
    double x, double y,
    double w, double h,
    double r,
    float* color,
    cairo_t *cr
) {
    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;
    if (r < 0) r = 0;

    cairo_save(cr);

    cairo_set_source_rgba(
        cr, 
        color[0],
        color[1],
        color[2],
        color[3]
    );

    cairo_new_sub_path(cr);

    if (r == 0) {
        cairo_rectangle(cr, x, y, w, h);
    } else {
        // Явное построение пути с дугами и линиями
        cairo_move_to(cr, x + r, y);
        
        // Верхняя грань -> Правая верхняя дуга
        cairo_line_to(cr, x + w - r, y);
        cairo_arc(cr, x + w - r, y + r, r, -G_PI / 2, 0);
        
        // Правая грань -> Правая нижняя дуга
        cairo_line_to(cr, x + w, y + h - r);
        cairo_arc(cr, x + w - r, y + h - r, r, 0, G_PI / 2);
        
        // Нижняя грань -> Левая нижняя дуга
        cairo_line_to(cr, x + r, y + h);
        cairo_arc(cr, x + r, y + h - r, r, G_PI / 2, G_PI);
        
        // Левая грань -> Левая верхняя дуга
        cairo_line_to(cr, x, y + r);
        cairo_arc(cr, x + r, y + r, r, G_PI, 3 * G_PI / 2);
        
        cairo_close_path(cr);
    }

    cairo_fill(cr);
    cairo_restore(cr);
}

void sw_draw_dashed_round_corner_rect(
    double x, double y,
    double w, double h,
    double r,
    double stroke,
    float* color,
    cairo_t *cr
) {
    cairo_set_source_rgba(
        cr, 
        color[0],
        color[1],
        color[2],
        color[3]
    );

    double dashes[] = { 10.0, 10.0 }; 
    int num_dashes = 2;
    double offset = 0;

    cairo_set_dash(cr, dashes, num_dashes, offset);

    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - r, y + r,     r, -G_PI / 2, 0);
    cairo_arc(cr, x + w - r, y + h - r, r, 0, G_PI / 2);
    cairo_arc(cr, x + r,     y + h - r, r, G_PI / 2, G_PI);
    cairo_arc(cr, x + r,     y + r,     r, G_PI, 3 * G_PI / 2);
    cairo_close_path(cr);

    cairo_set_line_width(cr, stroke);
    cairo_stroke(cr);
    cairo_set_dash(cr, NULL, 0, 0);
}

void sw_draw_outlined_round_corner_rect(
    double x, double y,
    double w, double h,
    double r,
    double stroke,
    float* color,
    cairo_t *cr
) {
    cairo_set_source_rgba(
        cr, 
        color[0],
        color[1],
        color[2],
        color[3]
    );

    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - r, y + r,     r, -G_PI / 2, 0);
    cairo_arc(cr, x + w - r, y + h - r, r, 0, G_PI / 2);
    cairo_arc(cr, x + r,     y + h - r, r, G_PI / 2, G_PI);
    cairo_arc(cr, x + r,     y + r,     r, G_PI, 3 * G_PI / 2);
    cairo_close_path(cr);

    cairo_set_line_width(cr, stroke);
    cairo_stroke(cr);
}
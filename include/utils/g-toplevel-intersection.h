#ifndef WLR_SURFACE_UTILS_H
#define WLR_SURFACE_UTILS_H

#include "../../include/g-toplevel.h"
#include "../../include/g-popup.h"
#include <stdbool.h>

struct g_toplevel_intersection_info {
    bool status;
    struct wlr_surface *nearest_surface;
    double surface_pos_x, surface_pos_y;

    struct g_toplevel *toplevel;
    struct g_popup *popup;
};

struct g_toplevel_intersection_info g_toplevel_at(struct wl_list *toplevels, double pos_x, double pos_y);

#endif
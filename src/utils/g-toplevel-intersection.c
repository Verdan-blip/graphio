#include "../../include/utils/g-toplevel-intersection.h"
#include <wayland-util.h>

static struct g_toplevel_intersection_info empty_info = { 0 };

static struct g_toplevel_intersection_info g_popup_at(struct g_toplevel *toplevel, double pos_x, double pos_y) {
    struct g_popup *popup;

    double surface_x, surface_y;
    wl_list_for_each_reverse(popup, &toplevel->popups, link) {
        double local_x = pos_x - toplevel->pos_x - popup->pos_x;
        double local_y = pos_y - toplevel->pos_y - popup->pos_y;

        struct wlr_surface *surface = wlr_surface_surface_at(
            popup->surface, 
            local_x, local_y,
            &surface_x, &surface_y
        );

        if (surface) {
            return (struct g_toplevel_intersection_info) {
                .nearest_surface = surface,
                .surface_pos_x = surface_x,
                .surface_pos_y = surface_y,
                .popup = popup,
                .toplevel = toplevel,
                .status = true
            };
        }
    }

    return empty_info;
}

struct g_toplevel_intersection_info g_toplevel_at(struct wl_list *toplevels, double pos_x, double pos_y) {
    double surface_x, surface_y;

    struct g_toplevel *toplevel;
    wl_list_for_each_reverse(toplevel, toplevels, link) {
        if (!toplevel->mapped) continue;

        struct g_toplevel_intersection_info info = g_popup_at(toplevel, pos_x, pos_y);
        if (info.status) {
            return info;
        }

        double local_x = pos_x - toplevel->pos_x;
        double local_y = pos_y - toplevel->pos_y;

        struct wlr_surface *surface = wlr_surface_surface_at(
            toplevel->surface, 
            local_x, local_y,
            &surface_x, &surface_y
        );

        if (surface) {
            return (struct g_toplevel_intersection_info) {
                .nearest_surface = surface,
                .surface_pos_x = surface_x,
                .surface_pos_y = surface_y,
                .popup = NULL,
                .toplevel = toplevel,
                .status = true
            };
        }
    }

    return empty_info;
}
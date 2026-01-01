#include <stdlib.h>
#include "../include/g-popup.h"
#include "../include/g-server.h"
#include "wlr/util/log.h"

struct g_popup* g_popup_create(struct g_server *server, struct wlr_xdg_popup *xdg_popup) {
    struct g_popup *popup = calloc(1, sizeof(struct g_popup));
    popup->server = server;

    struct wlr_xdg_surface *parent = wlr_xdg_surface_try_from_wlr_surface(xdg_popup->parent);
    if (!parent) {
        wlr_log(WLR_ERROR, "No parent found for popup, cancelling creation");
        return NULL;
    }

    popup->xdg_popup = xdg_popup;
    popup->surface = xdg_popup->base->surface;

    popup->commit_listener.notify = g_popup_on_commit;
	wl_signal_add(&xdg_popup->base->surface->events.commit, &popup->commit_listener);

    popup->map_listener.notify = g_popup_on_map;
	wl_signal_add(&xdg_popup->base->surface->events.map, &popup->map_listener);

    popup->unmap_listener.notify = g_popup_on_unmap;
	wl_signal_add(&xdg_popup->base->surface->events.unmap, &popup->unmap_listener);

	popup->destroy_listener.notify = g_popup_on_destroy;
	wl_signal_add(&xdg_popup->events.destroy, &popup->destroy_listener);

    return popup;
}

void g_popup_on_map(struct wl_listener *listener, void *data) {
    struct g_popup *popup = wl_container_of(listener, popup, map_listener);
    popup->mapped = true;
}

void g_popup_on_unmap(struct wl_listener *listener, void *data) {
    struct g_popup *popup = wl_container_of(listener, popup, unmap_listener);
    popup->mapped = false;
}

void g_popup_on_commit(struct wl_listener *listener, void *data) {
    struct g_popup *popup = wl_container_of(listener, popup, commit_listener);

	if (popup->xdg_popup->base->initial_commit) {
		wlr_xdg_surface_schedule_configure(popup->xdg_popup->base);
	}

    popup->width = popup->xdg_popup->base->geometry.width;
    popup->height = popup->xdg_popup->base->geometry.height;
}

void g_popup_on_destroy(struct wl_listener *listener, void *data) {
    struct g_popup *popup = wl_container_of(listener, popup, destroy_listener);

	wl_list_remove(&popup->commit_listener.link);
    wl_list_remove(&popup->map_listener.link);
    wl_list_remove(&popup->unmap_listener.link);
	wl_list_remove(&popup->destroy_listener.link);
    wl_list_remove(&popup->link);

	free(popup);
}

// Contract
struct g_popup_render_data {
    struct g_popup *popup;
    struct wlr_render_pass *pass;
};

void g_popup_send_frame_done(struct g_popup *popup, struct timespec *now) {
    wlr_surface_send_frame_done(popup->surface, now);
}

// Utils
struct g_popup* g_popup_at(
    struct wl_list *popups, 
    double x, double y,
    double *surface_x, double *surface_y
) {
    struct g_popup *popup;
    wl_list_for_each_reverse(popup, popups, link) {
        if (!popup->mapped) continue;
        
        double local_x = x - popup->pos_x;
        double local_y = y - popup->pos_y;
        
        struct wlr_surface *surface = wlr_xdg_surface_surface_at(
            popup->xdg_popup->base, 
            local_x, local_y, 
            surface_x, surface_y
        );
        
        if (surface) {
            return popup;
        }
    }

    return NULL;
}
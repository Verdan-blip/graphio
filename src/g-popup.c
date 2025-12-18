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

	if (popup->xdg_popup->base->initial_commit) {
		wlr_xdg_surface_schedule_configure(popup->xdg_popup->base);
	}
    
    popup->mapped = true;
}

void g_popup_on_unmap(struct wl_listener *listener, void *data) {
    struct g_popup *popup = wl_container_of(listener, popup, unmap_listener);

	if (popup->xdg_popup->base->initial_commit) {
		wlr_xdg_surface_schedule_configure(popup->xdg_popup->base);
	}

    popup->mapped = false;
}

void g_popup_on_commit(struct wl_listener *listener, void *data) {
    struct g_popup *popup = wl_container_of(listener, popup, commit_listener);

	if (popup->xdg_popup->base->initial_commit) {
		wlr_xdg_surface_schedule_configure(popup->xdg_popup->base);
	}

    struct wlr_fbox window_geo_box;
    wlr_surface_get_buffer_source_box(popup->surface, &window_geo_box);

    popup->width = window_geo_box.width;
    popup->height = window_geo_box.height;
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

static void g_popup_on_each_xdg_surface(
    struct wlr_surface *s, 
    int sx, int sy, 
    void *data
) {
    struct g_popup_render_data *render_data = data;
    struct wlr_texture *texture = wlr_surface_get_texture(s);

    if (!texture) {
        wlr_log(WLR_ERROR, "No texture found for popup surface, skipping render");
        return;
    }

    const float alpha = 1.0f;

    wlr_render_pass_add_texture(render_data->pass, &(struct wlr_render_texture_options) {
        .texture = texture,
        .alpha = &alpha,
        .src_box = { 0, 0, s->current.width, s->current.height },
        .dst_box = { 
            render_data->popup->pos_x, 
            render_data->popup->pos_y, 
            s->current.width, 
            s->current.height 
        }
    });
}

void g_popup_on_render_pass(struct g_popup *popup, struct wlr_render_pass *pass) {
    wlr_xdg_surface_for_each_surface(
        popup->xdg_popup->base, 
        g_popup_on_each_xdg_surface, 
        &(struct g_popup_render_data) {
            .popup = popup,
            .pass = pass
        }
    );
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
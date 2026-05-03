#ifndef G_POPUP_H
#define G_POPUP_H

#include <wayland-server-core.h>

struct g_server;

struct g_popup {
	struct wlr_xdg_popup *xdg_popup;
	struct wl_listener commit;
	struct wl_listener destroy;
};

void g_popup_init(
    struct g_server *server,
    struct wlr_xdg_popup *wlr_xdg_popup
);

#endif
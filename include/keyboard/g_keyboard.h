#ifndef G_KEYBOARD_H
#define G_KEYBOARD_H

#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_input_device.h>

struct g_server;

struct g_keyboard {
	struct wl_list link;
	struct g_server *server;
	struct wlr_keyboard *wlr_keyboard;

	struct wl_listener modifiers;
	struct wl_listener key;
	struct wl_listener destroy;
};

struct g_keyboard* g_keyboard_create(
    struct g_server *server, 
    struct wlr_input_device *device
);

#endif

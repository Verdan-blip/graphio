#ifndef G_KEYBOARD_H
#define G_KEYBOARD_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_keyboard.h>

struct g_server;

struct g_keyboard {
    struct wl_list link;

    struct g_server *server;

    struct wlr_keyboard *wlr_keyboard;

    struct wl_listener modifier_listener;
    struct wl_listener key_listener;
    struct wl_listener destroy_listener;
};

struct g_keyboard* g_keyboard_create(struct g_server *server, struct wlr_input_device *device);

void g_keyboard_on_modifier(struct wl_listener *listener, void *data);
void g_keyboard_on_key(struct wl_listener *listener, void *data);
void g_keyboard_on_destroy(struct wl_listener *listener, void *data);

#endif
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_input_device.h>
#include <xkbcommon/xkbcommon.h>
#include "../include/g-server.h"
#include "../include/g-keyboard.h"

struct g_keyboard* g_keyboard_create(struct g_server *server, struct wlr_input_device *device) {
    struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);

    struct g_keyboard *keyboard = calloc(1, sizeof(struct g_keyboard));
    keyboard->server = server;

    keyboard->wlr_keyboard = wlr_keyboard;

    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_rule_names rules = { 0 };
    struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, &rules, XKB_KEYMAP_COMPILE_NO_FLAGS);

    wlr_keyboard_set_keymap(wlr_keyboard, keymap);

    xkb_keymap_unref(keymap);
	xkb_context_unref(context);
	wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);

    keyboard->modifier_listener.notify = g_keyboard_on_modifier;
    wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->modifier_listener);

    keyboard->key_listener.notify = g_keyboard_on_key;
    wl_signal_add(&wlr_keyboard->events.key, &keyboard->key_listener);

    keyboard->destroy_listener.notify = g_keyboard_on_destroy;
    wl_signal_add(&device->events.destroy, &keyboard->destroy_listener);

    wlr_seat_set_keyboard(server->seat->wlr_seat, wlr_keyboard);

    return keyboard;
}

void g_keyboard_on_modifier(struct wl_listener *listener, void *data) {
    struct g_keyboard *keyboard = wl_container_of(listener, keyboard, modifier_listener);
    struct g_server *server = keyboard->server;

    wlr_seat_set_keyboard(server->seat->wlr_seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_notify_modifiers(server->seat->wlr_seat, &keyboard->wlr_keyboard->modifiers);
}

void g_keyboard_on_key(struct wl_listener *listener, void *data) {
    struct g_keyboard *keyboard = wl_container_of(listener, keyboard, key_listener);
    struct g_server *server = keyboard->server;

    struct wlr_keyboard_key_event *event = data;
    struct wlr_seat *seat = server->seat->wlr_seat;

    uint32_t keycode = event->keycode + 8;
    const xkb_keysym_t *syms;

    int nsyms = xkb_state_key_get_syms(keyboard->wlr_keyboard->xkb_state, keycode, &syms);
    
    wlr_seat_set_keyboard(seat, keyboard->wlr_keyboard);
	wlr_seat_keyboard_notify_key(seat, event->time_msec, event->keycode, event->state);
}

void g_keyboard_on_destroy(struct wl_listener *listener, void *data) {
    struct g_keyboard *keyboard = wl_container_of(listener, keyboard, destroy_listener);
    wlr_seat_set_keyboard(keyboard->server->seat->wlr_seat, NULL);

    wl_list_remove(&keyboard->modifier_listener.link);
	wl_list_remove(&keyboard->key_listener.link);
	wl_list_remove(&keyboard->destroy_listener.link);
	wl_list_remove(&keyboard->link);
    free(keyboard);
}
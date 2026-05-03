#include <stdlib.h>

#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_scene.h>

#include "include/g_server.h"
#include "include/keyboard/g_keyboard.h"

static void keyboard_handle_modifiers(struct wl_listener *listener, void *data) {
	struct g_keyboard *keyboard = wl_container_of(listener, keyboard, modifiers);

	wlr_seat_set_keyboard(keyboard->server->seat, keyboard->wlr_keyboard);
	wlr_seat_keyboard_notify_modifiers(keyboard->server->seat, &keyboard->wlr_keyboard->modifiers);
}

static bool handle_keybinding(struct g_server *server, xkb_keysym_t sym) {
	switch (sym) {
	case XKB_KEY_Escape: {
		wl_display_terminate(server->wl_display);
		break;
	}

	default:
		return false;
	}
	return true;
}

static bool handle_key_press(struct g_server *server, struct g_keyboard *keyboard, xkb_keysym_t sym) {
	switch (sym) {
	case XKB_KEY_Super_L: {
		wlr_scene_node_set_enabled(&server->foregound_tree->node, true);

		if (server->switcher_surface) {
			wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);
			wlr_seat_keyboard_notify_enter(
					server->seat, 
				server->switcher_surface,
				keyboard->wlr_keyboard->keycodes, 
				keyboard->wlr_keyboard->num_keycodes, 
				&keyboard->wlr_keyboard->modifiers
			);
		}
		return true;
	}
	default:
		return false;
	}
	return true;
}

static bool handle_key_release(struct g_server *server, xkb_keysym_t sym) {
	return false;
}

static void keyboard_handle_key(struct wl_listener *listener, void *data) {
	struct g_keyboard *keyboard = wl_container_of(listener, keyboard, key);
	struct g_server *server = keyboard->server;
	struct wlr_keyboard_key_event *event = data;
	struct wlr_seat *seat = server->seat;

	uint32_t keycode = event->keycode + 8;

	const xkb_keysym_t *syms;
	int nsyms = xkb_state_key_get_syms(keyboard->wlr_keyboard->xkb_state, keycode, &syms);

	uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);

	bool handled = false;
	if ((modifiers & WLR_MODIFIER_ALT) && event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		for (int i = 0; i < nsyms; i++) {
			handled = handle_keybinding(server, syms[i]);
		}
	}

	if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		for (int i = 0; i < nsyms; i++) {
			handled = handle_key_press(server, keyboard, syms[i]);
		}
	}

	if (event->state == WL_KEYBOARD_KEY_STATE_RELEASED) {
		for (int i = 0; i < nsyms; i++) {
			handled = handle_key_release(server, syms[i]);
		}
	}

	if (!handled) {
		wlr_seat_set_keyboard(seat, keyboard->wlr_keyboard);
		wlr_seat_keyboard_notify_key(seat, event->time_msec,
			event->keycode, event->state);
	}
}

static void keyboard_handle_destroy(struct wl_listener *listener, void *data) {
	struct g_keyboard *keyboard = wl_container_of(listener, keyboard, destroy);
	wl_list_remove(&keyboard->modifiers.link);
	wl_list_remove(&keyboard->key.link);
	wl_list_remove(&keyboard->destroy.link);
	wl_list_remove(&keyboard->link);
	free(keyboard);
}

struct g_keyboard* g_keyboard_create(
    struct g_server *server, 
    struct wlr_input_device *device
) {
	struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);

	struct g_keyboard *keyboard = calloc(1, sizeof(*keyboard));
	keyboard->server = server;
	keyboard->wlr_keyboard = wlr_keyboard;

	struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, NULL,
		XKB_KEYMAP_COMPILE_NO_FLAGS);

	wlr_keyboard_set_keymap(wlr_keyboard, keymap);
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);
	wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);

	keyboard->modifiers.notify = keyboard_handle_modifiers;
	wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->modifiers);
	keyboard->key.notify = keyboard_handle_key;
	wl_signal_add(&wlr_keyboard->events.key, &keyboard->key);
	keyboard->destroy.notify = keyboard_handle_destroy;
	wl_signal_add(&device->events.destroy, &keyboard->destroy);

	wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);

    return keyboard;
}
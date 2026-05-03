#include <stdlib.h>
#include <assert.h>

#include <wlr/types/wlr_scene.h>

#include "include/g_server.h"
#include "include/popup/g_popup.h"

static void xdg_popup_commit(struct wl_listener *listener, void *data) {
	struct g_popup *popup = wl_container_of(listener, popup, commit);

	if (popup->xdg_popup->base->initial_commit) {
		wlr_xdg_surface_schedule_configure(popup->xdg_popup->base);
	}
}

static void xdg_popup_on_destroy(struct wl_listener *listener, void *data) {
	struct g_popup *popup = wl_container_of(listener, popup, destroy);

	wl_list_remove(&popup->commit.link);
	wl_list_remove(&popup->destroy.link);

	free(popup);
}

void g_popup_init(
    struct g_server *server,
    struct wlr_xdg_popup *xdg_popup
) {
    struct g_popup *popup = calloc(1, sizeof(struct g_popup));
	popup->xdg_popup = xdg_popup;

	struct wlr_xdg_surface *parent = wlr_xdg_surface_try_from_wlr_surface(xdg_popup->parent);
	assert(parent != NULL);
	struct wlr_scene_tree *parent_tree = parent->data;
	xdg_popup->base->data = wlr_scene_xdg_surface_create(parent_tree, xdg_popup->base);

	popup->commit.notify = xdg_popup_commit;
	wl_signal_add(&xdg_popup->base->surface->events.commit, &popup->commit);

	popup->destroy.notify = xdg_popup_on_destroy;
	wl_signal_add(&xdg_popup->events.destroy, &popup->destroy);
}
#include <stdlib.h>

#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>

#include "include/output/g_output.h"
#include "include/g_server.h"

static float DEFAULT_OUTPUT_BACKGROUND_COLOR[4] = { 0.1f, 0.2f, 0.3f, 1.0f };

static void output_frame(struct wl_listener *listener, void *data) {
	struct g_output *output = wl_container_of(listener, output, frame);
	struct wlr_scene *scene = output->server->scene;

	struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(scene, output->wlr_output);

	wlr_scene_output_commit(scene_output, NULL);

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	wlr_scene_output_send_frame_done(scene_output, &now);
}

static void output_request_state(struct wl_listener *listener, void *data) {
	struct g_output *output = wl_container_of(listener, output, request_state);
	const struct wlr_output_event_request_state *event = data;

	wlr_output_commit_state(output->wlr_output, event->state);
}

static void output_destroy(struct wl_listener *listener, void *data) {
	struct g_output *output = wl_container_of(listener, output, destroy);

	wl_list_remove(&output->frame.link);
	wl_list_remove(&output->request_state.link);
	wl_list_remove(&output->destroy.link);
	wl_list_remove(&output->link);
	free(output);
}

static void set_backgroud_default_color(struct g_server *server, struct wlr_output *wlr_output) {
    wlr_scene_rect_create(
		server->background_tree, 
		wlr_output->width, 
		wlr_output->height, 
		DEFAULT_OUTPUT_BACKGROUND_COLOR
	);
}

struct g_output* g_output_create(
    struct g_server *server,
    struct wlr_output *wlr_output
) {
	wlr_output_init_render(wlr_output, server->allocator, server->renderer);

	struct wlr_output_state state;
	wlr_output_state_init(&state);
	wlr_output_state_set_enabled(&state, true);

	struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
	if (mode != NULL) {
		wlr_output_state_set_mode(&state, mode);
	}

	wlr_output_commit_state(wlr_output, &state);
	wlr_output_state_finish(&state);

	struct g_output *output = calloc(1, sizeof(*output));
	output->wlr_output = wlr_output;
	output->server = server;

	output->frame.notify = output_frame;
	wl_signal_add(&wlr_output->events.frame, &output->frame);

	output->request_state.notify = output_request_state;
	wl_signal_add(&wlr_output->events.request_state, &output->request_state);

	output->destroy.notify = output_destroy;
	wl_signal_add(&wlr_output->events.destroy, &output->destroy);

	struct wlr_output_layout_output *l_output = wlr_output_layout_add_auto(server->output_layout, wlr_output);
	struct wlr_scene_output *scene_output = wlr_scene_output_create(server->scene, wlr_output);

	wlr_scene_output_layout_add_output(server->scene_layout, l_output, scene_output);

	set_backgroud_default_color(server, wlr_output);

    return output;
}
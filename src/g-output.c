#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <time.h>
#include <pixman.h>
#include <pixman-1/pixman.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/render/pass.h>
#include "../include/g-output.h"

struct g_output* g_output_create(
    struct wlr_output *wlr_output,
    struct wlr_allocator *allocator,
    struct wlr_renderer *renderer
) {
	wlr_output_init_render(wlr_output, allocator, renderer);

	struct wlr_output_state state;
	wlr_output_state_init(&state);
	wlr_output_state_set_enabled(&state, true);

	struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
	if (mode != NULL) {
		wlr_output_state_set_mode(&state, mode);
	}

	wlr_output_commit_state(wlr_output, &state);
	wlr_output_state_finish(&state);

    // Allocata memory
    struct g_output *output = calloc(1, sizeof(struct g_output));
	output->wlr_output = wlr_output;

    // Listeners
	output->frame_listener.notify = g_output_on_new_frame;
	wl_signal_add(&wlr_output->events.frame, &output->frame_listener);

    output->request_state_listener.notify = g_output_on_request_state;
    wl_signal_add(&wlr_output->events.frame, &output->request_state_listener);

	output->destroy_listener.notify = g_output_on_destroy;
	wl_signal_add(&wlr_output->events.destroy, &output->destroy_listener);

    return output;
}

void g_output_on_new_frame(struct wl_listener *listener, void *data) {
    struct g_output *output = wl_container_of(listener, output, frame_listener);

    struct wlr_output_state state;
    wlr_output_state_init(&state);

    struct wlr_render_pass *pass = wlr_output_begin_render_pass(output->wlr_output, &state, NULL);

    if (!pass) {
        wlr_output_state_finish(&state);
        return;
    }

    wlr_render_pass_submit(pass);

    pixman_region32_t damage;
    pixman_region32_init_rect(&damage, 0, 0, output->wlr_output->width, output->wlr_output->height);
    wlr_output_state_set_damage(&state, &damage);
    pixman_region32_fini(&damage);

    wlr_output_commit_state(output->wlr_output, &state);
    wlr_output_state_finish(&state);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
}

void g_output_on_request_state(struct wl_listener *listener, void *data) {
    struct g_output *output = wl_container_of(listener, output, request_state_listener);
	const struct wlr_output_event_request_state *event = data;
    
	wlr_output_commit_state(output->wlr_output, event->state);
}

void g_output_on_destroy(struct wl_listener *listener, void *data) {
    struct g_output *output = wl_container_of(listener, output, destroy_listener);

	wl_list_remove(&output->frame_listener.link);
    wl_list_remove(&output->request_state_listener.link);
	wl_list_remove(&output->destroy_listener.link);
	wl_list_remove(&output->link);
	free(output);
}
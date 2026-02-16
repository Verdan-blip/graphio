#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <wlr/util/log.h>

#include "../include/g_workspace_window.h"

static const double DECAY = 0.98;
static const double REWARD = 1.0;
static const double INITAL_SCORE = 1.0;
static const double WINDOW_LIFE_DURATION = 15;

static double calculate_decay(uint64_t now, uint64_t last_usage) {
    uint64_t dt = last_usage - now;
    return exp2(dt / WINDOW_LIFE_DURATION);
}

struct g_workspace_window* g_workspace_window_create(struct g_toplevel *toplevel) {
    if (toplevel == NULL) {
        wlr_log(WLR_ERROR, "g_workspace: got NULL toplevel, skipping workspace window creation");
    }

    struct g_workspace_window *workspace_window = malloc(sizeof(struct g_workspace_window));
    workspace_window->toplevel = toplevel;
    workspace_window->score = 0.0;
    workspace_window->last_usage = 0;

    return workspace_window;
}

void g_workspace_window_destroy(struct g_workspace_window* workspace_window) {
    workspace_window->toplevel = NULL;
    free(workspace_window);
}

void g_workspace_window_add_penalty(struct g_workspace_window* workspace_window, uint64_t now) {
    workspace_window->score += calculate_decay(now, workspace_window->last_usage);
}

void g_workspace_window_add_reward(struct g_workspace_window* workspace_window, uint64_t now) {
    workspace_window->score += REWARD;
    workspace_window->last_usage = now;
}

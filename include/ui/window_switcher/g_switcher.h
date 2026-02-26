#ifndef G_SWITCHER_H
#define G_SWITCHER_H

#include "include/g_navgraph.h"

struct g_server;

struct g_switcher {
    struct wlr_scene_tree *scene_tree;

    struct wlr_scene_rect *background;
    struct wlr_scene_rect *highlight;

    struct wlr_scene_node *window_nodes[G_NAVGRAPH_TOPS_COUNT];

    int selected_index;
    int current_window_count;
};

struct g_switcher* g_switcher_create(struct g_server *server);

void g_switcher_show(struct g_switcher *switcher);
void g_switcher_hide(struct g_switcher *switcher);

void g_switcher_set_selected(struct g_switcher *switcher, int index);
void g_switcher_add_toplevel(struct g_switcher *switcher, struct g_toplevel *toplevel);
void g_switcher_remove_toplevel(struct g_switcher *switcher, struct g_toplevel *toplevel);

void g_switcher_destroy(struct g_switcher *switcher);

#endif
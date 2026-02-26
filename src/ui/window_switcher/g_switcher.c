#include <stdlib.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "include/ui/window_switcher/g_switcher.h"
#include "include/g_navgraph.h"
#include "include/g_server.h"


struct g_switcher* g_switcher_create(struct g_server *server) {
    struct g_switcher *switcher = malloc(sizeof(struct g_switcher));

    switcher->scene_tree = wlr_scene_tree_create(&server->scene->tree);
    wlr_scene_node_raise_to_top(&switcher->scene_tree->node);

    float bg_color[4] = { 0.3f, 0.2f, 0.1f, 0.3f };
    float high_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    switcher->background = wlr_scene_rect_create(
        switcher->scene_tree, 800, 600, bg_color
    );

    switcher->highlight = wlr_scene_rect_create(
        switcher->scene_tree, 200, 150, high_color
    );

    switcher->selected_index = -1;
    switcher->current_window_count = 0;

    g_switcher_hide(switcher);

    return switcher;
}

void g_switcher_show(struct g_switcher *switcher) {
    wlr_scene_node_raise_to_top(&switcher->scene_tree->node);
    wlr_scene_node_set_enabled(&switcher->scene_tree->node, true);
}

void g_switcher_hide(struct g_switcher *switcher) {
    wlr_scene_node_set_enabled(&switcher->scene_tree->node, false);
}

void g_switcher_set_selected(struct g_switcher *switcher, int index) {
    if (index < 0 && index >= G_NAVGRAPH_TOPS_COUNT) return;

    switcher->selected_index = index;

    struct wlr_scene_node *selected_window_node = switcher->window_nodes[index];

    wlr_scene_node_set_position(
        &switcher->highlight->node, 
        selected_window_node->x, 
        selected_window_node->y
    );
}

void g_switcher_add_toplevel(struct g_switcher *switcher, struct g_toplevel *toplevel) {
    if (switcher->current_window_count >= G_NAVGRAPH_TOPS_COUNT) return;

    struct wlr_scene_surface *win_node = wlr_scene_surface_create(
        switcher->scene_tree, 
        toplevel->xdg_toplevel->base->surface
    );

    switcher->selected_index = switcher->current_window_count;
    switcher->current_window_count += 1;

    switcher->window_nodes[switcher->selected_index] = &win_node->buffer->node;

    struct wlr_scene_node *node = switcher->window_nodes[switcher->selected_index];
    
    wlr_scene_node_set_position(
    node, 
        50 + switcher->selected_index * 220, 
        100
    );
}

void g_switcher_remove_toplevel(struct g_switcher *switcher, struct g_toplevel *toplevel) {

}

void g_switcher_destroy(struct g_switcher *switcher);
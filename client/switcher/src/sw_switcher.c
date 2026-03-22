#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-client-protocol.h>
#include <wayland-util.h>

#include "include/sw_switcher.h"
#include "include/sw_toplevel.h"
#include "include/ui/sw_switcher_widget.h"

struct sw_switcher* sw_switcher_create() {
    struct sw_switcher *switcher = malloc(sizeof(struct sw_switcher));
    wl_list_init(&switcher->toplevels);

    for (int i = 0; i < PRIMATY_TOPLEVEL_COUNT; i++) {
        switcher->primary_toplevels[i] = NULL;
    }
    
    return switcher;
}

void sw_switcher_add_toplevel(
    struct sw_switcher *switcher, 
    struct sw_toplevel *toplevel
) {
    for (int i = 0; i < PRIMATY_TOPLEVEL_COUNT; i++) {
        if (switcher->primary_toplevels[i] == NULL) {
            switcher->primary_toplevels[i] = toplevel;
            return;
        }
    }

    wl_list_insert(&switcher->toplevels, &toplevel->link);
}

void sw_switcher_remove_toplevel(
    struct sw_switcher *switcher, 
    struct sw_toplevel *toplevel
) {
    for (int i = 0; i < PRIMATY_TOPLEVEL_COUNT; i++) {
        if (switcher->primary_toplevels[i] == toplevel) {
            switcher->primary_toplevels[i] = NULL;
            return;
        }
    }

    if (switcher->current_toplevel == toplevel) {
        switcher->current_toplevel = NULL;
    }

    wl_list_remove(&toplevel->link);
}

void sw_switcher_set_activated(
    struct sw_switcher *switcher, 
    struct sw_toplevel *toplevel
) {
    sw_toplevel_activate(toplevel);
}

void sw_switcher_set_primary_activated(struct sw_switcher *switcher, int index) {
    struct sw_toplevel *toplevel = switcher->primary_toplevels[index];

    if (toplevel == NULL) return;

    sw_switcher_set_activated(switcher, toplevel);
}

void sw_switcher_notify_toplevel_activated(
    struct sw_switcher *switcher, 
    struct sw_toplevel *toplevel
) {
    if (toplevel == NULL) return;

    toplevel->activated = true;

    for (int i = 0; i < PRIMATY_TOPLEVEL_COUNT; i++) {
        struct sw_toplevel *primary = switcher->primary_toplevels[i];

        if (primary == NULL) continue;
        if (primary == toplevel) continue;

        primary->activated = false;
    }

    struct sw_toplevel *slot_toplevel;
    wl_list_for_each(slot_toplevel, &toplevel->switcher->toplevels, link) {
        slot_toplevel->activated = false;
    }

    switcher->current_toplevel = toplevel;

    sw_switcher_widget_redraw(switcher->switcher_widget);
}

void sw_switcher_destroy(struct sw_switcher *switcher) {
    wl_list_remove(&switcher->toplevels);
    free(switcher);
}
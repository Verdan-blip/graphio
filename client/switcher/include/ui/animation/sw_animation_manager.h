#ifndef SW_ANIMATION_MANAGER_H
#define SW_ANIMATION_MANAGER_H

#include <wayland-util.h>
#include <stdint.h>

#include "include/ui/animation/sw_animation.h"

struct sw_animation_manager {
    struct wl_list queue;
    int64_t last_frame_time;
};

struct sw_animation_manager* sw_animation_manager_create();
void sw_animation_manager_add(struct sw_animation_manager *m, struct sw_animation *a);
void sw_animation_manager_cancel_for(struct sw_animation_manager *m, void *target);
void sw_animation_manager_update(struct sw_animation_manager *m, int64_t frame_time_usec);
void sw_animation_manager_destroy(struct sw_animation_manager *m);

#endif
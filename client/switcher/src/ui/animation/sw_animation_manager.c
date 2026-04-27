#include <stdlib.h>

#include "include/ui/animation/sw_animation_manager.h"

struct sw_animation_manager* sw_animation_manager_create() {
    struct sw_animation_manager *m = calloc(1, sizeof(struct sw_animation_manager));
    wl_list_init(&m->queue);
    return m;
}

void sw_animation_manager_add(struct sw_animation_manager *m, struct sw_animation *a) {
    sw_animation_manager_cancel_for(m, a->target);
    wl_list_insert(&m->queue, &a->link);
}

void sw_animation_manager_cancel_for(struct sw_animation_manager *m, void *target) {
    struct sw_animation *a, *tmp;
    wl_list_for_each_safe(a, tmp, &m->queue, link) {
        if (a->target == target) {
            wl_list_remove(&a->link);
            sw_animation_destroy(a);
        }
    }
}

void sw_animation_manager_update(struct sw_animation_manager *m, int64_t now) {
    if (m->last_frame_time == 0) {
        m->last_frame_time = now;
        return;
    }

    double dt = (now - m->last_frame_time) / 1000.0;
    m->last_frame_time = now;

    struct sw_animation *a, *tmp;
    wl_list_for_each_safe(a, tmp, &m->queue, link) {
        a->elapsed += dt;
        double t = a->elapsed / (double)a->duration;
        
        if (t >= 1.0) {
            a->update_cb(a, 1.0); // Финализируем
            wl_list_remove(&a->link);
            sw_animation_destroy(a);
        } else {
            a->update_cb(a, a->easing(t));
        }
    }
}
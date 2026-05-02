#include "include/ui/sw_border_item.h"
#include "include/math/sw_vec2.h"
#include <stdlib.h>

struct sw_border_item* sw_border_item_create() {
    struct sw_border_item *item = calloc(1, sizeof(struct sw_border_item));

    return item;
}

void sw_border_item_set_position(
    struct sw_border_item* item,
    struct sw_vec2 pos
) {
    item->pos = pos;
}

void sw_border_item_update(
    struct sw_border_item* item,
    struct sw_vec2 size,
    double corner_radius,
    double thickness
) {
    sw_border_item_set_size(item, size);
    sw_border_item_set_corner_radius(item, corner_radius);
    sw_border_item_set_thickness(item, thickness);
}

void sw_border_item_set_size(
    struct sw_border_item* item,
    struct sw_vec2 size
) {
    item->size = size;
}

void sw_border_item_set_corner_radius(
    struct sw_border_item* item,
    double corner_radius
) {
    item->corner_radius = corner_radius;
}

void sw_border_item_set_thickness(
    struct sw_border_item* item,
    double thickness
) {
    item->thickness = thickness;
}

void sw_border_item_set_data(
    struct sw_border_item* item,
    void *data
) {
    item->data = data;
}

void sw_border_item_destroy(struct sw_border_item* item) {
    free(item);
}
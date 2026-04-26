#include "include/ui/sw_border_item.h"
#include <stdlib.h>

struct sw_border_item* sw_border_item_create() {
    struct sw_border_item *item = calloc(1, sizeof(struct sw_border_item));

    return item;
}

void sw_border_item_resize(
    struct sw_border_item* item,
    double size,
    double corner_radius,
    double thickness
) {
    item->size = size;
    item->thickness = thickness;
    item->corner_radius = corner_radius;
}

void sw_border_item_set_position(
    struct sw_border_item* item,
    double x, double y
) {
    item->x = x;
    item->y = y;
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
#ifndef SW_BORDER_ITEM_H
#define SW_BORDER_ITEM_H

#include "include/math/sw_vec2.h"
#include <stdbool.h>

struct sw_border_item {
    struct sw_vec2 pos;
    struct sw_vec2 size;
    double corner_radius;
    double thickness;

    void *data;
};

struct sw_border_item* sw_border_item_create();

void sw_border_item_update(
    struct sw_border_item* item,
    struct sw_vec2 size,
    double corner_radius,
    double thickness
);

void sw_border_item_set_size(
    struct sw_border_item* item,
    struct sw_vec2 size
);

void sw_border_item_set_corner_radius(
    struct sw_border_item* item,
    double corner_radius
);

void sw_border_item_set_thickness(
    struct sw_border_item* item,
    double thickness
);

void sw_border_item_set_position(
    struct sw_border_item* item,
    struct sw_vec2 pos
);

void sw_border_item_set_data(
    struct sw_border_item* item,
    void *data
);

void sw_border_item_destroy(struct sw_border_item* item);

#endif
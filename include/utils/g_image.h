#ifndef G_IMAGE_H
#define G_IMAGE_H

#include <cairo/cairo.h>
#include <stddef.h>

struct g_image {
    int width, height;
    size_t stride;
    void *data;
};

struct g_image* g_load_image_from_path(const char* path);
void g_destroy_image(struct g_image *image);

#endif
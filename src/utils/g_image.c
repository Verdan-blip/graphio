#include <cairo/cairo.h>
#include <stdlib.h>

#include "include/utils/g_image.h"
#include "wlr/util/log.h"

struct g_image* g_load_image_from_path(const char* path) {
    cairo_surface_t *surface = cairo_image_surface_create_from_png(path);

    cairo_status_t status = cairo_surface_status(surface);
    if (status != CAIRO_STATUS_SUCCESS) {
        const char* status_string = cairo_status_to_string(status);
        wlr_log(WLR_ERROR, "failed to load cairo image from path: %s. Reason: %s", path, status_string);
        cairo_surface_destroy(surface);
        return NULL;
    }

    struct g_image *image = malloc(sizeof(struct g_image));
    image->width = cairo_image_surface_get_width(surface);
    image->height = cairo_image_surface_get_height(surface);
    image->stride = cairo_image_surface_get_height(surface);
    image->data = cairo_image_surface_get_data(surface);

    cairo_surface_destroy(surface);

    return image;
}

void g_destroy_image(struct g_image *image) {
    free(image->data);
    free(image);
}
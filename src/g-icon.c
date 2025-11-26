#include <stdint.h>
#include <stdlib.h>
#include <cairo/cairo.h>
#include <wlr/util/transform.h>
#include <wlr/util/log.h>
#include <wlr/interfaces/wlr_buffer.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/render/pass.h>

#include "../include/g-icon.h"
#include "../include/g-server.h"
#include <drm/drm_fourcc.h>

struct g_icon* g_icon_load_from_path(const char *path, struct wlr_renderer *renderer) {
    cairo_surface_t *surface = cairo_image_surface_create_from_png(path);

    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(surface);
        return NULL;
    }

    int width = cairo_image_surface_get_width(surface);
    int height = cairo_image_surface_get_height(surface);
    int stride = cairo_image_surface_get_stride(surface);

    unsigned char *data = cairo_image_surface_get_data(surface);
    
    struct wlr_texture *texture = wlr_texture_from_pixels(
        renderer, DRM_FORMAT_ARGB8888, stride, width, height, data
    );

    cairo_surface_destroy(surface);

    if (texture == NULL) {
        return NULL;
    }

    struct g_icon *icon = malloc(sizeof(struct g_icon));
    icon->width = width;
    icon->height = height;
    icon->texture = texture;

    return icon;
}

void g_icon_destroy(struct g_icon *icon) {
    if (icon->texture) {
        wlr_texture_destroy(icon->texture);
        icon->texture = NULL;
    }
    free(icon);
}

struct g_icon* g_icon_create_empty(int width, int height, struct wlr_renderer *renderer) {
    size_t data_size = width * height * 4;
    unsigned char *pixel_data = malloc(data_size);

    unsigned char red = 0;
    unsigned char green = 255;
    unsigned char blue = 0;
    unsigned char alpha = 255;

    for (int i = 0; i < width * height; i++) {
        pixel_data[i * 4 + 0] = red;     // R
        pixel_data[i * 4 + 1] = green;   // G
        pixel_data[i * 4 + 2] = blue;    // B
        pixel_data[i * 4 + 3] = alpha;   // A
    }

    struct wlr_texture *texture = wlr_texture_from_pixels(
        renderer, DRM_FORMAT_ARGB8888, width * 4, width, height, pixel_data
    );

    struct g_icon *icon = malloc(sizeof(struct g_icon));
    icon->width = width;
    icon->height = height;
    icon->texture = texture;

    return icon;
}

struct g_icon* g_icon_load_from_path_or_default(
    const char *path, 
    int width, 
    int height, 
    struct wlr_renderer *renderer
) {
    if (path == NULL) return g_icon_create_empty(width, height, renderer);

    struct g_icon *icon = g_icon_load_from_path(path, renderer);

    return icon == NULL ? g_icon_create_empty(width, height, renderer) : icon;
}

void g_icon_on_render_pass(
    struct g_icon *icon, 
    int pos_x, int pos_y,
    struct wlr_render_pass *pass
) {
    if (!icon->texture) {
        return;
    }

     const float alpha = 1.0f;

     struct wlr_render_texture_options opts = {
        .texture = icon->texture,
        .transform = WL_OUTPUT_TRANSFORM_NORMAL,
        .alpha = &alpha,
        .src_box = { 0, 0, icon->width, icon->height },
        .dst_box = { pos_x, pos_y, icon->width, icon->height},
    };

    wlr_render_pass_add_texture(pass, &opts);
}
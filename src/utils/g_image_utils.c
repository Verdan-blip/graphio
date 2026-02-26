#include <drm/drm_fourcc.h>
#include <stddef.h>
#include <stdint.h>
#include <wayland-server-core.h>
#include <wlr/render/drm_format_set.h>
#include <wlr/render/allocator.h>
#include <wlr/types/wlr_buffer.h>
#include <wlr/util/log.h>

#include "include/g_server.h"
#include "include/utils/g_image.h"
#include "include/utils/g_image_utils.h"

struct wlr_buffer* g_load_buffer_from_image(struct g_image *image, struct g_server *server) {
    uint64_t modifiers[] = { DRM_FORMAT_MOD_INVALID };

    struct wlr_drm_format format = (struct wlr_drm_format) {
        .format = WL_SHM_FORMAT_ARGB8888,
        .capacity = 1,
        .len = 1,
        .modifiers = modifiers
    };

    struct wlr_buffer *buffer = wlr_allocator_create_buffer(
        server->allocator, image->width, image->height, 
        &format
    );

    if (!buffer) {
        wlr_log(WLR_ERROR, "g_image: failed to load wlr_buffer from g_image");
        return NULL;
    }

    uint32_t buffer_format;
    size_t stride;
    void *data = NULL;
    if (!wlr_buffer_begin_data_ptr_access(
        buffer, WLR_BUFFER_DATA_PTR_ACCESS_WRITE,
        &data, &buffer_format, &stride)
    ) {
        wlr_buffer_drop(buffer);
        return NULL;
    }

    for (int y = 0; y < image->height; y++) {
        memcpy(
            (uint8_t*) data + y * stride,
            image->data + y * image->stride,
            image->width * 4
        );
    }

    wlr_buffer_end_data_ptr_access(buffer);

    return buffer;
}

struct wlr_buffer* g_load_buffer_from_image_path(const char *path, struct g_server *server) {
    struct g_image *image = g_load_image_from_path(path);

    if (!image) return NULL;

    return g_load_buffer_from_image(image, server);
}
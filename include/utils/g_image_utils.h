#ifndef G_IMAGE_UTILS_H
#define G_IMAGE_UTILS_H

struct g_server;
struct g_image;

struct wlr_buffer* g_load_buffer_from_image(struct g_image *image, struct g_server *server);
struct wlr_buffer* g_load_buffer_from_image_path(const char *path, struct g_server *server);

#endif
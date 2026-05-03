#ifndef SW_APP_STORAGE_H
#define SW_APP_STORAGE_H

#include "glib.h"
#include <stdbool.h>
#include <stdint.h>

struct sw_app_storage_data {
    double score;
    uint64_t last_update;
};

struct sw_app_storage {
    GHashTable *table;
    char *filepath;
};


struct sw_app_storage* sw_app_storage_load(const char *filename);
void sw_app_storage_destroy(struct sw_app_storage *storage);

const struct sw_app_storage_data* sw_app_storage_get(
    struct sw_app_storage *storage,
    const char *app_id
);

void sw_app_storage_set(
    struct sw_app_storage *storage,
    const char *app_id,
    const struct sw_app_storage_data* data
);

bool sw_app_storage_sync(struct sw_app_storage *storage);

#endif
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "include/storage/sw_app_storage.h"

static const char *SW_STORAGE_HEADER = "# sw-switcher-storage v1";

static void storage_data_free(gpointer data) {
    free(data);
}

struct sw_app_storage* sw_app_storage_load(const char *filename) {
    struct sw_app_storage *storage = malloc(sizeof(struct sw_app_storage));
    if (!storage) return NULL;

    storage->table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, storage_data_free);
    
    char *dir_path = g_build_filename(g_get_user_cache_dir(), "sw-switcher", NULL);
    storage->filepath = g_build_filename(dir_path, filename, NULL);
    
    g_mkdir_with_parents(dir_path, 0755);
    g_free(dir_path);

    FILE *f = fopen(storage->filepath, "r");
    if (!f) return storage;

    char line[1024];
    if (fgets(line, sizeof(line), f)) {
        if (strncmp(line, SW_STORAGE_HEADER, strlen(SW_STORAGE_HEADER)) != 0) {
            fclose(f);
            return storage;
        }
    }

    while (fgets(line, sizeof(line), f)) {
        char app_id[256];
        double score;
        uint64_t last_update;

        if (sscanf(line, "%255[^;];%lf;%lu", app_id, &score, &last_update) == 3) {
            struct sw_app_storage_data *data = malloc(sizeof(struct sw_app_storage_data));
            data->score = score;
            data->last_update = last_update;
            g_hash_table_insert(storage->table, g_strdup(app_id), data);
        }
    }

    fclose(f);
    return storage;
}

void sw_app_storage_destroy(struct sw_app_storage *storage) {
    if (!storage) return;
    g_hash_table_destroy(storage->table);
    g_free(storage->filepath);
    free(storage);
}

const struct sw_app_storage_data* sw_app_storage_get(
    struct sw_app_storage *storage,
    const char *app_id
) {
    if (!storage || !app_id) return NULL;
    return g_hash_table_lookup(storage->table, app_id);
}

void sw_app_storage_set(
    struct sw_app_storage *storage,
    const char *app_id,
    const struct sw_app_storage_data* data
) {
    if (!storage || !app_id || !data) return;

    struct sw_app_storage_data *copy = malloc(sizeof(struct sw_app_storage_data));
    memcpy(copy, data, sizeof(struct sw_app_storage_data));

    g_hash_table_insert(storage->table, g_strdup(app_id), copy);
}

bool sw_app_storage_sync(struct sw_app_storage *storage) {
    if (!storage || !storage->filepath) return false;

    char *tmp_path = g_strdup_printf("%s.tmp", storage->filepath);
    FILE *f = fopen(tmp_path, "w");
    if (!f) {
        g_free(tmp_path);
        return false;
    }

    fprintf(f, "%s\n", SW_STORAGE_HEADER);

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, storage->table);
    
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        char *app_id = (char *)key;
        struct sw_app_storage_data *data = (struct sw_app_storage_data *)value;
        fprintf(f, "%s;%.6f;%lu\n", app_id, data->score, data->last_update);
    }

    fflush(f);
    fsync(fileno(f));
    fclose(f);

    if (rename(tmp_path, storage->filepath) != 0) {
        perror("sw_app_storage_sync: rename failed");
        unlink(tmp_path);
        g_free(tmp_path);
        return false;
    }

    g_free(tmp_path);
    return true;
}
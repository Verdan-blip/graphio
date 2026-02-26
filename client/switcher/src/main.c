#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <wayland-client.h>

#include "../protocols/wlr-foreign-toplevel-management-unstable-v1-client-protocol.h"

struct toplevel {
    struct zwlr_foreign_toplevel_handle_v1 *handle;
    char *title;
    char *app_id;
    uint32_t *states;
    size_t states_count;
    uint32_t *outputs;
    size_t outputs_count;
    struct toplevel *parent;
    struct wl_list link;
    struct client_state *client_state;
};

struct client_state {
    struct wl_display *display;
    struct wl_registry *registry;
    struct zwlr_foreign_toplevel_manager_v1 *toplevel_manager;
    struct wl_list windows;
    int running;
};

static void handle_global(void *data, struct wl_registry *registry,
                         uint32_t name, const char *interface, uint32_t version);
static void handle_global_remove(void *data, struct wl_registry *registry,
                                uint32_t name);

static const struct wl_registry_listener registry_listener = {
    .global = handle_global,
    .global_remove = handle_global_remove,
};

static void toplevel_handle_title(void *data,
                                 struct zwlr_foreign_toplevel_handle_v1 *handle,
                                 const char *title) {
    struct toplevel *toplevel = data;
    
    if (toplevel->title) {
        free(toplevel->title);
    }
    toplevel->title = strdup(title);
    
    printf("Title changed: %s\n", title);
}

static void toplevel_handle_app_id(void *data,
                                  struct zwlr_foreign_toplevel_handle_v1 *handle,
                                  const char *app_id) {
    struct toplevel *toplevel = data;
    
    if (toplevel->app_id) {
        free(toplevel->app_id);
    }
    toplevel->app_id = strdup(app_id);
    
    printf("App ID changed: %s\n", app_id);
}

static void toplevel_handle_state(void *data,
                                 struct zwlr_foreign_toplevel_handle_v1 *handle,
                                 struct wl_array *state) {
    struct toplevel *toplevel = data;
    uint32_t *states = state->data;
    size_t count = state->size / sizeof(uint32_t);
    
    if (toplevel->states) {
        free(toplevel->states);
    }
    toplevel->states = malloc(state->size);
    memcpy(toplevel->states, states, state->size);
    toplevel->states_count = count;
    
    printf("State changed (count: %zu): ", count);
    for (size_t i = 0; i < count; i++) {
        printf("%d ", states[i]);
    }
    printf("\n");
}

static void toplevel_handle_output_enter(void *data,
                                       struct zwlr_foreign_toplevel_handle_v1 *handle,
                                       struct wl_output *output) {
    struct toplevel *toplevel = data;

    uint32_t output_id = (uint32_t)(uintptr_t)output;
    
    toplevel->outputs = realloc(toplevel->outputs,
                               (toplevel->outputs_count + 1) * sizeof(uint32_t));
    toplevel->outputs[toplevel->outputs_count++] = output_id;
    
    printf("Output enter: %u\n", output_id);
}

static void toplevel_handle_output_leave(void *data,
                                       struct zwlr_foreign_toplevel_handle_v1 *handle,
                                       struct wl_output *output) {
    struct toplevel *toplevel = data;
    uint32_t output_id = (uint32_t)(uintptr_t)output;
    
    for (size_t i = 0; i < toplevel->outputs_count; i++) {
        if (toplevel->outputs[i] == output_id) {
            memmove(&toplevel->outputs[i], &toplevel->outputs[i + 1],
                   (toplevel->outputs_count - i - 1) * sizeof(uint32_t));
            toplevel->outputs_count--;
            break;
        }
    }
    
    printf("Output leave: %u\n", output_id);
}

static void toplevel_handle_parent(void *data,
                                  struct zwlr_foreign_toplevel_handle_v1 *handle,
                                  struct zwlr_foreign_toplevel_handle_v1 *parent) {
    struct toplevel *toplevel = data;
    
    toplevel->parent = (struct toplevel *)parent;
    
    printf("Parent set\n");
}

static void toplevel_handle_done(void *data,
                                struct zwlr_foreign_toplevel_handle_v1 *handle) {
    struct toplevel *toplevel = data;
    printf("Toplevel done: [%s] %s\n",
           toplevel->app_id ? toplevel->app_id : "unknown",
           toplevel->title ? toplevel->title : "untitled");
}

static void toplevel_handle_closed(void *data,
                                  struct zwlr_foreign_toplevel_handle_v1 *handle) {
    struct toplevel *toplevel = data;
    
    printf("Toplevel closed: [%s] %s\n",
           toplevel->app_id ? toplevel->app_id : "unknown",
           toplevel->title ? toplevel->title : "untitled");
    
    wl_list_remove(&toplevel->link);
    
    // Освобождаем ресурсы
    if (toplevel->title) free(toplevel->title);
    if (toplevel->app_id) free(toplevel->app_id);
    if (toplevel->states) free(toplevel->states);
    if (toplevel->outputs) free(toplevel->outputs);
    free(toplevel);
}

// Интерфейс для toplevel_handle
static const struct zwlr_foreign_toplevel_handle_v1_listener toplevel_handle_listener = {
    .title = toplevel_handle_title,
    .app_id = toplevel_handle_app_id,
    .state = toplevel_handle_state,
    .output_enter = toplevel_handle_output_enter,
    .output_leave = toplevel_handle_output_leave,
    .parent = toplevel_handle_parent,
    .done = toplevel_handle_done,
    .closed = toplevel_handle_closed,
};

// Обработчик для toplevel_manager
static void handle_toplevel(void *data,
                          struct zwlr_foreign_toplevel_manager_v1 *manager,
                          struct zwlr_foreign_toplevel_handle_v1 *handle) {
    struct client_state *state = data;
    
    printf("New toplevel detected\n");
    
    struct toplevel *toplevel = calloc(1, sizeof(struct toplevel));
    toplevel->handle = handle;
    toplevel->parent = NULL;
    toplevel->client_state = state;  // ← сохраняем указатель на состояние
    
    wl_list_insert(&state->windows, &toplevel->link);
    
    zwlr_foreign_toplevel_handle_v1_add_listener(handle,
                                                &toplevel_handle_listener,
                                                toplevel);
}

static void handle_finished(void *data,
                          struct zwlr_foreign_toplevel_manager_v1 *manager) {
    struct client_state *state = data;
    printf("Toplevel manager finished\n");
    state->running = 0;
}

// Интерфейс для toplevel_manager
static const struct zwlr_foreign_toplevel_manager_v1_listener toplevel_manager_listener = {
    .toplevel = handle_toplevel,
    .finished = handle_finished,
};

// Обработчик глобальных интерфейсов
static void handle_global(void *data, struct wl_registry *registry,
                         uint32_t name, const char *interface, uint32_t version) {
    struct client_state *state = data;
    
    printf("Global interface: %s, version: %d\n", interface, version);
    
    if (strcmp(interface, "zwlr_foreign_toplevel_manager_v1") == 0) {
        // Создаем менеджер toplevel
        state->toplevel_manager = wl_registry_bind(registry, name,
                                                  &zwlr_foreign_toplevel_manager_v1_interface,
                                                  3);
        
        // Устанавливаем слушатель для менеджера
        zwlr_foreign_toplevel_manager_v1_add_listener(state->toplevel_manager,
                                                      &toplevel_manager_listener,
                                                      state);
    }
}

static void handle_global_remove(void *data, struct wl_registry *registry,
                                uint32_t name) {
    // В простом примере ничего не делаем
}

// Функция для печати всех окон
void print_all_windows(struct client_state *state) {
    struct toplevel *toplevel;
    int count = 0;
    
    printf("\n=== Current windows ===\n");
    wl_list_for_each(toplevel, &state->windows, link) {
        printf("%d: [%s] %s\n", ++count,
               toplevel->app_id ? toplevel->app_id : "unknown",
               toplevel->title ? toplevel->title : "untitled");
    }
    printf("=======================\n\n");
}

// Обработчик сигнала для корректного завершения
static int signal_fd;
static void signal_handler(int signum) {
    write(signal_fd, &signum, sizeof(signum));
}

int main(int argc, char *argv[]) {
    struct client_state state = {0};
    struct wl_display *display;
    struct wl_event_queue *queue;
    
    // Инициализируем список окон
    wl_list_init(&state.windows);
    state.running = 1;
    
    // Подключаемся к Wayland-сокету
    display = wl_display_connect(NULL);
    if (!display) {
        fprintf(stderr, "Failed to connect to Wayland display\n");
        return 1;
    }
    state.display = display;
    printf("Connected to Wayland display\n");
    
    // Получаем реестр глобальных объектов
    state.registry = wl_display_get_registry(display);
    if (!state.registry) {
        fprintf(stderr, "Failed to get registry\n");
        wl_display_disconnect(display);
        return 1;
    }
    
    // Устанавливаем слушатель для реестра
    wl_registry_add_listener(state.registry, &registry_listener, &state);
    
    // Выполняем roundtrip для получения всех глобальных интерфейсов
    wl_display_roundtrip(display);
    
    // Проверяем, поддерживается ли протокол
    if (!state.toplevel_manager) {
        fprintf(stderr, "Compositor doesn't support wlr_foreign_toplevel_manager_v1\n");
        wl_registry_destroy(state.registry);
        wl_display_disconnect(display);
        return 1;
    }
    
    // Выполняем еще один roundtrip для получения начальных окон
    wl_display_roundtrip(display);
    
    printf("Listening for toplevel events... Press Ctrl+C to exit.\n");
    
    // Настраиваем обработку сигналов
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    
    // Главный цикл обработки событий
    while (state.running && wl_display_dispatch(display) != -1) {
        // Каждые 10 событий печатаем список окон
        static int counter = 0;
        if (++counter % 10 == 0) {
            print_all_windows(&state);
        }
    }
    
    printf("\nShutting down...\n");
    
    // Очистка ресурсов
    if (state.toplevel_manager) {
        zwlr_foreign_toplevel_manager_v1_destroy(state.toplevel_manager);
    }
    if (state.registry) {
        wl_registry_destroy(state.registry);
    }
    if (state.display) {
        wl_display_disconnect(state.display);
    }
    
    return 0;
}
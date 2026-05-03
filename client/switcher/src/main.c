#include "include/app/sw_application.h"

int main(int argc, char **argv) {
    struct sw_application *app = sw_application_create(&argc, &argv);
    if (!app) return 1;

    int status = sw_application_run(app);

    sw_application_destroy(app);
    return status;
}
#include <wlr/util/log.h>
#include "../include/g-server.h"

int main(int argc, char** argv) {
	wlr_log_init(WLR_DEBUG, NULL);

	struct g_server *server = g_server_create();
	if (!server) {
		wlr_log(WLR_ERROR, "Failed to create Graphio Server");
		return -1;
	}

	g_server_run(server);
	g_server_destroy(server);

	return 0;
};

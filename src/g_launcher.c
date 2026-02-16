#include <unistd.h>
#include "../include/g_launcher.h"

int g_launch(const char* exec_path) {
    int pid = fork();

    if (pid == 0) {
		execl("/bin/sh", "/bin/sh", "-c", exec_path, (void *)NULL);
	}

    return pid;
}
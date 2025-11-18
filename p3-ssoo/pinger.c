#include <stdio.h>
#include <unistd.h>
#include <unistd.h>
#include <err.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

enum TipoSalida {
	SALIDA_PING = 0,
	SALIDA_ERR_PING = 1,
	SALIDA_FAIL = 2
};

int
main(int argc, char *argv[])
{
	int pid;
	int sts;

	if ((argc - 1) == 0) {
		fprintf(stderr, "usage: pinger ip/name [ip/name ...]\n");
		exit(SALIDA_FAIL);
	}

	for (int i = 1; i < argc; i++) {
		switch (fork()) {
		case -1:
			err(SALIDA_FAIL, "fork failed!");
			break;
		case 0:
			execl("/bin/ping", "ping", "-w", "5", "-i", "6",
			      argv[i], NULL);
			err(SALIDA_FAIL, "execl failed!");
			break;
		default:
			break;
		}
	}
	int all_success = 1;

	while ((pid = wait(&sts)) != -1) {
		if (!WIFEXITED(sts) || WEXITSTATUS(sts) != 0) {
			all_success = 0;
		}
	}

	if (all_success) {
		exit(SALIDA_PING);
	} else {
		exit(SALIDA_ERR_PING);
	}
}

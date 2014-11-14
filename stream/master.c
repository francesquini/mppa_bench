#include <stdio.h>
#include <assert.h>
#include <mppaipc.h>
#include <mppa/osconfig.h>

int
main(int argc, char **argv) 
{
	int status;
	int pid;
	int i, j;
	int nb_clusters, nb_threads;

	pid = mppa_spawn(0, NULL, "stream_slave", NULL, NULL);
	assert(pid >= 0);

	status = 0;
	if ((status = mppa_waitpid(pid, &status, 0)) < 0) {
		printf("[I/O] Waitpid on cluster %d failed.\n", pid);
		mppa_exit(status);
	}

	mppa_exit(0);
}

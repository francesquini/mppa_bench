#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sched.h>
#include <unistd.h>

#include "interface_mppa.h"
#include "common.h"

#define ARGC_SLAVE 4

void 
spawn_slaves(const char slave_bin_name[], int nb_clusters, int nb_threads) 
{
	int i;
	int cluster_id;
	int pid;

	// Prepare arguments to send to slaves
	char **argv_slave = (char**) malloc(sizeof (char*) * ARGC_SLAVE);
	for (i = 0; i < ARGC_SLAVE - 1; i++)
		argv_slave[i] = (char*) malloc (sizeof (char) * 10);

	sprintf(argv_slave[0], "%d", nb_clusters);
	sprintf(argv_slave[1], "%d", nb_threads);
	argv_slave[3] = NULL;

	// Spawn slave processes
	for (cluster_id = 0; cluster_id < nb_clusters; cluster_id++) {
		sprintf(argv_slave[2], "%d", cluster_id);
		pid = mppa_spawn(cluster_id, NULL, slave_bin_name, (const char **)argv_slave, NULL);
		assert(pid >= 0);
		LOG("Spawned Cluster %d\n", cluster_id);
	}

	// Free arguments
	for (i = 0; i < ARGC_SLAVE; i++)
		free(argv_slave[i]);
	free(argv_slave);
}

int
main(int argc, char **argv) 
{
	int status;
	int pid;
	int i, j;
	int nb_clusters;
	char path[256];
	uint64_t start_time, exec_time;

	nb_clusters = atoi(argv[1]);

	char *comm_buffer = (char *) malloc(MAX_BUFFER_SIZE * nb_clusters);
	assert(comm_buffer != NULL);
	fill_buffer(comm_buffer, MAX_BUFFER_SIZE * nb_clusters, 0);

	LOG("Number of clusters: %d\n", nb_clusters);
	
	mppa_init_time();

	// Spawn slave processes
	spawn_slaves("slave", nb_clusters, 1);

	// Initialize global barrier
	barrier_t *global_barrier = mppa_create_master_barrier(BARRIER_SYNC_MASTER, BARRIER_SYNC_SLAVE, nb_clusters);

	// ===========================================================================================================

	portal_t ***write_portals = (portal_t ***) malloc (sizeof(portal_t **) * MAX_PORTALS);
	for(i = 0; i < MAX_PORTALS; i++) {
		
		write_portals[i] = (portal_t **) malloc (sizeof(portal_t *) * nb_clusters);
		
		for (j = 0; j < nb_clusters; j++) {
			sprintf(path, "/mppa/portal/%d:%d", j, 4 + (i * nb_clusters) + j);
			write_portals[i][j] = mppa_create_write_portal(path); //, comm_buffer, MAX_BUFFER_SIZE);
		}
	}

	printf ("type;exec;direction;nb_clusters;size;time\n");

	mppa_barrier_wait(global_barrier);

	int nb_exec;
	int size;
	for (nb_exec = 1; nb_exec <= NB_EXEC; nb_exec++) {

		// ----------- MASTER -> SLAVE ---------------	
		for (size = 1; size <= MAX_BUFFER_SIZE; size *= 2) {
			mppa_barrier_wait(global_barrier);

			start_time = mppa_get_time();
			
			// post asynchronous writes
			for(i = 0; i < MAX_PORTALS; i++)
				for (j = 0; j < nb_clusters; j++)
					mppa_async_write_portal(write_portals[i][j], comm_buffer, size, i * MAX_BUFFER_SIZE);

			// block until all asynchronous writes have finished
			for(i = 0; i < MAX_PORTALS; i++)
				for (j = 0; j < nb_clusters; j++)
					mppa_async_write_wait_portal(write_portals[i][j]);

			exec_time = mppa_diff_time(start_time, mppa_get_time());
			printf("n-portals-per-cluster;%d;%s;%d;%d;%llu\n", nb_exec, "master-slave", nb_clusters, size, exec_time);
		}
	}

	mppa_barrier_wait(global_barrier);

	for(i = 0; i < MAX_PORTALS; i++)
		for (j = 0; j < nb_clusters; j++)
			mppa_close_portal(write_portals[i][j]);

	// ===========================================================================================================

	// Initialize communication portals to send messages to clusters (one portal per cluster)
	portal_t **write_portals_2 = (portal_t **) malloc (sizeof(portal_t *) * nb_clusters);
	for (i = 0; i < nb_clusters; i++) {
		sprintf(path, "/mppa/portal/%d:%d", i, 4 + i);
		write_portals_2[i] = mppa_create_write_portal(path);
	}

	for (nb_exec = 1; nb_exec <= NB_EXEC; nb_exec++) {
		// ----------- MASTER -> SLAVE ---------------	
		for (size = 1; size <= MAX_BUFFER_SIZE; size *= 2) {
			mppa_barrier_wait(global_barrier);

			start_time = mppa_get_time();
			
			// post asynchronous writes
			for(i = 0; i < MAX_PORTALS; i++) {
				for (j = 0; j < nb_clusters; j++)
					mppa_async_write_portal(write_portals_2[j], comm_buffer, size, i * MAX_BUFFER_SIZE);

				// block until all asynchronous writes have finished
				for (j = 0; j < nb_clusters; j++)
					mppa_async_write_wait_portal(write_portals_2[j]);
			}

			exec_time = mppa_diff_time(start_time, mppa_get_time());
			printf("1-portal-per-cluster;%d;%s;%d;%d;%llu\n", nb_exec, "master-slave", nb_clusters, size, exec_time);
		}
	}

	mppa_barrier_wait(global_barrier);

	for (i = 0; i < nb_clusters; i++)
		mppa_close_portal(write_portals_2[i]);

	// ===========================================================================================================

	LOG("MASTER: waiting clusters to finish\n");

	// Wait for all slave processes to finish
	for (pid = 0; pid < nb_clusters; pid++) {
		status = 0;
		if ((status = mppa_waitpid(pid, &status, 0)) < 0) {
			printf("[I/O] Waitpid on cluster %d failed.\n", pid);
			mppa_exit(status);
		}
	}

	LOG("MASTER: clusters finished\n");

	// mppa_close_barrier(global_barrier);

	// for(i = 0; i < MAX_PORTALS; i++)
	// 	for (j = 0; j < nb_clusters; j++)
	// 		mppa_close_portal(write_portals[i][j]);

	mppa_exit(0);

	return 0;
}

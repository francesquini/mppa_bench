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

#ifdef USE_PORTAL
#ifdef USE_CHANNEL
	assert (0);
#endif
#endif	

	nb_clusters = atoi(argv[1]);

#ifdef USE_PORTAL
	char *comm_buffer = (char *) malloc(MAX_BUFFER_SIZE * nb_clusters);
	assert(comm_buffer != NULL);
	init_buffer(comm_buffer, MAX_BUFFER_SIZE * nb_clusters);
#endif
#ifdef USE_CHANNEL
	char *comm_buffer = (char *) malloc(MAX_BUFFER_SIZE);
	assert(comm_buffer != NULL);
	init_buffer(comm_buffer, MAX_BUFFER_SIZE);
#endif

	LOG("Number of clusters: %d\n", nb_clusters);
	
	mppa_init_time();

	// Spawn slave processes
	spawn_slaves("slave", nb_clusters, 1);

	// Initialize global barrier
	barrier_t *global_barrier = mppa_create_master_barrier(BARRIER_SYNC_MASTER, BARRIER_SYNC_SLAVE, nb_clusters);

#ifdef USE_PORTAL
	// Initialize communication portal to receive messages from clusters
	portal_t *read_portal = mppa_create_read_portal("/mppa/portal/128:3", comm_buffer, MAX_BUFFER_SIZE * nb_clusters, nb_clusters, NULL);

	// Initialize communication portals to send messages to clusters (one portal per cluster)
	portal_t **write_portals = (portal_t **) malloc (sizeof(portal_t *) * nb_clusters);
	for (i = 0; i < nb_clusters; i++) {
		sprintf(path, "/mppa/portal/%d:%d", i, 4 + i);
		write_portals[i] = mppa_create_write_portal(path, comm_buffer, MAX_BUFFER_SIZE);
	}
#endif	

#ifdef USE_CHANNEL
	int base_tag;

	channel_t **write_channels = (channel_t **) malloc (sizeof(channel_t *) * nb_clusters);
	
	base_tag = 3;
	for (i = 0; i < nb_clusters; i++) {
		sprintf(path, "/mppa/channel/%d:%d/%d:%d", i, base_tag + i, 128, (base_tag + i) + nb_clusters);
		write_channels[i] = mppa_create_write_channel(path);
	}

	channel_t **read_channels = (channel_t **) malloc (sizeof(channel_t *) * nb_clusters);
	
	base_tag = 3 + (2 * nb_clusters);
	for (i = 0; i < nb_clusters; i++) {
		sprintf(path, "/mppa/channel/%d:%d/%d:%d", 128, base_tag + i, i, (base_tag + i) + nb_clusters);
		read_channels[i] = mppa_create_read_channel(path);
	}
#endif	

	printf ("type;exec;direction;nb_clusters;size;time\n");

	mppa_barrier_wait(global_barrier);

	int nb_exec;
	for (nb_exec = 1; nb_exec <= NB_EXEC; nb_exec++) {

#ifdef USE_PORTAL
		// ----------- MASTER -> SLAVE ---------------	
		for (i = 1; i <= MAX_BUFFER_SIZE; i *= 2) {
			mppa_barrier_wait(global_barrier);

			start_time = mppa_get_time();
			
			// post asynchronous writes
			for (j = 0; j < nb_clusters; j++)
				mppa_async_write_portal(write_portals[j], comm_buffer, i, 0);

			// block until all asynchronous writes have finished
			for (j = 0; j < nb_clusters; j++)
				mppa_async_write_wait_portal(write_portals[j]);

			exec_time = mppa_diff_time(start_time, mppa_get_time());
			printf("portal;%d;%s;%d;%d;%llu\n", nb_exec, "master-slave", nb_clusters, i, exec_time);
		}

		// ----------- SLAVE -> MASTER ---------------	
		for (i = 1; i <= MAX_BUFFER_SIZE; i *= 2) {
			mppa_barrier_wait(global_barrier);

			start_time = mppa_get_time();

			// Block until receive the asynchronous write FROM ALL CLUSTERS and prepare for next asynchronous writes
			// This is possible because we set the trigger = nb_clusters, so the IO waits for nb_cluster messages
			mppa_async_read_wait_portal(read_portal);
			
			exec_time = mppa_diff_time(start_time, mppa_get_time());
			printf ("portal;%d;%s;%d;%d;%llu\n", nb_exec, "slave-master", nb_clusters, i, exec_time);
		}
#endif


#ifdef USE_CHANNEL
		// ----------- MASTER -> SLAVE ---------------
		for (i = 1; i <= MAX_BUFFER_SIZE; i *= 2) {				
			start_time = mppa_get_time();

			for (j = 0; j < nb_clusters; j++)
				mppa_write_channel(write_channels[j], comm_buffer, i);
			
			exec_time = mppa_diff_time(start_time, mppa_get_time());
			printf ("channel;%d;%s;%d;%d;%llu\n", nb_exec, "master-slave", nb_clusters, i, exec_time);
		}		
		
		// ----------- SLAVE -> MASTER ---------------	
		for (i = 1; i <= MAX_BUFFER_SIZE; i *= 2) {
			start_time = mppa_get_time();

			for (j = 0; j < nb_clusters; j++)
				mppa_read_channel(read_channels[j], comm_buffer, i);

			exec_time = mppa_diff_time(start_time, mppa_get_time());
			printf ("channel;%d;%s;%d;%d;%llu\n", nb_exec, "slave-master", nb_clusters, i, exec_time);
		}
#endif
	}

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

	mppa_close_barrier(global_barrier);

#ifdef USE_PORTAL
	mppa_close_portal(read_portal);
	for (i = 0; i < nb_clusters; i++)
		mppa_close_portal(write_portals[i]);
#endif	

#ifdef USE_CHANNEL
	for (i = 0; i < nb_clusters; i++) {
		mppa_close_channel(write_channels[i]);
		mppa_close_channel(read_channels[i]);
	}
#endif	

	mppa_exit(0);

	return 0;
}

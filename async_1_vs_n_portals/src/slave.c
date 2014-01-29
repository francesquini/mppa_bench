#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "interface_mppa.h"


int main(int argc,char **argv) {
	char path[25];
	int i;

	// Global data
	char *comm_buffer = (char *) malloc(MAX_BUFFER_SIZE * MAX_PORTALS);
	assert(comm_buffer != NULL);

  	// Set initial parameters
	int nb_clusters = atoi(argv[0]);
	// int nb_threads  = atoi(argv[1]);
	int cluster_id  = atoi(argv[2]);

	int nb_exec;
	int size;

	// Initialize global barrier
	barrier_t *global_barrier = mppa_create_slave_barrier (BARRIER_SYNC_MASTER, BARRIER_SYNC_SLAVE);

	// ===========================================================================================================	

	portal_t **read_portals = (portal_t **) malloc (sizeof(portal_t *) * MAX_PORTALS);

	for(i = 0; i < MAX_PORTALS; i++) {
		sprintf(path, "/mppa/portal/%d:%d", cluster_id, 4 + (i * nb_clusters) + cluster_id);
		read_portals[i] = mppa_create_read_portal(path, comm_buffer, MAX_BUFFER_SIZE * MAX_PORTALS, 1, NULL);
	}

	mppa_barrier_wait(global_barrier);

	LOG("Slave %d started\n", cluster_id);

	init_buffer(comm_buffer, MAX_BUFFER_SIZE * MAX_PORTALS);

	for (nb_exec = 1; nb_exec <= NB_EXEC; nb_exec++) {

		// ----------- MASTER -> SLAVE ---------------
		for (size = 1; size <= MAX_BUFFER_SIZE; size *= 2) {
			mppa_barrier_wait(global_barrier);

			// Block until receive the asynchronous write and prepare for next asynchronous writes		
			for(i = 0; i < MAX_PORTALS; i++)
				mppa_async_read_wait_portal(read_portals[i]);
			
			// print_buffer(comm_buffer, MAX_BUFFER_SIZE * MAX_PORTALS);
			// init_buffer(comm_buffer, MAX_BUFFER_SIZE * MAX_PORTALS);
		}
	}

	mppa_barrier_wait(global_barrier);

	for(i = 0; i < MAX_PORTALS; i++)
		mppa_close_portal(read_portals[i]);

	// ===========================================================================================================	

	// Initialize communication portal to receive messages from IO-node
	sprintf(path, "/mppa/portal/%d:%d", cluster_id, 4 + cluster_id);
	portal_t *read_portal = mppa_create_read_portal(path, comm_buffer, MAX_BUFFER_SIZE * MAX_PORTALS, MAX_PORTALS, NULL);

	init_buffer(comm_buffer, MAX_BUFFER_SIZE * MAX_PORTALS);

	for (nb_exec = 1; nb_exec <= NB_EXEC; nb_exec++) {
		// ----------- MASTER -> SLAVE ---------------
		for (size = 1; size <= MAX_BUFFER_SIZE; size *= 2) {
			mppa_barrier_wait(global_barrier);

			// Block until receive the asynchronous write and prepare for next asynchronous writes		
			mppa_async_read_wait_portal(read_portal);

			// print_buffer(comm_buffer, MAX_BUFFER_SIZE * MAX_PORTALS);
			// init_buffer(comm_buffer, MAX_BUFFER_SIZE * MAX_PORTALS);
		}
	}

	mppa_barrier_wait(global_barrier);

	mppa_close_portal(read_portal);

	// ===========================================================================================================	

	// mppa_close_barrier(global_barrier);

	// for(i = 0; i < MAX_PORTALS; i++)
	// 	mppa_close_portal(read_portals[i]);

	LOG("Slave %d finished\n", cluster_id);

	mppa_exit(0);

	return 0;
}

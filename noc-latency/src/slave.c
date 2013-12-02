#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "interface_mppa.h"


int main(int argc,char **argv) {
	char path[25];
	int i;

	// Global data
	char *comm_buffer = (char *) malloc(MAX_BUFFER_SIZE);
	assert(comm_buffer != NULL);

	for(i = 0; i < MAX_BUFFER_SIZE; i++)
		comm_buffer[i] = 0;

  	// Set initial parameters
	// int nb_clusters = atoi(argv[0]);
	// int nb_threads  = atoi(argv[1]);
	int cluster_id  = atoi(argv[2]);

#ifdef USE_PORTAL
	// Initialize global barrier
	barrier_t *global_barrier = mppa_create_slave_barrier (BARRIER_SYNC_MASTER, BARRIER_SYNC_SLAVE);
	
	// Initialize communication portals
	portal_t *write_portal = mppa_create_write_portal("/mppa/portal/128:2", comm_buffer, MAX_BUFFER_SIZE);	
	
	// Initialize communication portal to receive messages from IO-node
	sprintf(path, "/mppa/portal/%d:%d", cluster_id, 3 + cluster_id);
	portal_t *read_portal = mppa_create_read_portal(path, comm_buffer, MAX_BUFFER_SIZE, 1, NULL);
#endif	

#ifdef USE_CHANNEL
	sprintf(path, "/mppa/channel/%d:%d/%d:%d", cluster_id, cluster_id + 1, 128, cluster_id + 17);
	channel_t *read_channel = mppa_create_read_channel(path);
	sprintf(path, "/mppa/channel/%d:%d/%d:%d", 128, cluster_id + 34, cluster_id, cluster_id + 50);
	channel_t *write_channel = mppa_create_write_channel(path);
#endif

#ifdef USE_PORTAL
	mppa_barrier_wait(global_barrier);
#endif
	LOG("Slave %d started\n", cluster_id);

	int nb_exec;
	for (nb_exec = 1; nb_exec <= NB_EXEC; nb_exec++) {

#ifdef USE_PORTAL
		// ----------- MASTER -> SLAVE ---------------
		for (i = 1; i <= MAX_BUFFER_SIZE; i *= 2) {
			mppa_barrier_wait(global_barrier);

			// Block until receive the asynchronous write and prepare for next asynchronous writes		
			mppa_async_read_wait_portal(read_portal);
		}

		// ----------- SLAVE -> MASTER ---------------
		for (i = 1; i <= MAX_BUFFER_SIZE; i *= 2) {
			mppa_barrier_wait(global_barrier);
			
			// post asynchronous write
			mppa_async_write_portal(write_portal, comm_buffer, i, cluster_id * MAX_BUFFER_SIZE);
			
			// wait for the end of the transfer
			mppa_async_write_wait_portal(write_portal);
		}
#endif

#ifdef USE_CHANNEL
		// ----------- MASTER -> SLAVE ---------------
		for (i = 1; i <= MAX_BUFFER_SIZE; i *= 2)
			mppa_read_channel(read_channel, comm_buffer, i);
		
		// ----------- SLAVE -> MASTER ---------------
		for (i = 1; i <= MAX_BUFFER_SIZE; i *= 2)
			mppa_write_channel(write_channel, comm_buffer, i);
#endif			
	}

#ifdef USE_PORTAL
	mppa_close_barrier(global_barrier);
	mppa_close_portal(write_portal);
	mppa_close_portal(read_portal);
#endif

#ifdef USE_CHANNEL
	mppa_close_channel(read_channel);
	mppa_close_channel(write_channel);
#endif

	LOG("Slave %d finished\n", cluster_id);

	mppa_exit(0);

	return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "interface_mppa.h"


int main(int argc,char **argv) {
  char path[25];
  int i;
  int nb_exec;
  int cluster;

  // Global data
  char *comm_buffer = (char *) malloc(BUFFER_SIZE);
  assert(comm_buffer != NULL);
  
  for(i = 0; i < BUFFER_SIZE; i++)
    comm_buffer[i] = 0;
  
  // Set initial parameters
  int nb_clusters = atoi(argv[0]);
  // int nb_threads  = atoi(argv[1]);
  int cluster_id  = atoi(argv[2]);
  
  // Initialize global barrier
  barrier_t *global_barrier = mppa_create_slave_barrier (BARRIER_SYNC_MASTER, BARRIER_SYNC_SLAVE);
  
  // Initialize communication portals
  sprintf(path, "/mppa/portal/%d:3", 128 + (cluster_id % 4));
  portal_t *write_portal = mppa_create_write_portal(path, comm_buffer, BUFFER_SIZE, 128 + (cluster_id % 4));
  
  // Initialize communication portal to receive messages from IO-node
  // The read_portal is configure so it expects a single message before unblocking
  sprintf(path, "/mppa/portal/%d:%d", cluster_id, 4 + cluster_id);
  portal_t *read_portal = mppa_create_read_portal(path, comm_buffer, BUFFER_SIZE, 1, NULL);
  
  LOG("Slave %d started\n", cluster_id);
  
  for (nb_exec = 1; nb_exec <= NB_EXEC; nb_exec++) {
    for (cluster = 0; cluster < nb_clusters; cluster++) {
      mppa_barrier_wait(global_barrier);
      
      // ping: master ->slave
      // Block until receive the asynchronous write and prepare for next asynchronous writes
      // The cluster will wait for a single message before unblocking
      if(cluster_id == cluster)
	mppa_async_read_wait_portal(read_portal);
      
      // pong: slave -> master
      mppa_barrier_wait(global_barrier);
      
      // post asynchronous write
      if(cluster_id == cluster) {
	mppa_async_write_portal(write_portal, comm_buffer, i, cluster_id * BUFFER_SIZE);
	// wait for the end of the transfer
	mppa_async_write_wait_portal(write_portal);
      }
    }
  }
  
  mppa_close_barrier(global_barrier);
  
  mppa_close_portal(write_portal);
  mppa_close_portal(read_portal);
  
  LOG("Slave %d finished\n", cluster_id);
  
  mppa_exit(0);
  
  return 0;
}

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
  int i;
  int nb_clusters;
  char path[256];
  uint64_t start_time, exec_time_master_slave, exec_time_slave_master;
  int nb_exec, cluster;
  
  nb_clusters = 16;
  
  char *comm_buffer = (char *) malloc(BUFFER_SIZE * nb_clusters);
  assert(comm_buffer != NULL);
  init_buffer(comm_buffer, BUFFER_SIZE * nb_clusters);
  
  LOG("Number of clusters: %d\n", nb_clusters);
  
  mppa_init_time();
  
  // Spawn slave processes, 1 thread per slave
  spawn_slaves("slave", nb_clusters, 1);
  
  // Initialize global barrier
  barrier_t *global_barrier = mppa_create_master_barrier(BARRIER_SYNC_MASTER, BARRIER_SYNC_SLAVE, nb_clusters);
  
  // Initialize communication portal to receive messages from clusters
  int number_dmas = nb_clusters < 4 ? nb_clusters : 4;
  portal_t **read_portals = (portal_t **) malloc (sizeof(portal_t *) * number_dmas);
  
  // One message is expected on the DMA to unblock the mppa_async_read_wait_portal function
  for (i = 0; i < number_dmas; i++) {
    sprintf(path, "/mppa/portal/%d:3", 128 + i);
    read_portals[i] = mppa_create_read_portal(path, comm_buffer, BUFFER_SIZE * nb_clusters, 1, NULL);
  }
  
  // Initialize communication portals to send messages to clusters (one portal per cluster)
  portal_t **write_portals = (portal_t **) malloc (sizeof(portal_t *) * nb_clusters);
  for (i = 0; i < nb_clusters; i++) {
    sprintf(path, "/mppa/portal/%d:%d", i, 4 + i);
    write_portals[i] = mppa_create_write_portal(path, comm_buffer, BUFFER_SIZE, i);
  }
  
  printf ("type;exec;size;slave;master-slave;slave-master;latency\n");
  
  for (nb_exec = 1; nb_exec <= NB_EXEC; nb_exec++) {  
    for (cluster = 0; cluster < nb_clusters; cluster++) {
      mppa_barrier_wait(global_barrier);
    
      // ping: master -> slave 
      start_time = mppa_get_time();
      
      mppa_async_write_portal(write_portals[cluster], comm_buffer, BUFFER_SIZE, 0);
      mppa_async_write_wait_portal(write_portals[cluster]);
      
      exec_time_master_slave = mppa_diff_time(start_time, mppa_get_time());
      
      // pong: slave -> master
      mppa_barrier_wait(global_barrier);
      
      start_time = mppa_get_time();

      // receive on the correct DMA: the slave will send to the correct DMA automatically through mppa_async_write_portal()
      mppa_async_read_wait_portal(read_portals[cluster % 4]);
      exec_time_slave_master = mppa_diff_time(start_time, mppa_get_time());
      
      printf("portal;%d;%d;%d;%llu;%llu;%.2f\n", nb_exec, BUFFER_SIZE, cluster, exec_time_master_slave, exec_time_slave_master, 1.0*(exec_time_master_slave + exec_time_slave_master)/2);
    }
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
  
  for (i = 0; i < number_dmas; i++)
    mppa_close_portal(read_portals[i]);
  
  for (i = 0; i < nb_clusters; i++)
    mppa_close_portal(write_portals[i]);
  
  mppa_exit(0);
  
  return 0;
}

#include <mppa/osconfig.h>
#include "interface_mppa.h"

void set_path_name(char *path, char *template_path, int rx, int tag) {
	sprintf(path, template_path, rx, tag);
}

/**************************************
 * PORTAL COMMUNICATION
 **************************************/

portal_t *mppa_create_read_portal (char *path, void* buffer, unsigned long buffer_size, int trigger, void (*function)(mppa_sigval_t)) {
	portal_t *ret = (portal_t*) malloc (sizeof(portal_t));
	int status;
	ret->file_descriptor = mppa_open(path, O_RDONLY);
	assert(ret->file_descriptor != -1);
	
	mppa_aiocb_t tmp = MPPA_AIOCB_INITIALIZER(ret->file_descriptor, buffer, buffer_size);
	memcpy(&ret->portal, &tmp, sizeof(mppa_aiocb_t));
	
	if (trigger > -1) {
		mppa_aiocb_set_trigger(&ret->portal, trigger);
	}

	// Attention: we can't use callbacks with trigger/mppa_aio_wait (bug?)
	if (function)
		mppa_aiocb_set_callback(&ret->portal, function);
	
	status = mppa_aio_read(&ret->portal);
	assert(status == 0);

	return ret;
}

portal_t *mppa_create_write_portal (char *path) {	
	portal_t *ret = (portal_t*) malloc (sizeof(portal_t));
	ret->file_descriptor = mppa_open(path, O_WRONLY);
	assert(ret->file_descriptor != -1);

   	return ret;
}

int mppa_get_trigger_portal(portal_t *portal) {
	return mppa_aiocb_trigger(&portal->portal);
}

void mppa_aio_wait_portal(portal_t *portal) {
	int status;
	// status = mppa_aio_wait(&portal->portal);
	status = mppa_aio_rearm(&portal->portal);
	assert(status != -1);
}

void mppa_close_portal (portal_t *portal) {
	assert(mppa_close (portal->file_descriptor) != -1);
	free (portal);
}

void mppa_write_portal (portal_t *portal, void *buffer, int buffer_size, int offset) {
	// printf("Cluster %d will write.\n", __k1_get_cluster_id());
	int status;
	status = mppa_pwrite(portal->file_descriptor, buffer, buffer_size, offset);
	assert(status == buffer_size);
}

/**************************************
 * CHANNEL COMMUNICATION
 **************************************/


channel_t *mppa_create_read_channel (char *path) {
	channel_t *ret = (channel_t*) malloc (sizeof(channel_t));
	ret->file_descriptor = mppa_open(path, O_RDONLY);
	assert(ret->file_descriptor != -1);

   	return ret;
}

channel_t *mppa_create_write_channel (char *path) {
	channel_t *ret = (channel_t*) malloc (sizeof(channel_t));
	ret->file_descriptor = mppa_open(path, O_WRONLY);
	assert(ret->file_descriptor != -1);

   	return ret;
}

void mppa_write_channel (channel_t *channel, void *buffer, int buffer_size) {
	int status= mppa_write(channel->file_descriptor, buffer, buffer_size);
	assert(status == buffer_size);
}

void mppa_read_channel (channel_t *channel, void *buffer, int buffer_size) {
	int status= mppa_read(channel->file_descriptor, buffer, buffer_size);
	assert(status == buffer_size);
}

void mppa_close_channel (channel_t *channel) {
	assert(mppa_close(channel->file_descriptor) != -1);
	free (channel);	
}


/**************************************
 * BARRIER - INTERNAL
 **************************************/

barrier_t *mppa_create_master_barrier (char *path_master, char *path_slave, int clusters) {
	int status, i;
	int ranks[clusters];
	long long match;

	barrier_t *ret = (barrier_t*) malloc (sizeof (barrier_t));

	ret->sync_fd_master = mppa_open(path_master, O_RDONLY);
	assert(ret->sync_fd_master != -1);
	
	ret->sync_fd_slave = mppa_open(path_slave, O_WRONLY);
	assert(ret->sync_fd_slave != -1);

	match = -1 << clusters;
	status = mppa_ioctl(ret->sync_fd_master, MPPA_RX_SET_MATCH, match);
	assert(status == 0);
	
	for (i = 0; i < clusters; i++)
		ranks[i] = i;

	status = mppa_ioctl(ret->sync_fd_slave, MPPA_TX_SET_RX_RANKS, clusters, ranks);
	assert(status == 0);

	ret->mode = BARRIER_MASTER;

	return ret;
}

barrier_t *mppa_create_slave_barrier (char *path_master, char *path_slave) {
	int status;

	barrier_t *ret = (barrier_t*) malloc (sizeof (barrier_t));

	ret->sync_fd_master = mppa_open(path_master, O_WRONLY);
	assert(ret->sync_fd_master != -1);

	ret->sync_fd_slave = mppa_open(path_slave, O_RDONLY);
	assert(ret->sync_fd_slave != -1);

	status = mppa_ioctl(ret->sync_fd_slave, MPPA_RX_SET_MATCH, (long long) 0);
	assert(status == 0);

	ret->mode = BARRIER_SLAVE;

	return ret;
}

void mppa_barrier_wait(barrier_t *barrier) {
	int status;
	long long dummy;

	if(barrier->mode == BARRIER_MASTER) {
		dummy = -1;
		status = mppa_read(barrier->sync_fd_master, &barrier->match, sizeof(long long));
		assert(status == sizeof(long long));

		status = mppa_write(barrier->sync_fd_slave, &dummy, sizeof(long long));
		assert(status == sizeof(long long));
	}
	else {
		long long mask = (long long) 1 << __k1_get_cluster_id();
		dummy = 0;
		
		status = mppa_write(barrier->sync_fd_master, &mask, sizeof(long long));
		assert(status == sizeof(long long));

		status = mppa_read(barrier->sync_fd_slave, &dummy, sizeof(long long));
		assert(status == sizeof(long long));
	}
}

void mppa_close_barrier (barrier_t *barrier) {
	assert(mppa_close(barrier->sync_fd_master) != -1);
	assert(mppa_close(barrier->sync_fd_slave) != -1);
	free(barrier);
}


/**************************************
 * TIME
 **************************************/

static uint64_t residual_error = 0;

void mppa_init_time(void) {
	uint64_t t1, t2;
	t1 = mppa_get_time();
	t2 = mppa_get_time();
	residual_error = t2 - t1;
}

inline uint64_t mppa_get_time(void) {
	return __k1_io_read64((void *)0x70084040) / MPPA_FREQUENCY;
}

inline uint64_t mppa_diff_time(uint64_t t1, uint64_t t2) {
	return t2 - t1 - residual_error;
}

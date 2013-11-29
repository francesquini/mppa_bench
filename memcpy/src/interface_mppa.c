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
	
	mppa_aiocb_ctor(&ret->aiocb, ret->file_descriptor, buffer, buffer_size);
	
	if (trigger > -1) {
		mppa_aiocb_set_trigger(&ret->aiocb, trigger);
	}

	// Attention: we can't use callbacks with trigger/mppa_aio_wait (bug?)
	if (function)
		mppa_aiocb_set_callback(&ret->aiocb, function);
	
	status = mppa_aio_read(&ret->aiocb);
	assert(status == 0);

	return ret;
}

portal_t *mppa_create_write_portal (char *path, void* buffer, unsigned long buffer_size) {	
	portal_t *ret = (portal_t*) malloc (sizeof(portal_t));
	ret->file_descriptor = mppa_open(path, O_WRONLY);
	assert(ret->file_descriptor != -1);

	// we need to initialize an aiocb for asynchronous writes.
	// it seems that the buffer and buffer size parameters are not important here,
	// because we're going to specify them with mppa_aiocb_set_pwrite()
	// before calling mppa_aio_write()
	assert(mppa_aiocb_ctor(&ret->aiocb, ret->file_descriptor, buffer, buffer_size) == &ret->aiocb);

   	return ret;
}

void mppa_async_read_wait_portal(portal_t *portal) {
	int status;
	status = mppa_aio_rearm(&portal->aiocb);
	assert(status != -1);
}

void mppa_async_write_wait_portal(portal_t *portal) {
	int status;

	while(mppa_aio_error(&portal->aiocb) == EINPROGRESS);

	status = mppa_aio_return(&portal->aiocb);
	assert(status != -1);
}

void mppa_close_portal (portal_t *portal) {
	assert(mppa_close(portal->file_descriptor) != -1);
	free (portal);
}

void mppa_write_portal (portal_t *portal, void *buffer, int buffer_size, int offset) {
	int status;
	status = mppa_pwrite(portal->file_descriptor, buffer, buffer_size, offset);
	assert(status == buffer_size);
}

void mppa_async_write_portal (portal_t *portal, void *buffer, int buffer_size, int offset) {
	int status;
	mppa_aiocb_set_pwrite(&portal->aiocb, buffer, buffer_size, offset);

	while ((status = mppa_aio_write(&portal->aiocb)) == -EAGAIN);
	assert(status == 0);
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
 * BARRIER
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

	match = (long long) - (1 << clusters);
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
		long long match;

		// printf("Master: barrier wait\n");
		status = mppa_read(barrier->sync_fd_master, &match, sizeof(match));
		assert(status == sizeof(match));
		// printf("Master: barrier end\n");
		
		status = mppa_write(barrier->sync_fd_slave, &dummy, sizeof(long long));
		assert(status == sizeof(long long));
	}
	else {
		dummy = 0;
		long long mask;

		mask = 0;
		mask |= 1 << __k1_get_cluster_id();

		// printf("Slave: barrier wait\n");
		status = mppa_write(barrier->sync_fd_master, &mask, sizeof(mask));
		assert(status == sizeof(mask));
		// printf("Slave: barrier end\n");
		
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
 * RQUEUE COMMUNICATION
 **************************************/

rqueue_t *mppa_create_read_rqueue (int message_size, int rx_id, int rx_tag, char *tx_ids, int tx_tag) {
	rqueue_t *ret = (rqueue_t *) malloc (sizeof(rqueue_t));
	char path_name[256];
	
	sprintf(path_name, "/mppa/queue.%d/%d:%d/%s:%d", message_size, rx_id, rx_tag, tx_ids, tx_tag);
	ret->file_descriptor = mppa_open(path_name, O_RDONLY);
	assert(ret->file_descriptor != -1);

	return ret;
}

void mppa_init_read_rqueue(rqueue_t *rqueue, int credit) {
	int status = mppa_ioctl(rqueue->file_descriptor, MPPA_RX_SET_CREDITS, credit);
	assert(status == 0);
}

rqueue_t *mppa_create_write_rqueue (int message_size, int rx_id, int rx_tag, char *tx_ids, int tx_tag) {
	rqueue_t *ret = (rqueue_t *) malloc (sizeof(rqueue_t));
	char path_name[256];
	
	sprintf(path_name, "/mppa/queue.%d/%d:%d/%s:%d", message_size, rx_id, rx_tag, tx_ids, tx_tag);
	ret->file_descriptor = mppa_open(path_name, O_WRONLY);
	assert(ret->file_descriptor != -1);

	return ret;
}

void mppa_read_rqueue (rqueue_t *rqueue, void *buffer, int buffer_size) {
	int status;
	status = mppa_read(rqueue->file_descriptor, buffer, buffer_size);
	assert(status == buffer_size);
}

void mppa_write_rqueue (rqueue_t *rqueue, void *buffer, int buffer_size) {
	int status;
	status = mppa_write(rqueue->file_descriptor, buffer, buffer_size);
	assert(status == buffer_size);
}

void mppa_close_rqueue(rqueue_t *rqueue) {
	assert(mppa_close (rqueue->file_descriptor) != -1);
	free (rqueue);
}

/**************************************
 * TIME
 **************************************/

static uint64_t residual_error = 0;

void init_time(void) {
	uint64_t t1, t2;
	t1 = get_time();
	t2 = get_time();
	residual_error = t2 - t1;
}

inline uint64_t get_time(void) {
	return __k1_io_read64((void *)0x70084040) / MPPA_FREQUENCY;
}

inline uint64_t diff_time(uint64_t t1, uint64_t t2) {
	return t2 - t1 - residual_error;
}

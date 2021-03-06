#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "defs_mppa.h"

void 
init_buffer(char *buffer, int size)
{	
	int i;
	for(i = 0; i < size; i++)
		buffer[i] = 0;
}


void my_memcpy(void *to_buffer, void *from_buffer, int size) {
	portal_t *read_portal = mppa_create_read_portal("/mppa/portal/128:2", to_buffer, size, 1, NULL);
	portal_t *write_portal = mppa_create_write_portal("/mppa/portal/128:2", from_buffer, size);
	// post asynchronous write                                          
	mppa_async_write_portal(write_portal, from_buffer, size, 0);
	// wait for the end of the transfer                                 
	mppa_async_write_wait_portal(write_portal);
	// prepare for next read                                            
	mppa_async_read_wait_portal(read_portal);
	mppa_close_portal(read_portal);
	mppa_close_portal(write_portal);
}

int
main(int argc, char **argv) 
{
	int nb_runs;
	int size;
	int base = KB;

	INIT_TIMER();

	char *from_buffer = (char *) malloc(sizeof(char) * MAX_BUFFER_SIZE);
	char *to_buffer = (char *) malloc(sizeof(char) * MAX_BUFFER_SIZE);
	assert(from_buffer != NULL && to_buffer != NULL);

	init_buffer(from_buffer, MAX_BUFFER_SIZE);

	// basic memory copy using memcpy
	for (nb_runs = 1; nb_runs <= NUMBER_RUNS; nb_runs++) {
		for (size = 1 * KB; size <= MAX_BUFFER_SIZE; size += 64 * KB) {
			START_TIMER();
			memcpy(to_buffer, from_buffer, size);
			STOP_TIMER();
			PRINT_TIMER(nb_runs, "mppa-io-memcpy", size/base);
		}
	}

	portal_t *read_portal = mppa_create_read_portal("/mppa/portal/128:250", to_buffer, MAX_BUFFER_SIZE, 1, NULL);
	portal_t *write_portal = mppa_create_write_portal("/mppa/portal/128:250", from_buffer, MAX_BUFFER_SIZE);	

	// memory copy using portals: we open portals and reuse them for all memory copies
	for (nb_runs = 1; nb_runs <= NUMBER_RUNS; nb_runs++) {
		for (size = 1 * KB; size <= MAX_BUFFER_SIZE; size += 64 * KB) {
			START_TIMER();
			
			// post asynchronous write
			mppa_async_write_portal(write_portal, from_buffer, size, 0);
			
			// wait for the end of the transfer
			mppa_async_write_wait_portal(write_portal);			
			
			STOP_TIMER();
			PRINT_TIMER(nb_runs, "mppa-io-portal", size/base);

			// prepare for next read
			mppa_async_read_wait_portal(read_portal);
		}
	}

	mppa_close_portal(read_portal);
	mppa_close_portal(write_portal);

	// my_memcpy: open/close portals each time we perform a memcpy
	for (nb_runs = 1; nb_runs <= NUMBER_RUNS; nb_runs++) {
		for (size = 1 * KB; size <= MAX_BUFFER_SIZE; size += 64 * KB) {
			START_TIMER();
			my_memcpy(to_buffer, from_buffer, size);
			STOP_TIMER();
			PRINT_TIMER(nb_runs, "mppa-io-my_memcpy", size/base);
		}
	}

	FINALIZE();
	
	return 0;
}

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "defs_x86.h"

void 
init_buffer(char *buffer, int size)
{	
	int i;
	for(i = 0; i < size; i++)
		buffer[i] = 0;
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
			PRINT_TIMER(nb_runs, "x86", size/base);
		}
	}

	FINALIZE();
	
	return 0;
}

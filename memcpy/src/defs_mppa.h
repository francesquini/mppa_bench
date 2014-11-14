#include <inttypes.h>

#define KB 1024
#define MB 1024 * KB
#define MAX_BUFFER_SIZE 2048 * KB

#include "interface_mppa.h"
#define NUMBER_RUNS 5

#define FINALIZE()							mppa_exit(0);

// Timer helpers
#define INIT_TIMER()  						uint64_t start_time, exec_time; init_time(); printf("exec;processor;size;time\n");
#define START_TIMER()						start_time = get_time();
#define STOP_TIMER()						exec_time = diff_time(start_time, get_time());
#define PRINT_TIMER(nb_exec, type, info)	printf("%d;%s;%d;%llu\n", nb_exec, type, info, (long long unsigned int) exec_time);

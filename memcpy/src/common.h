#include <inttypes.h>

#define KB 1024
#define MB 1024 * KB
#define MAX_BUFFER_SIZE 4 * MB

#ifdef MPPA
	#include "interface_mppa.h"

	#define NUMBER_RUNS 5

	#define FINALIZE()							mppa_exit(0);

#else //x86
	#include <sys/time.h>

	#define NUMBER_RUNS 30

	// Timer functions
	static uint64_t residual_error = 0; // residual error to calibrate timers

	inline uint64_t 
	get_time(void) 
	{
		struct timeval t1;
		gettimeofday(&t1, NULL);
		uint64_t ret = UINT64_C(1000000) * ((uint64_t) t1.tv_sec);
		ret += (uint64_t) t1.tv_usec;
		return ret;
	}

	void 
	init_time(void) 
	{
		uint64_t t1, t2;
		t1 = get_time();
		t2 = get_time();
		residual_error = t2 - t1;
	}

	inline uint64_t 
	diff_time(uint64_t t1, uint64_t t2) 
	{
		return t2 - t1 - residual_error;
	}

	#define FINALIZE()	

#endif

// Timer helpers
#define INIT_TIMER()  						uint64_t start_time, exec_time; init_time(); printf("exec;processor;size;time\n");
#define START_TIMER()						start_time = get_time();
#define STOP_TIMER()						exec_time = diff_time(start_time, get_time());
#define PRINT_TIMER(nb_exec, type, info)	printf("%d;%s;%d;%llu\n", nb_exec, type, info, (long long unsigned int) exec_time);

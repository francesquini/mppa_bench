#include <inttypes.h>

#define KB 1024
#define MB 1024 * KB
#define MAX_BUFFER_SIZE 16 * MB

#define NUMBER_RUNS 1

// residual error to calibrate timers
static uint64_t residual_error = 0;

#ifdef MPPA
	#include <mppa/osconfig.h>
	#include <mppaipc.h>

	#define MPPA_FREQUENCY 400

	// Timer functions
	inline uint64_t 
	get_time(void) 
	{
		return __k1_io_read64((void *)0x70084040) / MPPA_FREQUENCY;
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

	// Other specific functions
	#define PRINT_TIMER(nb_exec, info)	printf("%d;mppa-io;%d;%llu\n", nb_exec, info, (long long unsigned int) exec_time);
	#define FINALIZE()					mppa_exit(0);

#else //x86
	#include <sys/time.h>

	// Timer functions
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

	// Other specific functions
	#define PRINT_TIMER(nb_exec, info)	printf("%d;x86;%d;%llu\n", nb_exec, info, (long long unsigned int) exec_time);
	#define FINALIZE()	
#endif

// Timer helpers
#define INIT_TIMER()  				uint64_t start_time, exec_time; init_time(); printf("exec;processor;size;time\n");
#define START_TIMER()				start_time = get_time();
#define STOP_TIMER()				exec_time = diff_time(start_time, get_time());




#define KB 1024
#define MB 1024 * KB

#define MAX_BUFFER_SIZE 1 * MB
#define NB_EXEC 1

// Select PORTAL or CHANNEL
#define USE_PORTAL
// #define USE_CHANNEL

#ifdef DEBUG
#define LOG(...) printf(__VA_ARGS__); fflush(stdout)
#else
#define LOG(...) 
#endif //DEBUG

typedef struct {
	int cluster_id;
} rqueue_msg_t;


void init_buffer(char *buffer, int size);
void fill_buffer(char *buffer, int size, int cluster_id);
void print_buffer(char *buffer, int size);

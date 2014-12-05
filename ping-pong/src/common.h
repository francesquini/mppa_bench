#define USE_PORTAL 1

#define KB 1024
#define MB 1024 * KB

#define BUFFER_SIZE 4 * KB
#define NB_EXEC 30

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

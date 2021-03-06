
#define KB 1024
#define MB 1024 * KB

#define MAX_BUFFER_SIZE 64 * KB
// #define MAX_BUFFER_SIZE 64
#define NB_EXEC 1

#define MAX_PORTALS 15

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

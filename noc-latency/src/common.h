
#define KB 1024
#define MB 1024 * KB

#define MAX_BUFFER_SIZE 1 * MB
// #define MAX_BUFFER_SIZE 64
#define NB_EXEC 1

//
#define USE_PORTAL
// #define USE_CHANNEL

#ifdef DEBUG
#define LOG(...) printf(__VA_ARGS__); fflush(stdout)
#else
#define LOG(...) 
#endif //DEBUG
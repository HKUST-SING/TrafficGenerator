#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdbool.h>

/* structure of flow metadata */
struct flow_metadata
{
    unsigned int id;    /* ID */
    unsigned int size;  /* flow size (bytes) */
    unsigned int tos;   /* ToS value */
    unsigned int rate;  /* sending rate (Mbps) */
};

/* flow meata data size */
#define TG_METADATA_SIZE (sizeof(struct flow_metadata))
/* default server port */
#define TG_SERVER_PORT 5001
/* default number of backlogged connections for listen() */
#define TG_SERVER_BACKLOG_CONN SOMAXCONN
/* maximum number of bytes to write in a 'send' system call */
#define TG_MAX_WRITE (1 << 20)
/* minimum number of bytes to write in a 'send' system call (used with rate limiting) */
#define TG_MIN_WRITE (1 << 16)
/* maximum amount of data to read in a 'recv' system call */
#define TG_MAX_READ (1 << 20)
/* default initial number of TCP connections per pair */
#define TG_PAIR_INIT_CONN 5
/* default goodput / link capacity ratio */
#define TG_GOODPUT_RATIO (1448.0 / (1500 + 14 + 4 + 8 + 12))

#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
    #define min(a,b) ((a) < (b) ? (a) : (b))
#endif

/*
 * I borrow read_exact() and write_exact() from https://github.com/datacenter/empirical-traffic-gen.
 * Thanks Mohammad Alizadeh. I really learn a lot from your code and papers!
 */

/* read exactly 'count' bytes from a socket 'fd' */
unsigned int read_exact(int fd, char *buf, size_t count, size_t max_per_read, bool dummy_buf);

/* write exactly 'count' bytes into a socket 'fd' */
unsigned int write_exact(int fd, char *buf, size_t count, size_t max_per_write,
    unsigned int rate_mbps, unsigned int tos, unsigned int sleep_overhead_us, bool dummy_buf);

/* read the metadata of a flow from a socket and return true if it succeeds. */
bool read_flow_metadata(int fd, struct flow_metadata *f);

/* write a flow request into a socket and return true if it succeeds */
bool write_flow_req(int fd, struct flow_metadata *f);

/* write a flow (response) into a socket and return true if it succeeds */
bool write_flow(int fd, struct flow_metadata *f, unsigned int sleep_overhead_us);

/* print error information and terminate the program */
void error(char *msg);

/* remove '\r' and '\n' from a string */
void remove_newline(char *str);

/* generate poission process arrival interval */
double poission_gen_interval(double avg_rate);

/* calculate usleep overhead */
unsigned int get_usleep_overhead(int iter_num);

/* randomly generate a value based on weights */
unsigned int gen_value_weight(unsigned int *vals, unsigned int *weights, unsigned int len, unsigned int weight_total);

/* display progress */
void display_progress(unsigned int num_finished, unsigned int num_total);

#endif

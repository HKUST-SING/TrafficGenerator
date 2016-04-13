#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdbool.h>

/* Default server port */
#define TG_SERVER_PORT 5001
/* Default number of backlogged connections for listen() */
#define TG_SERVER_BACKLOG_CONN SOMAXCONN
/* Maximum amount of data to write in a 'send' system call */
#define TG_MAX_WRITE 1048576 //1MB
/* Minimum amount of data to write in a 'send' system call (used with rate limiting) */
#define TG_MIN_WRITE 65536   //64KB
/* Maximum amount of data to read in a 'recv' system call */
#define TG_MAX_READ 1048576 //1MB
/* Default initial number of TCP connections per pair */
#define TG_PAIR_INIT_CONN 5
/* Default goodput / link capacity ratio */
#define TG_GOODPUT_RATIO (1448.0 / (1500 + 14 + 4 + 8 + 12))


/* I borrow following three functions from https://github.com/datacenter/empirical-traffic-gen. Thanks Mohammod! */
unsigned int read_exact(int fd, char *buf, size_t count,
    size_t max_per_read, bool dummy_buf);

unsigned int write_exact(int fd, char *buf, size_t count, size_t max_per_write,
    unsigned int rate_mbps, unsigned int tos, unsigned int usleep_overhead_us, bool dummy_buf);

/* Print error information and terminate the program */
void error(char *msg);

/* Remove \r \n from a string */
void remove_newline(char *str);

/* Generate poission process arrival interval */
double poission_gen_interval(double avg_rate);

/* Calculate usleep overhead */
unsigned int get_usleep_overhead(int iter_num);

/* Randomly generate a value based on weights */
unsigned int gen_value_weight(unsigned int *values, unsigned int *weights, unsigned int len, unsigned int weight_total);

#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "common.h"

/*
 * This function attemps to read exactly count bytes from file descriptor fd
 * into buffer starting at buf. It repeatedly calls read() until either:
 * 1. count bytes have been read
 * 2. end of file is reached, or for a network socket, the connection is closed
 * 3. read() produces an error
 * Each internal call to read() is for at most max_per_read bytes. The return
 * value gives the number of bytes successfully read.
 * The dummy_buf flag can be set by the caller to indicate that the contents
 * of buf are irrelevant. In this case, all read() calls put their data at
 * location starting at buf, overwriting previous reads.
 * To avoid buffer overflow, the length of buf should be at least count when
 * dummy_buf = false, and at least min{count, max_per_read} when
 * dummy_buf = true.
 */
unsigned int read_exact(int fd, char *buf, size_t count,
    size_t max_per_read, bool dummy_buf)
{
    unsigned int bytes_total_read = 0;  //total number of bytes that have been read
    unsigned int bytes_to_read = 0; //maximum number of bytes to read in next read() call
    char *cur_buf = NULL;   //current location
    int n;  //number of bytes read in current read() call

    while (count > 0)
    {
        bytes_to_read = (count > max_per_read) ? max_per_read : count;
        cur_buf = (dummy_buf) ? buf : (buf + bytes_total_read);
        n = read(fd, cur_buf, bytes_to_read);

        if (n <= 0)
        {
            if (n < 0)
                perror("ERROR: read in read_exact()");
            break;
        }
        else
        {
            bytes_total_read += n;
            count -= n;
        }
    }

    return bytes_total_read;
}


/*
 * This function attemps to write exactly count bytes from the buffer starting
 * at buf to file referred to by file descriptor fd. It repeatedly calls
 * write() until either:
 * 1. count bytes have been written
 * 2. write() produces an error
 * Each internal call to write() is for at most max_per_write bytes. The return
 * value gives the number of bytes successfully written.
 * The dummy_buf flag can be set by the caller to indicate that the contents
 * of buf are irrelevant. In this case, all write() calls get their data from
 * starting location buf.
 * To avoid buffer overflow, the length of buf should be at least count when
 * dummy_buf = false, and at least min{count, max_per_write} when
 * dummy_buf = true.
 * Users can rate-limit the sending of traffic. If rate_mbps is equal to 0, it indicates no rate-limiting.
 * Users can also set ToS value for traffic.
 */
unsigned int write_exact(int fd, char *buf, size_t count,
    size_t max_per_write, unsigned int rate_mbps, unsigned int tos, bool dummy_buf)
{
    unsigned int bytes_total_write = 0;  //total number of bytes that have been written
    unsigned int bytes_to_write = 0; //maximum number of bytes to write in next send() call
    char *cur_buf = NULL;   //current location
    int n;  //number of bytes read in current read() call
    struct timeval tv_start, tv_end;    //start and end time of write
    unsigned int sleep_us = 0;  //sleep time (us)
    unsigned int write_us = 0;  //time used for write()

    if (setsockopt(fd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)) < 0)
    {
        perror("Error: set IP_TOS option");
        return 0;
    }

    while (count > 0)
    {
        bytes_to_write = (count > max_per_write) ? max_per_write : count;
        cur_buf = (dummy_buf) ? buf : (buf + bytes_total_write);
        gettimeofday(&tv_start, NULL);
        n = write(fd, cur_buf, bytes_to_write);
        gettimeofday(&tv_end, NULL);
        write_us = (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + (tv_end.tv_usec - tv_start.tv_usec);
        sleep_us = (rate_mbps) ? n * 8 / rate_mbps : 0;

        if (n <= 0)
        {
            if (n < 0)
                perror("ERROR: write in write_exact()");
            break;
        }
        else
        {
            bytes_total_write += n;
            count -= n;
            if (write_us < sleep_us)
                usleep(sleep_us - write_us);
        }
    }

    return bytes_total_write;
}

/* Print error information */
void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

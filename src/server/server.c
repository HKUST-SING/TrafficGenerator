#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <pthread.h>

#include "../common/common.h"

int servPort = TG_SERVER_PORT;
unsigned int sleep_overhead_us = 50;
int debug_mode = 0;
char flow_max_buf[TG_MAX_WRITE] = {1};
char flow_min_buf[TG_MIN_WRITE] = {1};

/* Print usage of the program */
void print_usage(char *program);
/* Read command line arguments */
void read_args(int argc, char *argv[]);
/* Handle an incomming connection */
void* handle_connection(void* ptr);
/* Get usleep overhead in microsecond (us) */
unsigned int get_sleep_overhead(int iter_num);

int main(int argc, char *argv[])
{
    int listen_fd;
    struct sockaddr_in serv_addr; //local server address
    struct sockaddr_in cli_addr; //remote client address
    int sock_opt = 1;
    pthread_t serv_thread;   //server thread
	int* sockfd_ptr = NULL;
    socklen_t len = sizeof(struct sockaddr_in);

    /* Read arguments */
    read_args(argc, argv);

    /* Calculate usleep overhead */
    sleep_overhead_us = get_usleep_overhead(10);
    if (debug_mode)
        printf("usleep() overhead is around %u us\n", sleep_overhead_us);

    /* Initialize local server address */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;    //IP protocol
    serv_addr.sin_addr.s_addr = INADDR_ANY;    //0.0.0.0
    serv_addr.sin_port = htons(servPort);

    /* Initialize server socket */
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        error("Error: initialize socket");

    /* Set socket options */
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt)) < 0)
        error("Error: set SO_REUSEADDR option");
    if (setsockopt(listen_fd, IPPROTO_TCP, TCP_NODELAY, &sock_opt, sizeof(sock_opt)) < 0)
        error("ERROR: set TCP_NODELAY option");

    if (bind(listen_fd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr)) < 0)
        error("Error: bind");

    if (listen(listen_fd, TG_SERVER_BACKLOG_CONN) < 0)
        error("Error: listen");

    printf("Traffic Generator Server listens on 0.0.0.0:%d\n", servPort);

    while (1)
    {
        sockfd_ptr = (int*)malloc(sizeof(int));
        *sockfd_ptr = accept(listen_fd, (struct sockaddr *)&cli_addr, &len);

        if (*sockfd_ptr < 0)
        {
            close(listen_fd);
            free(sockfd_ptr);
            error("Error: accept");
        }
        else if (pthread_create(&serv_thread, NULL, handle_connection, (void*)sockfd_ptr) < 0)
        {
            close(listen_fd);
            free(sockfd_ptr);
            error("Error: create pthread");
        }
    }

    return 0;
}

/* Handle an incomming connection */
void* handle_connection(void* ptr)
{
    unsigned int flow_id, flow_size, flow_tos, flow_rate;
    unsigned int meta_data_size = 4 * sizeof(unsigned int);
    char buf[meta_data_size]; // buffer to hold meta data
    int sockfd = *(int*)ptr;
    free(ptr);

    while (1)
    {
        /* read meta-data */
        if (read_exact(sockfd, buf, meta_data_size, meta_data_size, false) != meta_data_size)
        {
            if (debug_mode)
                printf("Cannot read meta-data\n");
            break;
        }

        /* extract meta data */
        memcpy(&flow_id, buf, sizeof(unsigned int));
        memcpy(&flow_size, buf + sizeof(unsigned int), sizeof(unsigned int));
        memcpy(&flow_tos, buf + 2 * sizeof(unsigned int), sizeof(unsigned int));
        memcpy(&flow_rate, buf + 3 * sizeof(unsigned int), sizeof(unsigned int));

        if (debug_mode)
            printf("Flow request: ID: %u Size: %u bytes ToS: %u Rate: %u Mbps\n", flow_id, flow_size, flow_tos, flow_rate);

        /* echo back meta data */
        if (write_exact(sockfd, buf, meta_data_size, meta_data_size, 0, flow_tos, 0, false) != meta_data_size)
        {
            if (debug_mode)
                printf("Cannot write meta-data\n");
            break;
        }

        /* generate a flow with flow_size bytes */
        if (flow_rate > 0)
        {
            /* Use flow_min_buf with rate limiting */
            if (write_exact(sockfd, flow_min_buf, flow_size, TG_MIN_WRITE, flow_rate, flow_tos, sleep_overhead_us, true) != flow_size)
            {
                if (debug_mode)
                    printf("Cannot write enough data into socket buffer with rate limiting\n");
                break;
            }
        }
        else
        {
            /* Use flow_max_buf w/o rate limiting */
            if (write_exact(sockfd, flow_max_buf, flow_size, TG_MAX_WRITE, flow_rate, flow_tos, 0, true) != flow_size)
            {
                if (debug_mode)
                    printf("Cannot write enough data into socket buffer without rate limiting\n");
                break;
            }
        }
    }

    close(sockfd);
    return (void*)0;
}

/* Print usage of the program */
void print_usage(char *program)
{
    printf("Usage: %s [options]\n", program);
    printf("-p <port>   port number (default %d)\n", TG_SERVER_PORT);
    printf("-d          debug mode (print necessary information)\n");
    printf("-h          display help information\n");
}

/* Read command line arguments */
void read_args(int argc, char *argv[])
{
    int i = 1;

    while (i < argc)
    {
        if (strlen(argv[i]) == 2 && strcmp(argv[i], "-p") == 0)
        {
            if (i+1 < argc)
            {
                servPort = atoi(argv[i+1]);
                if (servPort < 0 || servPort > 65535)
                    error("Illegal port number");
                i += 2;
            }
            /* cannot read port number */
            else
            {
                printf("Cannot read port number\n");
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
        else if (strlen(argv[i]) == 2 && strcmp(argv[i], "-h") == 0)
        {
            print_usage(argv[0]);
            exit(EXIT_SUCCESS);
        }
        else if (strlen(argv[i]) == 2 && strcmp(argv[i], "-d") == 0)
        {
            debug_mode = 1;
            i += 1;
        }
        else
        {
            printf("Invalid option %s\n", argv[i]);
            print_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
}

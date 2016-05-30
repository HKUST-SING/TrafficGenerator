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

#include "../common/common.h"

char server_ip[16] = {0};   /* sender IP address */
char read_buf[TG_MAX_READ] = {1};
int server_port = TG_SERVER_PORT;   /* sender TCP port */
struct flow_metadata flow;
unsigned int flow_number = 10;  /* number of flows */

bool set_flow_tos = false;

/* print usage of the program */
void print_usage(char *program);
/* read command line arguments */
void read_args(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    unsigned int i = 0;
    struct timeval tv_start, tv_end;    /* start and end time */
    int sockfd; /* socket */
    int sock_opt = 1;
    struct sockaddr_in serv_addr;   /* server address */
    unsigned int fct_us;
    unsigned int goodput_mbps;
    flow.size = 1024;  /* flow size in bytes */
    flow.tos = 0;  /* ToS value of flows */
    flow.rate = 0;  /* sending rate of flows */

    read_args(argc,argv);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(server_ip);
    serv_addr.sin_port = htons(server_port);

    /* initialize server socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Error: initialize socket");

    /* set socket options */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt)) < 0)
        error("Error: set SO_REUSEADDR option");
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &sock_opt, sizeof(sock_opt)) < 0)
        error("ERROR: set TCP_NODELAY option");
    if (setsockopt(sockfd, IPPROTO_IP, IP_TOS, &flow.tos, sizeof(flow.tos)) < 0)
        error("Error: set IP_TOS option");

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("Error: connect");

    for (i = 0; i < flow_number; i ++)
    {
        printf("Generate flow request %u\n", i);
        flow.id = i;

        if (!set_flow_tos)
            flow.tos += 4;

        gettimeofday(&tv_start, NULL);

        if (!write_flow_req(sockfd, &flow))
            error("Error: generate request");

        if (!read_flow_metadata(sockfd, &flow))
            error("Error: read metadata");

        if (read_exact(sockfd, read_buf, flow.size, TG_MAX_READ, true) != flow.size)
            error("Error: receive flow");

        gettimeofday(&tv_end, NULL);
        fct_us = (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + (tv_end.tv_usec - tv_start.tv_usec);
        goodput_mbps = flow.size * 8 / fct_us;

        printf("Flow: ID: %u\nSize: %u bytes ToS: %u Rate: %u Mbps\n", flow.id, flow.size, flow.tos, flow.rate);
        printf("FCT: %u us Goodput: %u Mbps\n", fct_us, goodput_mbps);
    }

    close(sockfd);
    return 0;
}

void print_usage(char *program)
{
    printf("Usage: %s [options]\n", program);
    printf("-s <sender>        IP address of sender (required)\n");
    printf("-p <port>          port number (default %d)\n", TG_SERVER_PORT);
    printf("-n <bytes>         flow size in bytes (default %u)\n", flow.size);
    printf("-q <tos>           Type of Service (ToS) value (default increased from %u)\n", flow.tos);
    printf("-c <count>         number of flows (default %u)\n", flow_number);
    printf("-r <rate (Mbps)>   sending rate of flows (default 0: no rate limiting)\n");
    printf("-h                 display help information\n");
}

void read_args(int argc, char *argv[])
{
    int i = 1;

    if (argc == 1)
    {
        print_usage(argv[0]);
        exit(EXIT_SUCCESS);
    }

    while (i < argc)
    {
        if (strlen(argv[i]) == 2 && strcmp(argv[i], "-s") == 0)
        {
            if (i+1 < argc)
            {
                if (strlen(argv[i+1]) <= 15)
                    strncpy(server_ip, argv[i+1], strlen(argv[i+1]));
                else
                    error("Invalid IP address\n");
                i += 2;
            }
            /* cannot read IP address */
            else
            {
                printf("Cannot read IP address\n");
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
        else if (strlen(argv[i]) == 2 && strcmp(argv[i], "-p") == 0)
        {
            if (i+1 < argc)
            {
                server_port = atoi(argv[i+1]);
                if (server_port < 0 || server_port > 65535)
                    error("Invalid port number");
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
        else if (strlen(argv[i]) == 2 && strcmp(argv[i], "-n") == 0)
        {
            if (i+1 < argc)
            {
                sscanf(argv[i+1], "%u", &(flow.size));
                i += 2;
            }
            /* cannot read flow size */
            else
            {
                printf("Cannot read flow size\n");
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
        else if (strlen(argv[i]) == 2 && strcmp(argv[i], "-q") == 0)
        {
            if (i+1 < argc)
            {
                set_flow_tos = true;
                sscanf(argv[i+1], "%u", &(flow.tos));
                i += 2;
            }
            /* cannot read ToS value */
            else
            {
                printf("Cannot read ToS\n");
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
        else if (strlen(argv[i]) == 2 && strcmp(argv[i], "-c") == 0)
        {
            if (i+1 < argc)
            {
                sscanf(argv[i+1], "%u", &flow_number);
                i += 2;
            }
            /* cannot read number of flows */
            else
            {
                printf("Cannot read number of flows\n");
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
        else if (strlen(argv[i]) == 2 && strcmp(argv[i], "-r") == 0)
        {
            if (i+1 < argc)
            {
                sscanf(argv[i+1], "%u", &(flow.rate));
                i += 2;
            }
            /* cannot read sending rate of flows */
            else
            {
                printf("Cannot read sending rate of flows\n");
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
        else if (strlen(argv[i]) == 2 && strcmp(argv[i], "-h") == 0)
        {
            print_usage(argv[0]);
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("Invalid option %s\n", argv[i]);
            print_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
}

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "conn.h"

/* Initialize connection */
bool Init_Conn_Node(struct Conn_Node* node, int id, struct Conn_List* list)
{
    struct sockaddr_in serv_addr;   //Server address
    int sock_opt = 1;

    if (!node)
        return false;

    node->id = id;
    node->busy = false;
    node->next = NULL;
    node->list = list;
    node->connected = false;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(list->ip);
    serv_addr.sin_port = htons(list->port);

    /* Initialize server socket */
    node->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (node->sockfd < 0)
    {
        perror("Error: initialize socket");
        return false;
    }

    /* Set socket options */
    if (setsockopt(node->sockfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt)) < 0)
    {
        perror("Error: set SO_REUSEADDR option");
        return false;
    }
    if (setsockopt(node->sockfd, IPPROTO_TCP, TCP_NODELAY, &sock_opt, sizeof(sock_opt)) < 0)
    {
        perror("ERROR: set TCP_NODELAY option");
        return false;
    }

    if (connect(node->sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error: connect");
        return false;
    }

    node->connected = true;
    return true;
}

bool Init_Conn_List(struct Conn_List* list, int index, char *ip, unsigned short port)
{
    if (!list)
        return false;

    if (strlen(ip) < sizeof(list->ip))
    {
        strcpy(list->ip, ip);
        list->ip[strlen(list->ip)] = '\0';
    }
    else
        return false;

    list->index = index;
    list->port = port;
    list->head = NULL;
    list->tail = NULL;
    list->len = 0;
    list->available_len = 0;
    list->flow_finished = 0;
    pthread_mutex_init(&(list->lock), NULL);

    return true;
}

/* Insert several nodes to the tail of the linked list */
bool Insert_Conn_List(struct Conn_List* list, int num)
{
    int i = 0;
    struct Conn_Node* new_node = NULL;

    if (!list)
        return false;

    for (i = 0; i < num; i++)
    {
        new_node = (struct Conn_Node*)malloc(sizeof(struct Conn_Node));
        if (!Init_Conn_Node(new_node, list->len, list))
        {
            free(new_node);
            return false;
        }

        /* If the list is empty */
        if (list->len == 0)
        {
            list->head = new_node;
            list->tail = new_node;
        }
        else
        {
            list->tail->next = new_node;
            list->tail = new_node;
        }
        pthread_mutex_lock(&(list->lock));
        list->len++;
        list->available_len++;
        pthread_mutex_unlock(&(list->lock));
    }

    return true;
}

/* Find the first available connection (busy==false && connected==true) in the list. */
struct Conn_Node* Search_Conn_List(struct Conn_List* list)
{
    struct Conn_Node* ptr = NULL;

    if (!list || !(list->available_len))
        return NULL;

    ptr = list->head;
    while (true)
    {
        if (!ptr)
            return NULL;
        else if (!(ptr->busy) && ptr->connected)
            return ptr;
        else
            ptr = ptr->next;
    }

    return NULL;
}

/* Find N available connections in the list */
struct Conn_Node** Search_N_Conn_List(struct Conn_List* list, unsigned int num)
{
    struct Conn_Node* ptr = NULL;
    struct Conn_Node** result = NULL;
    int i = 0;

    if (!list || list->available_len < num || !num)
        return NULL;

    result = (struct Conn_Node**)malloc(num * sizeof(struct Conn_Node*));
    if (!result)
    {
        perror("Error: malloc");
        return NULL;
    }

    ptr = list->head;
    while (true)
    {
        if (ptr)
        {
            if (!(ptr->busy) && ptr->connected)
                result[i++] = ptr;

            if (i < num)
                ptr = ptr->next;
            else
                return result;
        }
        else
        {
            free(result);
            return NULL;
        }
    }

    return NULL;
}

/* Wait for all threads in the linked list to finish */
void Wait_Conn_List(struct Conn_List* list)
{
    struct Conn_Node* ptr = NULL;
    struct timespec ts;
    int s;

    if (!list)
        return;

    ptr = list->head;
    while (true)
    {
        if (!ptr)
            break;
        else
        {
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 5;
            s = pthread_timedjoin_np(ptr->thread, NULL, &ts);
            if (s != 0)
                perror("Cannot wait for this thread to stop\n");
            ptr = ptr->next;
        }
    }
}

/* Clear all the nodes in the linked list */
void Clear_Conn_List(struct Conn_List* list)
{
    struct Conn_Node* ptr = NULL;
    struct Conn_Node* next_node = NULL;

    if (!list)
        return;

    for (ptr = list->head; ptr != NULL; ptr = next_node)
    {
        next_node = ptr->next;
        free(ptr);
    }

    list->len = 0;
}

/* Print information of the linked list */
void Print_Conn_List(struct Conn_List* list)
{
    if (list)
        printf("%s:%hu  total connection number: %u  available connection number: %u  flows finished: %u\n", list->ip, list->port, list->len, list->available_len, list->flow_finished);
}

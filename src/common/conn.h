#ifndef CONN_H
#define CONN_H

#define _GNU_SOURCE

#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

struct Conn_List;

struct Conn_Node
{
    int id; //node id in the link
    int sockfd; //socket
    pthread_t thread;    //thread
    bool busy; //whether the connection is receiving data
    bool connected; //whether the connection is established
    struct Conn_Node* next;  //pointer to next node
    struct Conn_List* list; //pointer to parent list
};

struct Conn_List
{
    int index;  //server index
    char ip[20]; //server IP address
    unsigned short port;   //server port number
    struct Conn_Node* head;  //pointer to head node
    struct Conn_Node* tail;  //pointer to tail node
    unsigned int len;   //total number of nodes
    unsigned int available_len; //total number of available nodes
    unsigned int flow_finished; //the total number of flows finished
    pthread_mutex_t lock;
};


/* Initialize functions */
bool Init_Conn_Node(struct Conn_Node* node, int id, struct Conn_List* list);
bool Init_Conn_List(struct Conn_List* list, int index, char *ip, unsigned short port);

/* Insert several nodes to the tail of the linked list */
bool Insert_Conn_List(struct Conn_List* list, int num);

/* Find the first available connection (busy==false) in the list. */
struct Conn_Node* Search_Conn_List(struct Conn_List* list);

/* Find N available connections in the list */
struct Conn_Node** Search_N_Conn_List(struct Conn_List* list, unsigned int num);

/* Wait for all threads in the linked list to finish */
void Wait_Conn_List(struct Conn_List* list);

/* Clear all the nodes in the linked list */
void Clear_Conn_List(struct Conn_List* list);

/* Print information of the linked list */
void Print_Conn_List(struct Conn_List* list);

#endif

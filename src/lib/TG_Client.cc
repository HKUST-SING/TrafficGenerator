#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

#include "TG_Client.h"

using namespace std;

#define TG_Client_Query_Len 64

TG_Client::TG_Client(char* ip, int port)
{
    /* Initialize server local address */
    memset(&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family=AF_INET;  //IP protocol

    if(ip!=NULL)
        serverAddr.sin_addr.s_addr=inet_addr(ip);   //IP address

    if(port>0)
        serverAddr.sin_port=htons(port);    //TCP port number

}

unsigned long TG_Client::request(int size, int clientToS, int serverToS)
{
    struct timeval tv_start, tv_end;	//Start and End time
    int len;	//Read length
    char buf[BUFSIZ];	//Receive buffer
    char query[TG_Client_Query_Len];    //query=data size+ToS
    char tosStr[4]; //string of serverToS
    char sizeStr[TG_Client_Query_Len-4];    //string of size
    unsigned long fct;	//Flow Completion Time

    memset(query,'\0',TG_Client_Query_Len);
    memset(sizeStr,'\0',TG_Client_Query_Len-4);
    memset(tosStr,'\0',4);

    snprintf(sizeStr, TG_Client_Query_Len-5, "%d", size);
    snprintf(tosStr, 3, "%d", serverToS);

    strcat(query,sizeStr);
	strcat(query," ");
	strcat(query,tosStr);

    /* Initialize client socket */
    if((clientSockfd=socket(PF_INET,SOCK_STREAM,0))<0)
    {
        cerr<<"socket error\n";
        return 0;
    }

    /* Set ToS */
    if(clientToS>0&&clientToS<=255)
        setsockopt(clientSockfd, IPPROTO_IP, IP_TOS, &clientToS, sizeof(clientToS));

    /* Establish connection */
    if(connect(clientSockfd,(struct sockaddr *)&serverAddr,sizeof(struct sockaddr))<0)
    {
        cerr<<"connect error\n";
        return 0;
    }

    /* Get start time after connection establishment */
    gettimeofday(&tv_start,NULL);

    len=send(clientSockfd,query,strlen(query),0);
    if(len<=0)
    {
        cerr<<"send error\n";
        return 0;
    }

    while(1)
    {
        len=recv(clientSockfd,buf,BUFSIZ,0);
        if(len<=0)
            break;
    }

    /*Get end time after receiving all of the data */
    gettimeofday(&tv_end,NULL);
    close(clientSockfd);

    /* Calculate time interval (unit: us) */
    fct=(tv_end.tv_sec-tv_start.tv_sec)*1000000+(tv_end.tv_usec-tv_start.tv_usec);

    return fct;
}

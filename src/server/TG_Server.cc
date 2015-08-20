#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "TG_Server.h"

using namespace std;

TG_Server::TG_Server(char *ip, int port, int connBacklog)
{
    clientSockfdPtr=NULL;

    if(connBacklog>0)
        backlog=connBacklog;
    else
        backlog=TG_SERVER_DEFAULT_BACKLOG;

    /* Initialize server local address */
    memset(&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family=AF_INET;  //IP protocol

    if(ip!=NULL)
        serverAddr.sin_addr.s_addr=inet_addr(ip);   //IP address
    else
        serverAddr.sin_addr.s_addr=INADDR_ANY;  //0.0.0.0 by default

    if(port>0)
        serverAddr.sin_port=htons(port);    //TCP port number
    else
        serverAddr.sin_port=htons(TG_SERVER_DEFAULT_PORT);
}

static void* TG_Server_Thread(void* clientSockfdPtr)
{
    int clientSockfd=*(int*)clientSockfdPtr;
    char writeMsg[BUFSIZ+1]={0};
    char readMsg[1024]={0};
    char *savePtr;
	int i, len, dataSize, remainingSize, tos, loop;

    delete (int*)clientSockfdPtr;
    memset(writeMsg,1,BUFSIZ);
	writeMsg[BUFSIZ]='\0';
	len=recv(clientSockfd,readMsg,1024,0);

    if(len<=0)
	{
        cerr<<"receive error\n";
        goto err;
    }

    dataSize=atoi(strtok_r(readMsg," ",&savePtr)); //Get data volume (KB)
    tos=atoi(strtok_r(NULL," ",&savePtr)); //Get ToS

    if(tos>0&&tos<=255)
        setsockopt(clientSockfd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos));

    loop=dataSize*1024/BUFSIZ;  //Calculate send loop
    remainingSize=dataSize*1024-loop*BUFSIZ;    //Get size to sent in the last round

    for(i=0;i<loop;i++)
    {
        send(clientSockfd,writeMsg,strlen(writeMsg),0);
    }

    if(remainingSize>0)
    {
        writeMsg[remainingSize]='\0';
        send(clientSockfd,writeMsg,strlen(writeMsg),0);
    }

err:
    close(clientSockfd);
    return (void *)0;
}

void TG_Server::start()
{
    unsigned int sin_size=sizeof(struct sockaddr_in);
    pthread_t serverThread;

    /* Initialize server socket */
    if((serverSockfd=socket(PF_INET,SOCK_STREAM,0))<0)
	{
		cerr<<"socket error\n";
		return;
	}

    /* Bind server socket on local address */
    if(bind(serverSockfd,(struct sockaddr *)&serverAddr,sizeof(struct sockaddr))<0)
    {
        cerr<<"bind error\n";
        return;
    }

    /* Start listen */
    listen(serverSockfd, backlog);

    while(1)
    {
        clientSockfdPtr=new int;
        *clientSockfdPtr=accept(serverSockfd,(struct sockaddr *)&clientAddr,&sin_size);
        if(*clientSockfdPtr<0)
        {
            cerr<<"accept error\n";
            delete clientSockfdPtr;
        }
        else if(pthread_create(&serverThread, NULL , TG_Server_Thread, (void*)clientSockfdPtr)<0)
        {
            cerr<<"cannot creat thread\n";
            delete clientSockfdPtr;
        }
    }
}

void TG_Server::stop()
{
    close(serverSockfd);
}

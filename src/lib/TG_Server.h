#ifndef TG_SERVER_H
#define TG_SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>

#define TG_SERVER_DEFAULT_PORT 5001
#define TG_SERVER_DEFAULT_BACKLOG 5

class TG_Server
{
    public:
        TG_Server(char *ip, int port, int connBacklog);
        ~TG_Server(){};
        void start();
        void stop();

    private:
        int serverSockfd;   //server socket
        int *clientSockfdPtr;   //client socket
        struct sockaddr_in serverAddr;  //server address that TG_Server listens on
        struct sockaddr_in clientAddr;  //client address
        int backlog;    //the number of backlog connections for listen function
};

#endif

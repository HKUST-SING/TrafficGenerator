#ifndef TG_CLIENT_H
#define TG_CLIENT_H

#include <sys/socket.h>
#include <netinet/in.h>

class TG_Client
{
    public:
        TG_Client(char* ip, int port);
        ~TG_Client(){}
        unsigned long request(int size, int clientToS, int serverToS);    //request for data and return FCT result

    private:
        int clientSockfd;   //client socket
        struct sockaddr_in serverAddr;  //server address that TG_Client sends requests to
};

#endif

#include <iostream>
#include <stdlib.h>
#include "../../client/TG_Client.h"

using namespace std;

int main(int argc, char** argv)
{
    if(argc==6)
    {
        char* ip=argv[1];
        int port=atoi(argv[2]);
        int size=atoi(argv[3]);
        int clientToS=atoi(argv[4]);
        int serverToS=atoi(argv[5]);

        TG_Client client(ip,port);
        unsigned long fct=client.request(size, clientToS, serverToS);
        cout<<"FCT is "<<fct<<" us"<<endl;
    }
    else
    {
        cout<<"Usage: "<<argv[0]<<" [server IP] [server port] [flow size (KB)] [client ToS] [server ToS]"<<endl;
    }

    return 0;
}

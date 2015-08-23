#include <iostream>
#include <stdlib.h>
#include "TG_Server.h"

using namespace std;

int main(int argc, char** argv)
{
    if(argc==4)
    {
        char *ip=argv[1];
        int port=atoi(argv[2]);
        int backlog=atoi(argv[3]);

        TG_Server server(ip,port,backlog);
        server.start();
    }
    else
    {
        cout<<"Usage: "<<argv[0]<<" [server IP] [server port] [backlog connections]"<<endl;
    }
    return 0;
}

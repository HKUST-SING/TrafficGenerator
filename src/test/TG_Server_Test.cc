#include <iostream>
#include "../server/TG_Server.h"

int main()
{
    TG_Server server(NULL,5001,100);
    server.start();
    return 0;
}

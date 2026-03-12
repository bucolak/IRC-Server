#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <vector>
#include "libs/Server.hpp"

int main(int argc, char *argv[])
{
    Server s1(atoi(argv[1]), argv[2]);

    if(argc != 3)
    {
        std::cout<<"Usage: ./ircserv <port> <password>"<<std::endl;
        return 0;
    }

    try
    {
        s1.init_server();
        s1.execute_server();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}
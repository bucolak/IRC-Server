#include <iostream>
#include <cstdlib>
#include <csignal>
#include "libs/Server.hpp"

int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, handle_sigint);
    if(argc != 3)
    {
        std::cout<<"Usage: ./ircserv <port> <password>"<<std::endl;
        return 0;
    }
    int port = atoi(argv[1]);
    if(port < 1 || port > 65535)
    {
        std::cout<<"Error: Invalid port number!"<<std::endl;
        return 0;
    }
    
    Server s1(atoi(argv[1]), argv[2]);

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

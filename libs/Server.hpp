#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <map>
#include "Client.hpp"
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>


class Server {

    private:
        std::vector<pollfd> fds;
        std::map<int, Client*> clients;
        std::string password;
        int port;
        int server_fd;
    public:
        Server(int port, std::string password);
        std::vector<pollfd> get_fds();
        std::string get_password();
        int get_server_fd();
        int get_port();

        void init_server();
        void execute_server();
        ~Server();
};

#endif
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
#include <sstream>


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
        void find_first_msg(int wc);
        void parse_msg(std::string str);
        void handle_cmds();

        void handle_pass(Client *client, std::vector<std::string> args);
        void handle_nick(Client *client, std::vector<std::string> args);
        ~Server();
};

#endif
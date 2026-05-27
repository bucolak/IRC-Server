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
#include "Channel.hpp"
#include <cstdlib>
#include <csignal>
#include <cerrno>

#define MSG_SIZE 512

extern volatile sig_atomic_t g_shutdown_requested;
void handle_sigint(int signal);

class Server {

    private:
        std::vector<pollfd> fds;
        std::map<int, Client*> clients;
        std::map<std::string, Channel*> chnnls;
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
        void handle_client_data(size_t &i, size_t &ready_count);
        void disconnect_client(int dead_fd, size_t &i, size_t &ready_count);
        void accept_new_client();

        void find_first_msg(int wc);
        void parse_msg(std::string str, Client *client);
        bool mode_parser(Client *client, std::vector<std::string> collections, bool &changed);

        void handle_pass(Client *client, std::vector<std::string> collections);
        void handle_ping(Client *client, std::vector<std::string> collections);
        void handle_nick(Client *client, std::vector<std::string> collections);
        void handle_user(Client *client, std::vector<std::string> collections);
        void handle_join(Client *client, std::vector<std::string> collections);
        void handle_privmsg(Client *client, std::vector<std::string> collections);
        void handle_part(Client *client, std::vector<std::string> collections);
        void handle_quit(Client *client, std::vector<std::string> collections);
        void handle_kick(Client *client, std::vector<std::string> collections);
        void handle_topic(Client *client, std::vector<std::string> collections);
        void handle_invite(Client *client, std::vector<std::string> collections);
        void handle_mode(Client *client, std::vector<std::string> collections);
        void handle_who(Client *client, std::vector<std::string> collections);

        bool is_same_nick(std::string nick);
        bool is_valid_nick(std::string nick);
        bool has_this_channel(std::string c_name);
        bool is_member_of_channel(Client *client, std::string c_name);
        void chck_registration(Client *client);
        Client* get_client_for_fd(std::string nick);

        ~Server();
};

#endif

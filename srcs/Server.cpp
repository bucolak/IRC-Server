#include "../libs/Server.hpp"

Server::Server(int port, std::string password)
{
    this->port = port;
    this->password = password;
}

std::vector<pollfd> Server::get_fds()
{
    return fds;
}

std::string Server::get_password()
{
    return password;
}

int Server::get_server_fd()
{
    return server_fd;
}

int Server::get_port()
{
    return port;
}

void Server::init_server()
{
    this->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1)
        throw std::runtime_error("Error: Socket could not be created!");
    
    int optval = 1;
    if((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) == -1)
        throw std::runtime_error("Error: setsockopt!");

    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    sockaddr_in addr = {};
    addr.sin_family= AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_fd, (sockaddr *)&addr, sizeof(addr)) == -1)
        throw std::runtime_error("Error: bind");

    if(listen(server_fd, 10) == -1)
        throw std::runtime_error("Error: listen");

    pollfd server_poll = {};
    server_poll.fd = server_fd;
    server_poll.events = POLLIN;
    fds.push_back(server_poll);
}

void Server::execute_server()
{
    while(1)
    {
        if(poll(&fds[0], fds.size(), -1) == -1) // -1 sonsuza dek bekle demektir, CPU'yu %0 yorar.
            throw std::runtime_error("Error: poll!");
        
        for (size_t i = 0; i < fds.size(); i++)
        {
            if(fds[i].fd == server_fd)
            {
                if(fds[i].revents & POLLIN)
                {
                    sockaddr_in client_addr = {};
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(server_fd, (sockaddr *)&client_addr, &client_len);
                    fcntl(client_fd, F_SETFL, O_NONBLOCK);
                    if(client_fd != -1)
                    {
                        std::cout<<"We have a new clint! Fd : "<<client_fd<<std::endl;
                        Client* new_client = new Client(client_fd);
                        clients[client_fd] = new_client;

                        pollfd client_poll = {};
                        client_poll.fd = client_fd;
                        client_poll.events = POLLIN;
                        fds.push_back(client_poll);
                    }
                }
            }
            else
            {
                if(fds[i].revents & POLLIN)
                {
                    char buf[1000] = {};
                    int result_recv = recv(fds[i].fd, buf, sizeof(buf), 0);
                    if(result_recv > 0)
                    {
                        std::cout<<"There is a message: "<<buf<<std::endl;
                        send(fds[i].fd, buf, result_recv, 0);
                    }
                    else if(result_recv == 0)
                    {
                        std::cout<<"Client went"<<std::endl;

                        delete clients[fds[i].fd];
                        fds.erase(fds.begin() + i);
                        close(fds[i].fd);
                        i--;
                    }
                    else
                        throw std::runtime_error("Error: recv");
                }
            }
        }
    }
}

Server::~Server() 
{
    for(std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
        delete it->second;
    if(server_fd != -1)
        close(server_fd);
}
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <vector>

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        std::cout<<"Usage: ./ircserv <port> <password>"<<std::endl;
        return 0;
    }
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1)
    {
        std::cout<<"Error: Socket could not be created!"<<std::endl;
        return 0;
    }
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family= AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = INADDR_ANY;

    int result_bind = bind(server_fd, (sockaddr *)&addr, sizeof(addr));
    if(result_bind == -1)
    {
        std::cout<<"Error: bind"<<std::endl;
        return 0;
    }

    int result_listen = listen(server_fd, 10);
    if(result_listen == -1)
    {
        std::cout<<"Error: listen"<<std::endl;
        return 0;
    }

    std::vector<struct pollfd> fds;
    pollfd server_poll;
    server_poll.fd = server_fd;
    server_poll.events = POLLIN;

    fds.push_back(server_poll);

    while(1)
    {
        int p = poll(&fds[0], fds.size(), -1); // -1 sonsuza dek bekle demektir, CPU'yu %0 yorar.
        if(p == -1)
        {
            std::cout<<"Error: poll!"<<std::endl;
            return 0;
        }
        for (int i = 0; i < fds.size(); i++)
        {
            if(fds[i].fd == server_fd)
            {
                if(fds[i].revents & POLLIN)
                {
                    sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(server_fd, (sockaddr *)&client_addr, &client_len);
                    fcntl(client_fd, F_SETFL, O_NONBLOCK);
                    if(client_fd != -1)
                    {
                        std::cout<<"We have a new clint! Fd : "<<client_fd<<std::endl;
                        pollfd client_poll;
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
                    char buf[1000];
                    int result_recv = recv(fds[i].fd, buf, sizeof(buf), 0);
                    if(result_recv > 0)
                    {
                        std::cout<<"There is a message: "<<buf<<std::endl;
                        send(fds[i].fd, buf, sizeof(buf), 0);
                    }
                    else if(result_recv == 0)
                    {
                        std::cout<<"Client gone"<<std::endl;
                        fds.erase(fds.begin() + i);
                        i--;
                        close(fds[i].fd);
                    }
                    else
                    {
                        std::cout<<"Error: recv"<<std::endl;
                        return 0;
                    }
                }
            }
        }
    }

}
#include "../libs/Client.hpp"

Client::Client(int client_fd)
{
    this->client_fd = client_fd;
}

int Client::get_client_fd()
{
    return client_fd;
}

std::string Client::get_user_name()
{
    return user_name;
}

std::string Client::get_nick_name()
{
    return nick_name;
}

std::string Client::get_buffer()
{
    return buffer;
}
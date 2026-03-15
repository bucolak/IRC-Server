#include "../libs/Client.hpp"

Client::Client(int client_fd)
{
    this->client_fd = client_fd;
}

int Client::get_client_fd()
{
    return client_fd;
}

int Client::get_is_registered()
{
    return is_registered;
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

void Client::set_buffer(std::string str)
{
    buffer = str;
}

void Client::set_is_authenticated(bool state)
{
    is_authenticated = state;
}

void Client::buffer_append(std::string newstr)
{
    buffer += newstr;
}
#include "../libs/Channel.hpp"

Channel::Channel(std::string channel_name)
{
    this->channel_name = channel_name;
}

void Channel::add_members(Client *client, bool is_oprtr)
{
    members[client] = is_oprtr;
}

void Channel::remove_members(Client *client)
{
    members.erase(client);
}

void Channel::broadcast_msg(std::string &msg, int fd)
{
    std::map<Client *, bool>::iterator it;
    for(it = members.begin(); it != members.end(); ++it)
    {
        if(it->first->get_client_fd() != fd)
            send(it->first->get_client_fd(), msg.c_str(), msg.length(), 0);
    }
}

Channel::~Channel() {}
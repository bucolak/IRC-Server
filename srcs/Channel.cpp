#include "../libs/Channel.hpp"

Channel::Channel(std::string channel_name)
{
    this->channel_name = channel_name;
}

std::string Channel::get_channel_name()
{
    return channel_name;
}

std::string Channel::get_topic()
{
    return channel_topic;
}

std::string Channel::get_members()
{
    std::map<Client *, bool>::iterator it;
    std::string str;
    for(it = members.begin(); it != members.end(); ++it)
    {
        if(it->second == true)
            str += (" @" + it->first->get_user_name());
        else
            str += (" " + it->first->get_user_name());
    }
    return str;
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

bool Channel::is_operator(Client *client)
{
    for(std::map<Client *, bool>::iterator it = members.begin(); it != members.end(); ++it)
    {
        if(it->first == client && it->second == true)
            return true;
    }
    return false;
}


Channel::~Channel() {}
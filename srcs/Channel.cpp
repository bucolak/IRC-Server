#include "../libs/Channel.hpp"
#include <sys/socket.h>

Channel::Channel(std::string channel_name)
{
    this->channel_name = channel_name;
    this->only_invite = false;
    this->is_restricted_topic = true;
    this->is_channel_limited = false;
    this->limit_user = 0;
    this->creation_time = std::time(NULL);
}

std::string Channel::get_channel_name()
{
    return channel_name;
}

std::string Channel::get_topic()
{
    return channel_topic;
}

std::string Channel::get_channel_key()
{
    return channel_key;
}

std::string Channel::get_creation_time()
{
    std::stringstream ss;
    ss << creation_time;
    return ss.str();
}

unsigned int Channel::get_limit_user()
{
    return limit_user;
}

unsigned int Channel::get_member_count()
{
    return members.size();
}

bool Channel::get_is_restricted_topic()
{
    return is_restricted_topic;
}

bool Channel::get_only_invite()
{
    return only_invite;
}

bool Channel::get_is_channel_limited()
{
    return is_channel_limited;
}

void Channel::set_topic(std::string channel_topic)
{
    this->channel_topic = channel_topic;
}

void Channel::set_channel_key(std::string channel_key)
{
    this->channel_key = channel_key;
}

void Channel::set_limit_user(unsigned int limit_user)
{
    this->limit_user = limit_user;
}

void Channel::set_only_invite(bool only_invite)
{
    this->only_invite = only_invite;
}

void Channel::set_is_restricted_topic(bool is_restricted_topic)
{
    this->is_restricted_topic = is_restricted_topic;
}

void Channel::set_is_channel_limited(bool is_channel_limited)
{
    this->is_channel_limited = is_channel_limited;
}

void Channel::set_operator(Client *client, bool sign)
{
    members[client] = sign;
}

std::string Channel::get_members()
{
    std::map<Client *, bool>::iterator it;
    std::string str;
    for(it = members.begin(); it != members.end(); ++it)
    {
        if(it->second == true)
            str += (" @" + it->first->get_nick_name());
        else
            str += (" " + it->first->get_nick_name());
    }
    return str;
}

std::string Channel::get_modes()
{
    std::string modes = "+";
    std::string will_append=" ";
    if(!channel_key.empty())
    {
        modes+="k";
        will_append += (channel_key + " ");
    }
    if(limit_user>0)
    {
        modes+="l";
        std::stringstream str;
        str << limit_user;
        will_append += str.str();
        will_append+=" ";
    }
    if(only_invite)
        modes+="i";
    if(is_restricted_topic)
        modes+="t";
    if(modes == "+")
        return "+";
    return modes + will_append;
}

void Channel::add_invited_lists(Client *client)
{
    intvited_users_list.push_back(client);
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

std::vector<int> Channel::get_member_fds()
{
    std::vector<int> out;
    for(std::map<Client *, bool>::iterator it = members.begin(); it != members.end(); ++it)
        out.push_back(it->first->get_client_fd());
    return out;
}

bool Channel::is_user_invited(Client *client)
{
    std::vector<Client *>::iterator it;
    for (it = intvited_users_list.begin(); it != intvited_users_list.end(); ++it)
    {
        if (*it == client)
            return true;
    }
    return false;
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

bool Channel::is_member(Client *client)
{
    return members.find(client) != members.end();
}


Channel::~Channel() {}

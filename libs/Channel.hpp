#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <map>
#include "Client.hpp"

class Channel {

    private:
        std::string channel_name;
        std::map<Client *, bool> members;
    public:
        Channel(std::string channel_name);
        ~Channel();
        void add_members(Client *client, bool is_oprtr);
        void remove_members(Client *client);
        void broadcast_msg(std::string &msg, int fd);
};



#endif
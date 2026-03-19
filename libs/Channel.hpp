#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <map>
#include "Client.hpp"

class Channel {

    private:
        std::string channel_name;
        std::string channel_topic;
        std::string channel_key; // Kanal şifresi (+k modu için)
        unsigned int limit_user; // Kanal kişi sınırı (+l modu için)
        bool only_invite; // sadece davetle girilebilir (+i modu)
        bool is_restricted_topic; // Başlık değiştirme yetkisini kısıtlar (+t modu).
        std::vector<Client *> intvited_users_list; // Davet edilen kişilerin listesi.
        std::map<Client *, bool> members;
    public:
        Channel(std::string channel_name);
        ~Channel();

        std::string get_topic();
        std::string get_members();


        void add_members(Client *client, bool is_oprtr);
        void remove_members(Client *client);
        void broadcast_msg(std::string &msg, int fd);
};



#endif
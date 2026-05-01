#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <map>
#include "Client.hpp"
#include <sstream>
#include <vector>

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

        std::string get_channel_name();
        std::string get_topic();
        std::string get_members();
        std::string get_modes();
        std::string get_channel_key();
        unsigned int get_limit_user();
        bool get_only_invite();
        bool get_is_restricted_topic();
        void set_channel_key(std::string channel_key);
        void set_limit_user(unsigned int limit_user);
        void set_only_invite(bool only_invite);
        void set_is_restricted_topic(bool is_restricted_topic);
        void set_topic(std::string channel_topic);
        void set_operator(Client *client, bool sign);
    
        void add_invited_lists(Client *client);
        void add_members(Client *client, bool is_oprtr);
        void remove_members(Client *client);
        void broadcast_msg(std::string &msg, int fd);
        std::vector<int> get_member_fds();

        bool is_operator(Client *client);
};



#endif
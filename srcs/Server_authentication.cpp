#include "../libs/Server.hpp"
#include "../libs/ProtocolMessages.hpp"

void Server::handle_ping(Client *client, std::vector<std::string> collections)
{
    std::string pong = ":ircserv PONG ircserv ";
    if (collections.size() < 2)
        pong += ":ircserv";
    else {
        for (size_t j = 1; j < collections.size(); ++j) {
            if (j > 1)
                pong += " ";
            pong += collections[j];
        }
    }
    pong += "\r\n";
    send(client->get_client_fd(), pong.c_str(), pong.length(), 0);
}

void Server::chck_registration(Client *client)
{
    if(client->get_is_registered())
        return;
    if(!client->get_nick_name().empty() && !client->get_user_name().empty())
    {

        if(client->get_is_authenticated())
        {
            client->set_is_registered(true);
            std::string rply = RPL_WELCOME(client->get_nick_name(), client->get_user_name(), "localhost");
            send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
            std::cout<<"[ CONNECT ] A new client accepted! [FD: "<<client->get_client_fd()<<" , NICK: "<<client->get_nick_name()<<"]"<<std::endl;
        }
        else if(client->get_is_one() == false)
        {
            client->set_is_one(true);
            std::string rply = ERR_PASSWDMISMATCH(client->get_nick_name());
            send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
            client->set_quit_requested(true);
            shutdown(client->get_client_fd(), SHUT_RDWR);
            std::cout<<rply;
        }
    }
}

void Server::handle_pass(Client *client, std::vector<std::string> collections)
{
    std::string nick = client->get_nick_name().empty() ? "*" : client->get_nick_name();
    if(collections.size() < 2)
    {
        std::string rply = ERR_NEEDMOREPARAMS(nick, "PASS");
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        client->set_quit_requested(true);
        shutdown(client->get_client_fd(), SHUT_RDWR);
        return ;
    }
    if(client->get_is_registered() == 1)
    {
        std::string rply = ERR_ALREADYREGISTERED(nick);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        return ;
    }
    if(collections[1] != password)
    {
        std::string rply = ERR_PASSWDMISMATCH(nick);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        client->set_quit_requested(true);
        shutdown(client->get_client_fd(), SHUT_RDWR);
        return ;
    }
    client->set_is_authenticated(true);
}

bool Server::is_same_nick(std::string nick)
{
    std::map<int, Client*>::iterator it;
    for(it = clients.begin(); it != clients.end(); ++it)
    {
        if(it->second->get_nick_name() == nick)
            return true;
    }
    return false;
}

bool Server::is_valid_nick(std::string nick)
{
    if(nick.empty() || nick.length() > 9)
        return false;
    if(std::isdigit(nick[0]) || nick[0] == '#' || nick[0] == '&' || nick[0] == ':')
        return false;
    std::string allowed = "{}-|^_'[]";
    for(size_t i = 0; i < nick.length(); i++)
    {
        if(!std::isalnum(nick[i]) && allowed.find(nick[i]) == std::string::npos)
            return false;
    }
    return true;
}

void Server::handle_nick(Client *client, std::vector<std::string> collections)
{
    std::string nick = client->get_nick_name().empty() ? "*" : client->get_nick_name();

    if(!client->get_is_authenticated())
    {
        std::string rply = ERR_PASSREQUIRED(nick);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        rply = ERROR_PASSREQUIRED(collections[0]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        client->set_quit_requested(true);
        shutdown(client->get_client_fd(), SHUT_RDWR);
        return;
    }
    if(collections.size() < 2)
    {
        std::string rply = ERR_NONICKNAMEGIVEN(nick);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        return;
    }
    if(!is_valid_nick(collections[1]))
    {
        std::string rply = ERR_ERRONEUSNICKNAME(nick, collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        return;
    }
    if(is_same_nick(collections[1]))
    {
        std::string rply = ERR_NICKNAMEINUSE(nick, collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        return;
    }
    client->set_nick_name(collections[1]);
    std::string msg = ":" + nick + "!" + client->get_user_name() + "@localhost NICK :" + collections[1] + "\r\n";
    send(client->get_client_fd(), msg.c_str(), msg.length(), 0);
    std::cout<<"[ NICK    ] Client [FD: "<<client->get_client_fd()<<"] set their nickname: "<<nick<<" -> "<<client->get_nick_name()<<std::endl;
    for (std::map<std::string, Channel *>::iterator it = chnnls.begin(); it != chnnls.end(); ++it) {
        if (is_member_of_channel(client, it->first)) {
            it->second->broadcast_msg(msg, client->get_client_fd());
        }
    }
    chck_registration(client);
}

void Server::handle_user(Client *client, std::vector<std::string> collections)
{
    std::string nick = client->get_nick_name().empty() ? "*" : client->get_nick_name();

    if(!client->get_is_authenticated())
    {
        std::string rply = ERR_PASSREQUIRED(nick);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        rply = ERROR_PASSREQUIRED(collections[0]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        client->set_quit_requested(true);
        return;
    }
    if(collections.size() != 5)
    {
        std::string rply = ERR_NEEDMOREPARAMS(nick, collections[0]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        return;
    }
    if(client->get_is_registered())
    {
        std::string rply = ERR_ALREADYREGISTERED(nick);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        return;
    }
    client->set_user_name(collections[1]);
    client->set_real_name(collections[4].substr(1));
    chck_registration(client);
}

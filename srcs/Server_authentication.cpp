#include "../libs/Server.hpp"
#include "../libs/ProtocolMessages.hpp"

void Server::handle_pass(Client *client, std::vector<std::string> collections)
{
    std::string nick = client->get_nick_name().empty() ? "*" : client->get_nick_name();
    if(collections.size() < 2)
    {
        std::string rply = ERR_NEEDMOREPARAMS(nick, "PASS");
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return ;
    }
    if(client->get_is_registered() == 1)
    {
        std::string rply = ERR_ALREADYREGISTERED(nick);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return ;
    }
    if(collections[1] != password)
    {
        std::string rply = ERR_PASSWDMISMATCH(nick);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
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
        std::cout<<"Please complete the PASS stage first"<<std::endl;
        return;
    }
    if(collections.size() < 2)
    {
        std::string rply = ERR_NONICKNAMEGIVEN(nick);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(!is_valid_nick(collections[1]))
    {
        std::string rply = ERR_ERRONEUSNICKNAME(nick, collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(is_same_nick(collections[1]))
    {
        std::string rply = ERR_NICKNAMEINUSE(nick, collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    client->set_nick_name(collections[1]);
}

void Server::handle_user(Client *client, std::vector<std::string> collections)
{
    if(collections.size() != 5)
    {
        std::string rply = ERR_NEEDMOREPARAMS(client->get_nick_name(), collections[0]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(client->get_is_registered())
    {
        std::string rply = ERR_ALREADYREGISTERED(client->get_nick_name());
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    client->set_user_name(collections[1]);
    if (client->get_is_authenticated() && 
        !client->get_nick_name().empty() && 
        !client->get_user_name().empty())
        {
            client->set_is_registered(true);
            std::string rply = RPL_WELCOME(client->get_nick_name(), client->get_user_name(), "ANLAMADIM");
            send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        }
}
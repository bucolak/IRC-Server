#include "../libs/Server.hpp"
#include "../libs/ProtocolMessages.hpp"

bool Server::has_this_channel(std::string c_name)
{
    if(chnnls.find(c_name) != chnnls.end())
        return true;
    return false;
}

bool Server::is_member_of_channel(Client *client, std::string c_name)
{
    std::string members = chnnls[c_name]->get_members();
    if(members.find(client->get_nick_name()) != std::string::npos)
        return true;
    return false;
}

Client* Server::get_client_for_fd(std::string nick)
{
    std::map<int, Client*>::iterator it;
    for(it = clients.begin(); it != clients.end(); ++it)
    {
        if(it->second->get_nick_name() == nick)
            return it->second;
    }
    return NULL;
}
#include "../libs/Server.hpp"
#include "../libs/ProtocolMessages.hpp"

void Server::handle_join(Client *client, std::vector<std::string> collections)
{
    std::string nick = client->get_nick_name().empty() ? "*" : client->get_nick_name();
    if(collections.size() != 2)
    {
        std::string rply = ERR_NEEDMOREPARAMS(nick, "JOIN");
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(!client->get_is_registered())
    {
        std::string rply = ERR_NOTREGISTERED(nick);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(collections[1][0] != '#' && collections[1][0] != '&')
    {
        std::string rply = ERR_NOSUCHCHANNEL(nick, collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }

    if(chnnls.find(collections[1]) == chnnls.end())
    {
        Channel *new_ch = new Channel(collections[1]);
        new_ch->add_members(client, true);
        chnnls[collections[1]] = new_ch;
    }
    else
        chnnls[collections[1]]->add_members(client, false);
    std::string msg = ":"+client->get_nick_name()+"!"+client->get_user_name()+"@localhost JOIN :"+collections[1]+"\r\n";
    chnnls[collections[1]]->broadcast_msg(msg, -1);

    if(chnnls[collections[1]]->get_topic().empty())
    {
        msg = RPL_NOTOPIC(nick, collections[1]);
        send(client->get_client_fd(), msg.c_str(), msg.length(), 0);
    }
    else
    {
        msg = RPL_TOPIC(nick, collections[1], chnnls[collections[1]]->get_topic());
        send(client->get_client_fd(), msg.c_str(), msg.length(), 0);
    }
    msg = RPL_NAMREPLY(client->get_nick_name(), collections[1], chnnls[collections[1]]->get_members());
    send(client->get_client_fd(), msg.c_str(), msg.length(), 0);
    msg = RPL_ENDOFNAMES(client->get_nick_name(), collections[1]);
    send(client->get_client_fd(), msg.c_str(), msg.length(), 0);
}

void Server::handle_part(Client *client, std::vector<std::string> collections)
{
    std::string nick = client->get_nick_name().empty() ? "*" : client->get_nick_name();
    if(collections.size() < 2)
    {
        std::string rply = ERR_NEEDMOREPARAMS(nick, "PART");
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(!has_this_channel(collections[1]))
    {
        std::string rply = ERR_NOSUCHCHANNEL(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(!is_member_of_channel(client, collections[1]))
    {
        std::string rply = ERR_NOTONCHANNEL(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    std::string prefix = ":" + client->get_nick_name() + "!" + client->get_user_name() + "@localhost";
    std::string msg;
    if(collections.size()>2 && collections[2].size()>0)
        msg = prefix + " PART "+collections[1] + " :"+collections[2]+"\r\n";
    else
        msg = prefix + " PART "+collections[1] + " :"+client->get_nick_name()+"\r\n";
    chnnls[collections[1]]->broadcast_msg(msg, -1);
    chnnls[collections[1]]->remove_members(client);
    if(chnnls[collections[1]]->get_members().empty())
    {
        Channel *tmp = chnnls[collections[1]];
        chnnls.erase(collections[1]);
        delete tmp;
    }
}

void Server::handle_kick(Client *client, std::vector<std::string> collections)
{
    std::string rply;
    if(collections.size()<3)
    {
        rply = ERR_NEEDMOREPARAMS(client->get_nick_name(), "KICK");
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(!has_this_channel(collections[1]))
    {
        rply = ERR_NOSUCHCHANNEL(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(!is_member_of_channel(client, collections[1]))
    {
        rply = ERR_NOTONCHANNEL(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(!chnnls[collections[1]]->is_operator(client))
    {
        rply = ERR_CHANOPRIVSNEEDED(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    Client *will_thrown = get_client_for_fd(collections[2]);
    if(!will_thrown || !is_member_of_channel(will_thrown, collections[1]))
    {
        rply = ERR_USERNOTINCHANNEL(client->get_nick_name(), collections[2], collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    std::string prefix = ":" + client->get_nick_name() + "!" + client->get_user_name() + "@localhost";
    std::string msg;
    if(collections.size()>3 && collections[3].size()>0)
        msg = prefix + " KICK "+collections[1] + " "+collections[2]+ " :"+collections[3]+"\r\n";
    else
        msg = prefix + " KICK "+collections[1] + " "+collections[2]+ " :"+client->get_nick_name()+"\r\n";
    chnnls[collections[1]]->broadcast_msg(msg, -1);
    chnnls[collections[1]]->remove_members(will_thrown);
    if(chnnls[collections[1]]->get_members().empty())
    {
        Channel *tmp = chnnls[collections[1]];
        chnnls.erase(collections[1]);
        delete tmp;
    }
}

void Server::handle_topic(Client *client, std::vector<std::string> collections)
{
    std::string rply;
    if(collections.size()<2)
    {
        rply = ERR_NEEDMOREPARAMS(client->get_nick_name(), "TOPIC");
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(!has_this_channel(collections[1]))
    {
        rply = ERR_NOSUCHCHANNEL(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(!is_member_of_channel(client, collections[1]))
    {
        rply = ERR_NOTONCHANNEL(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(collections.size() == 2)
    {
        if(chnnls[collections[1]]->get_topic().empty())
            rply = RPL_NOTOPIC(client->get_nick_name(), collections[1]);
        else
            rply = RPL_TOPIC(client->get_nick_name(), collections[1], chnnls[collections[1]]->get_topic());
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
    }
    else
    {
        if(collections[2].empty())
            chnnls[collections[1]]->set_topic("");
        else
            chnnls[collections[1]]->set_topic(collections[2]);
        std::string prefix = ":" + client->get_nick_name() + "!" + client->get_user_name() + "@localhost";
        std::string msg = prefix + " TOPIC " + collections[1] + " :"+collections[2]+"\r\n";
        chnnls[collections[1]]->broadcast_msg(msg, -1);
    }
}

void Server::handle_invite(Client *client, std::vector<std::string> collections)
{
    std::string rply;
    if(collections.size()<3)
    {
        rply = ERR_NEEDMOREPARAMS(client->get_nick_name(), "INVITE");
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    Client *c = get_client_for_fd(collections[1]);
    if(!c)
    {
        rply = ERR_NOSUCHNICK(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(!has_this_channel(collections[2]))
    {
        rply = ERR_NOSUCHCHANNEL(client->get_nick_name(), collections[2]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(!is_member_of_channel(client, collections[2]))
    {
        rply = ERR_NOTONCHANNEL(client->get_nick_name(), collections[2]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(is_member_of_channel(c, collections[2]))
    {
        rply = ERR_USERONCHANNEL(client->get_nick_name(), collections[1], collections[2]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    chnnls[collections[2]]->add_invited_lists(c);
    rply = RPL_INVITING(client->get_nick_name(), collections[1], collections[2]);
    send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
    std::string prefix = ":" + client->get_nick_name() + "!" + client->get_user_name() + "@localhost";
    rply = prefix+" INVITE "+ collections[1]+" :"+collections[2]+"\r\n";
    send(c->get_client_fd(), rply.c_str(), rply.length(), 0);
}

void Server::mode_parser(Client *client, std::vector<std::string> collections)
{
    std::string rply;
    bool sign = true;
    size_t order_count = 3;
    std::string str = collections[2];
    for(size_t i = 0; i < str.length(); i++)
    {
        if(str[i] != '+' && str[i] != '-' && str[i] != 'i' && str[i] != 't' && str[i] != 'k' && str[i] != 'l' && str[i] != 'o')
        {
            rply = ERR_UNKNOWNMODE(client->get_nick_name(), str);
            send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
            return;
        }
        if(str[i] == '+')
            sign = true;
        if(str[i] == '-')
            sign = false;
        if(str[i] == 'k')
        {
            if(sign)
            {
                if (order_count < collections.size())
                    chnnls[collections[1]]->set_channel_key(collections[order_count]);
                order_count++;
            } 
            else
                chnnls[collections[1]]->set_channel_key("");
        }
        if(str[i] == 'l')
        {
            if(sign)
            {
                if (order_count < collections.size())
                    chnnls[collections[1]]->set_limit_user(std::atoi(collections[order_count].c_str()));
                order_count++;
            } 
            else
                chnnls[collections[1]]->set_limit_user(0);
        }
        if(str[i] == 'o')
        {
            Client *c = get_client_for_fd(collections[order_count]);
            if(c)
                chnnls[collections[1]]->set_operator(c,sign);
            order_count++;
        }
        if(str[i] == 'i')
            chnnls[collections[1]]->set_only_invite(sign);
        if(str[i] == 't')
            chnnls[collections[1]]->set_is_restricted_topic(sign);
    }
}

void Server::handle_mode(Client *client, std::vector<std::string> collections)
{
    std::string rply;
    if(collections.size()<2)
    {
        rply = ERR_NEEDMOREPARAMS(client->get_nick_name(), "MODE");
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(collections.size() == 2)
    {
        rply = RPL_CHANNELMODEIS(client->get_nick_name(), collections[1], chnnls[collections[1]]->get_modes());
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(!has_this_channel(collections[1]))
    {
        rply = ERR_NOSUCHCHANNEL(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(!is_member_of_channel(client, collections[1]))
    {
        rply = ERR_NOTONCHANNEL(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(!chnnls[collections[1]]->is_operator(client))
    {
        rply = ERR_CHANOPRIVSNEEDED(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    mode_parser(client, collections);
    std::string prefix = ":" + client->get_nick_name() + "!" + client->get_user_name() + "@localhost ";
    std::string msg;
    for(size_t i = 0;i < collections.size();i++)
    {
        msg += collections[i];
        if(i != collections.size() -1)
            msg += " ";
    }
    msg += "\r\n";
    chnnls[collections[1]]->broadcast_msg(msg, -1);
}
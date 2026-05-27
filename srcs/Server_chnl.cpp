#include "../libs/Server.hpp"
#include "../libs/ProtocolMessages.hpp"
#include <cctype>

static bool parse_channel_limit(std::string raw_limit, unsigned int &limit)
{
    if (raw_limit.empty())
        return false;
    for (size_t i = 0; i < raw_limit.length(); ++i)
    {
        if (!std::isdigit(static_cast<unsigned char>(raw_limit[i])))
            return false;
    }
    unsigned long parsed = std::strtoul(raw_limit.c_str(), NULL, 10);
    limit = static_cast<unsigned int>(parsed);
    return parsed > 0 && static_cast<unsigned long>(limit) == parsed;
}

void Server::handle_join(Client *client, std::vector<std::string> collections)
{
    std::string nick = client->get_nick_name().empty() ? "*" : client->get_nick_name();
    if(collections.size() < 2)
    {
        std::string rply = ERR_NEEDMOREPARAMS(nick, "JOIN");
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        return;
    }
    if(!client->get_is_registered())
    {
        std::string rply = ERR_NOTREGISTERED(nick);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout<<rply;
        return;
    }
    if(collections[1][0] != '#' && collections[1][0] != '&')
    {
        std::string rply = ERR_NOSUCHCHANNEL(nick, collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        return;
    }

    bool created_channel = false;
    if(chnnls.find(collections[1]) == chnnls.end())
    {
        Channel *new_ch = new Channel(collections[1]);
        new_ch->add_members(client, true);
        std::cout<<"[ CHANNEL ] "<<client->get_nick_name()<<" created "<<collections[1]<<" channel!"<<std::endl;
        chnnls[collections[1]] = new_ch;
        created_channel = true;
    }
    else
    {
        Channel *ch = chnnls[collections[1]];
        if (ch->get_only_invite() && !ch->is_user_invited(client))
        {
            std::string rply = ERR_INVITEONLYCHAN(nick, collections[1]);
            send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
            std::cout<<rply;
            return;
        }
        if (!ch->get_channel_key().empty())
        {
            if (collections.size() < 3 || collections[2] != ch->get_channel_key())
            {
                std::string rply = ERR_BADCHANNELKEY(nick, collections[1]);
                send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
                std::cout<<rply;
                return;
            }
        }
        if(ch->get_is_channel_limited() && ch->get_member_count() >= ch->get_limit_user())
        {
            std::string rply = ERR_CHANNELISFULL(nick, collections[1]);
            send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
            std::cout<<rply;
            return;
        }
        ch->add_members(client, false);
        std::cout<<"[ CHANNEL ]"<<client->get_nick_name()<<" joined "<<collections[1]<<" channel!"<<std::endl;
    }

    std::string msg = ":"+client->get_nick_name()+"!"+client->get_user_name()+"@localhost JOIN "+collections[1]+"\r\n";
    chnnls[collections[1]]->broadcast_msg(msg, -1);

    if (created_channel)
    {
        msg = ":ircserv MODE " + collections[1] + " +o " + client->get_nick_name() + "\r\n";
        chnnls[collections[1]]->broadcast_msg(msg, -1);
    }

    msg = RPL_CHANNELMODEIS(nick, collections[1], chnnls[collections[1]]->get_modes());
    send(client->get_client_fd(), msg.c_str(), msg.length(), 0);
    msg = RPL_CREATIONTIME(nick, collections[1], chnnls[collections[1]]->get_creation_time());
    send(client->get_client_fd(), msg.c_str(), msg.length(), 0);

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
        std::cout << rply;
        return;
    }
    if(!has_this_channel(collections[1]))
    {
        std::string rply = ERR_NOSUCHCHANNEL(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
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
        msg = prefix + " PART "+collections[1] + " "+collections[2]+"\r\n";
    else
        msg = prefix + " PART "+collections[1] + " :"+client->get_nick_name()+"\r\n";
    chnnls[collections[1]]->broadcast_msg(msg, -1);
    chnnls[collections[1]]->remove_members(client);
    std::cout<<client->get_nick_name()<<" left from "<<collections[1]<<" channel!";
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
        std::cout << rply;
        return;
    }
    if(!has_this_channel(collections[1]))
    {
        rply = ERR_NOSUCHCHANNEL(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        return;
    }
    if(!is_member_of_channel(client, collections[1]))
    {
        rply = ERR_NOTONCHANNEL(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout<<rply;
        return;
    }
    if(!chnnls[collections[1]]->is_operator(client))
    {
        rply = ERR_CHANOPRIVSNEEDED(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout<<"Not channel operator"<<std::endl;
        return;
    }
    Client *will_thrown = get_client_for_fd(collections[2]);
    if(!will_thrown || !is_member_of_channel(will_thrown, collections[1]))
    {
        rply = ERR_USERNOTINCHANNEL(client->get_nick_name(), collections[2], collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout<<rply;
        return;
    }
    std::string prefix = ":" + client->get_nick_name() + "!" + client->get_user_name() + "@localhost";
    std::string msg;
    if(collections.size()>3 && collections[3].size()>0)
    {
        std::string reason = collections[3];
        if(!reason.empty() && reason[0] == ':')
            reason.erase(0, 1);
        if(reason.empty())
            reason = client->get_nick_name();
        msg = prefix + " KICK "+collections[1] + " "+collections[2]+ " :"+reason+"\r\n";
    }
    else
        msg = prefix + " KICK "+collections[1] + " "+collections[2]+ " :"+client->get_nick_name()+"\r\n";
    chnnls[collections[1]]->broadcast_msg(msg, -1);
    chnnls[collections[1]]->remove_members(will_thrown);
    std::cout<<"[ CHANNEL ] "<<client->get_nick_name()<<" kicked "<<collections[2]<<" from "<<collections[1]<<std::endl;
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
        std::cout << rply;
        return;
    }
    if(!has_this_channel(collections[1]))
    {
        rply = ERR_NOSUCHCHANNEL(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        return;
    }
    if(!is_member_of_channel(client, collections[1]))
    {
        rply = ERR_NOTONCHANNEL(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout<<rply;
        return;
    }
    if(chnnls[collections[1]]->get_is_restricted_topic() && !chnnls[collections[1]]->is_operator(client))
    {
        rply = ERR_CHANOPRIVSNEEDED(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout<<rply;
        return;
    }
    if(collections.size() == 2)
    {
        if(chnnls[collections[1]]->get_topic().empty())
            rply = RPL_NOTOPIC(client->get_nick_name(), collections[1]); 
        else
            rply = RPL_TOPIC(client->get_nick_name(), collections[1], chnnls[collections[1]]->get_topic());
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return ;
    }
    std::string old_topic = chnnls[collections[1]]->get_topic();
    if(collections[2].empty())
        chnnls[collections[1]]->set_topic("");
    else
        chnnls[collections[1]]->set_topic(collections[2]);
    std::cout<<"[ CHANNEL ] "<<client->get_nick_name()<<" set topic: "<<old_topic<<" -> "<<chnnls[collections[1]]->get_topic()<<std::endl;
    std::string prefix = ":" + client->get_nick_name() + "!" + client->get_user_name() + "@localhost";
    std::string msg = prefix + " TOPIC " + collections[1] + " " + collections[2]+"\r\n";
    chnnls[collections[1]]->broadcast_msg(msg, -1);
}

void Server::handle_invite(Client *client, std::vector<std::string> collections)
{
    std::string rply;
    if(collections.size()<3)
    {
        rply = ERR_NEEDMOREPARAMS(client->get_nick_name(), "INVITE");
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        return;
    }
    Client *c = get_client_for_fd(collections[1]);
    if(!c)
    {
        rply = ERR_NOSUCHNICK(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout<<rply;
        return;
    }
    if(!has_this_channel(collections[2]))
    {
        rply = ERR_NOSUCHCHANNEL(client->get_nick_name(), collections[2]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        return;
    }
    if(!is_member_of_channel(client, collections[2]))
    {
        rply = ERR_NOTONCHANNEL(client->get_nick_name(), collections[2]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout<<rply;
        return;
    }
    if(!chnnls[collections[2]]->is_operator(client))
    {
        rply = ERR_CHANOPRIVSNEEDED(client->get_nick_name(), collections[2]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout<<rply;
        return;
    }
    if(is_member_of_channel(c, collections[2]))
    {
        rply = ERR_USERONCHANNEL(client->get_nick_name(), collections[1], collections[2]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout<<rply;
        return;
    }
    chnnls[collections[2]]->add_invited_lists(c);
    std::cout<<"[ CHANNEL ] "<<client->get_nick_name()<<" invited "<<collections[1]<<" to "<<collections[2]<<" channel!"<<std::endl;
    rply = RPL_INVITING(client->get_nick_name(), collections[1], collections[2]);
    send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
    std::string prefix = ":" + client->get_nick_name() + "!" + client->get_user_name() + "@localhost";
    std::string chan = collections[2];
    if(!chan.empty() && chan[0] == ':')
        chan.erase(0, 1);
    rply = prefix+" INVITE "+ collections[1]+" :"+chan+"\r\n";
    send(c->get_client_fd(), rply.c_str(), rply.length(), 0);
}

bool Server::mode_parser(Client *client, std::vector<std::string> collections, bool &changed)
{
    std::string rply;
    bool sign = true;
    size_t order_count = 3;
    std::string str = collections[2];
    changed = false;
    if(str.empty() || (str[0] != '+' && str[0] != '-'))
    {
        rply = ERR_UNKNOWNMODE(client->get_nick_name(), str);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        return false;
    }
    for(size_t i = 0; i < str.length(); i++)
    {
        if(str[i] != '+' && str[i] != '-' && str[i] != 'i' && str[i] != 't' && str[i] != 'k' && str[i] != 'l' && str[i] != 'o')
        {
            rply = ERR_UNKNOWNMODE(client->get_nick_name(), str);
            send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
            std::cout<<rply;
            return false;
        }
        if(str[i] == '+')
            sign = true;
        if(str[i] == '-')
            sign = false;
        if(str[i] == 'k')
        {
            if(sign)
            {
                if (order_count >= collections.size())
                {
                    rply = ERR_NEEDMOREPARAMS(client->get_nick_name(), "MODE");
                    send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
                    std::cout << rply;
                    return false;
                }
                if (chnnls[collections[1]]->get_channel_key() != collections[order_count])
                    changed = true;
                chnnls[collections[1]]->set_channel_key(collections[order_count]);
                order_count++;
            }
            else
            {
                if (!chnnls[collections[1]]->get_channel_key().empty())
                    changed = true;
                chnnls[collections[1]]->set_channel_key("");
            }
        }
        if(str[i] == 'l')
        {
            if(sign)
            {
                unsigned int new_limit = 0;
                if (order_count >= collections.size()
                    || !parse_channel_limit(collections[order_count], new_limit))
                {
                    rply = ERR_NEEDMOREPARAMS(client->get_nick_name(), "MODE");
                    send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
                    std::cout << rply;
                    return false;
                }
                if (!chnnls[collections[1]]->get_is_channel_limited()
                    || chnnls[collections[1]]->get_limit_user() != new_limit)
                    changed = true;
                chnnls[collections[1]]->set_is_channel_limited(true);
                chnnls[collections[1]]->set_limit_user(new_limit);
                order_count++;
            } 
            else
            {
                if (chnnls[collections[1]]->get_is_channel_limited()
                    || chnnls[collections[1]]->get_limit_user() != 0)
                    changed = true;
                chnnls[collections[1]]->set_is_channel_limited(false);
                chnnls[collections[1]]->set_limit_user(0);
            }
        }
        if(str[i] == 'o')
        {
            if (order_count >= collections.size())
            {
                rply = ERR_NEEDMOREPARAMS(client->get_nick_name(), "MODE");
                send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
                std::cout << rply;
                return false;
            }
            Client *c = get_client_for_fd(collections[order_count]);
            if(!c)
            {
                rply = ERR_NOSUCHNICK(client->get_nick_name(), collections[order_count]);
                send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
                std::cout << rply;
                return false;
            }
            if(!chnnls[collections[1]]->is_member(c))
            {
                rply = ERR_USERNOTINCHANNEL(client->get_nick_name(), collections[order_count], collections[1]);
                send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
                std::cout << rply;
                return false;
            }
            if(chnnls[collections[1]]->is_operator(c) != sign)
                changed = true;
        
            chnnls[collections[1]]->set_operator(c, sign);
            order_count++;
        }
        if(str[i] == 'i')
        {
            if (chnnls[collections[1]]->get_only_invite() != sign)
                changed = true;
            chnnls[collections[1]]->set_only_invite(sign);
        }
        if(str[i] == 't')
        {
            if (chnnls[collections[1]]->get_is_restricted_topic() != sign)
                changed = true;
            chnnls[collections[1]]->set_is_restricted_topic(sign);
        }
    }
    return true;
}

void Server::handle_mode(Client *client, std::vector<std::string> collections)
{
    std::string rply;
    if(collections.size()<2)
    {
        rply = ERR_NEEDMOREPARAMS(client->get_nick_name(), "MODE");
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        return;
    }
    if (collections[1][0] != '#' && collections[1][0] != '&')
        return;
    if(!has_this_channel(collections[1]))
    {
        rply = ERR_NOSUCHCHANNEL(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        return;
    }
    if(collections.size() == 2)
    {
        rply = RPL_CHANNELMODEIS(client->get_nick_name(), collections[1], chnnls[collections[1]]->get_modes());
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(!is_member_of_channel(client, collections[1]))
    {
        rply = ERR_NOTONCHANNEL(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout<<rply;
        return;
    }
    if(collections.size() == 3 && !collections[2].empty() && collections[2][0] != '+' && collections[2][0] != '-')
    {
        if(collections[2] == "b")
        {
            rply = RPL_ENDOFBANLIST(client->get_nick_name(), collections[1]);
            send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
            return;
        }
        else
        {
            rply = ERR_UNKNOWNMODE(client->get_nick_name(), collections[2]);
            send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
            return;
        }
    }

    if(!chnnls[collections[1]]->is_operator(client))
    {
        rply = ERR_CHANOPRIVSNEEDED(client->get_nick_name(), collections[1]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout<<rply;
        return;
    }
    bool changed = false;
    if(!mode_parser(client, collections, changed))
        return;
    std::cout<<"[ MODE    ] "<<client->get_nick_name()<<" applied: ";
    for(size_t i = 0;i < collections.size();i++)
    {
        if(!collections[i].empty())
            std::cout<<collections[i]<<" ";
    }
    std::cout<<std::endl;
    if(!changed)
        return;
    std::string prefix = ":" + client->get_nick_name() + "!" + client->get_user_name() + "@localhost ";
    std::string msg = prefix;
    for(size_t i = 0;i < collections.size();i++)
    {
        msg += collections[i];
        if(i != collections.size() -1)
            msg += " ";
    }
    msg += "\r\n";
    chnnls[collections[1]]->broadcast_msg(msg, -1);
}

void Server::handle_who(Client *client, std::vector<std::string> collections)
{
    std::string nick = client->get_nick_name().empty() ? "*" : client->get_nick_name();
    std::string mask = collections.size() >= 2 ? collections[1] : "*";
    std::string rply;

    if (mask.empty())
        mask = "*";

    if ((mask[0] == '#' || mask[0] == '&') && has_this_channel(mask))
    {
        for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); ++it)
        {
            Client *who_client = it->second;
            if (is_member_of_channel(who_client, mask))
              {
                std::string flags = "H";
                if (chnnls[mask]->is_operator(who_client))
                    flags += "@";
                std::string realname = who_client->get_real_name().empty() ? who_client->get_nick_name() : who_client->get_real_name();
                rply = RPL_WHOREPLY(nick, mask, who_client->get_user_name(), "localhost",
                                    "ircserv", who_client->get_nick_name(), flags, realname);
                send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
            }
        }  
    }

    rply = RPL_ENDOFWHO(nick, mask);
    send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
}

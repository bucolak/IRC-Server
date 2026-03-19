#include "../libs/Server.hpp"
#include "../libs/ProtocolMessages.hpp"

Server::Server(int port, std::string password)
{
    this->port = port;
    this->password = password;
}

std::vector<pollfd> Server::get_fds()
{
    return fds;
}

std::string Server::get_password()
{
    return password;
}

int Server::get_server_fd()
{
    return server_fd;
}

int Server::get_port()
{
    return port;
}

void Server::init_server()
{
    this->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1)
        throw std::runtime_error("Error: Socket could not be created!");
    
    int optval = 1;
    if((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) == -1)
        throw std::runtime_error("Error: setsockopt!");

    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    sockaddr_in addr = {};
    addr.sin_family= AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_fd, (sockaddr *)&addr, sizeof(addr)) == -1)
        throw std::runtime_error("Error: bind");

    if(listen(server_fd, 10) == -1)
        throw std::runtime_error("Error: listen");

    pollfd server_poll = {};
    server_poll.fd = server_fd;
    server_poll.events = POLLIN;
    fds.push_back(server_poll);
}

void Server::find_first_msg(int wc)
{
    size_t i;
    std::string msg = clients[fds[wc].fd]->get_buffer();
    while((i = msg.find("\r\n")) != std::string::npos)
    {
        std::string new_msg = msg.substr(0, i + 2);
        clients[fds[wc].fd]->set_buffer(msg.erase(0, i + 2));
        parse_msg(new_msg.substr(0,i));
        // TESTING
        // std::cout<<"Client["<<fds[wc].fd<<"] : "<<new_msg<<std::endl;
    }
}

void Server::parse_msg(std::string str)
{
    std::vector<std::string> collection;
    size_t i;
    std::string before_delimiter;
    std::string after_delimiter;
    std::string new_str;
    if((i = str.find(":")) != std::string::npos)
    {
        before_delimiter = str.substr(0, i);
        after_delimiter = str.substr(i, str.length() - i);
        std::stringstream ss(before_delimiter);
        while(ss >> new_str)
        {
            collection.push_back(new_str);
        }
        collection.push_back(after_delimiter);
    }
    else
    {
        std::stringstream ss(str);
        while(ss >> new_str)
        {
            collection.push_back(new_str);
        }
    }
    //TESTING
    // std::cout<<"---------"<<std::endl;
    // for (size_t k = 0; k < collection.size(); k++) {
    //     std::cout << "c[" << k << "] : " << collection[k] << std::endl;
    // }
    // std::cout<<"---------"<<std::endl;
}

// void Server::handle_cmds()
// {
//     std::string funcs_names[8] = { "PASS",
//                                 "NICK",
//                                 "USER",
//                                 "JOIN",
//                                 "PART",
//                                 "KICK"
//                                 "QUIT",
//                                 "PRIVMSG",
//                                 "PING"};
//     void (Server)

// }

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
    for(int i = 0; i < nick.length(); i++)
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
    if (client->get_is_authenticated() && 
        !client->get_nick_name().empty() && 
        !client->get_user_name().empty())
        {
            client->set_is_registered(true);
            std::string rply = RPL_WELCOME(client->get_nick_name(), client->get_user_name(), "ANLAMADIM");
            send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        }
}

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
    {
        chnnls[collections[1]]->add_members(client, false);
    }
    // kullanıcnın kanala girdiğine dair kanaldaki herkese mesaj
    std::string msg = ":"+client->get_nick_name()+"!"+client->get_user_name()+"@localhost JOIN :"+collections[1]+"\r\n";
    chnnls[collections[1]]->broadcast_msg(msg, -1);

    // yeni kullanıcıya topic bilgisi gönderme
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

    // yeni kullanıcıya kanaldaki kişilerin listesi gönderilir
    msg = RPL_NAMREPLY(client->get_nick_name(), collections[1], chnnls[collections[1]]->get_members());
    send(client->get_client_fd(), msg.c_str(), msg.length(), 0);
    msg = RPL_ENDOFNAMES(client->get_nick_name(), collections[1]); // bu da listenin bittiğine dair bir mesaj imiş
    send(client->get_client_fd(), msg.c_str(), msg.length(), 0);
}

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

void Server::handle_privmsg(Client *client, std::vector<std::string> collections)
{
    if(collections.size() < 2)
    {
        std::string rply = ERR_NORECIPIENT(client->get_nick_name());
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    if(collections[2].empty())
    {
        std::string rply = ERR_NOTEXTTOSEND(client->get_nick_name());
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        return;
    }
    std::string prefix = ":" + client->get_nick_name() + "!" + client->get_user_name() + "@localhost"; // // Mesajı gönderen kişinin kimlik bilgisi (Prefix)
    std::string all_msg = prefix + " PRIVMSG " + collections[1] + " :" + collections[2] + "\r\n";;
    if(collections[1][0] == '#' || collections[1][0] == '&')
    {
        if(!has_this_channel(collections[1]))
        {
            std::string rply = ERR_NOSUCHCHANNEL(client->get_nick_name(), collections[1]);
            send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
            return;
        }
        if(!is_member_of_channel(client, collections[1]))
        {
            std::string rply = ERR_CANNOTSENDTOCHAN(client->get_nick_name(), collections[1]);
            send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
            return;
        }
        chnnls[collections[1]]->broadcast_msg(all_msg, client->get_client_fd());
    }
    else
    {
        Client *c = get_client_for_fd(collections[1]);
        if(!c)
        {
            std::string rply = ERR_NOSUCHNICK(client->get_nick_name(), collections[1]);
            send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
            return;
        }
        send(c->get_client_fd(), all_msg.c_str(), all_msg.length(), 0);
    }
}

void Server::execute_server()
{
    while(1)
    {
        if(poll(&fds[0], fds.size(), -1) == -1) // -1 sonsuza dek bekle demektir, CPU'yu %0 yorar.
            throw std::runtime_error("Error: poll!");
        
        for (size_t i = 0; i < fds.size(); i++)
        {
            if(fds[i].fd == server_fd)
            {
                if(fds[i].revents & POLLIN)
                {
                    sockaddr_in client_addr = {};
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(server_fd, (sockaddr *)&client_addr, &client_len);
                    fcntl(client_fd, F_SETFL, O_NONBLOCK);
                    if(client_fd != -1)
                    {
                        std::cout<<"We have a new clint! Fd : "<<client_fd<<std::endl;
                        Client* new_client = new Client(client_fd);
                        clients[client_fd] = new_client;

                        pollfd client_poll = {};
                        client_poll.fd = client_fd;
                        client_poll.events = POLLIN;
                        fds.push_back(client_poll);
                    }
                }
            }
            else
            {
                if(fds[i].revents & POLLIN)
                {
                    char messg[1000] = {};
                    int result_recv = recv(fds[i].fd, messg, sizeof(messg), 0);
                    if(result_recv > 0)
                    {
                        clients[fds[i].fd]->buffer_append(messg);
                        find_first_msg(i);
                    }
                    else if(result_recv == 0)
                    {
                        std::cout<<"Client went"<<std::endl;

                        delete clients[fds[i].fd];
                        fds.erase(fds.begin() + i);
                        close(fds[i].fd);
                        i--;
                    }
                    else
                        throw std::runtime_error("Error: recv");
                }
            }
        }
    }
}

Server::~Server() 
{
    for(std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
        delete it->second;
    if(server_fd != -1)
        close(server_fd);
}
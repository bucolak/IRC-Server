#include "../libs/ProtocolMessages.hpp"
#include "../libs/Server.hpp"
#include <set>

static std::vector<std::string> split_privmsg_targets(const std::string &targets, bool &malformed)
{
  std::vector<std::string> result;
  malformed = false;

  if (targets.empty() || targets[0] == ',' || targets[targets.size() - 1] == ',')
  {
    malformed = true;
    return result;
  }
  size_t start = 0;
  while (start <= targets.length())
  {
    size_t comma = targets.find(',', start);
    std::string token = (comma == std::string::npos) ? targets.substr(start) : targets.substr(start, comma - start);
    if (token.empty())
    {
      malformed = true;
      return result;
    }
    result.push_back(token);
    if (comma == std::string::npos)
      break;
    start = comma + 1;
  }
  return result;
}

static std::string extract_text(const std::vector<std::string> &collections)
{
  if (collections.size() < 3)
    return "";
  std::string text = collections[2];
  if (!text.empty() && text[0] == ':')
    text.erase(0, 1);
  return text;
}

void Server::handle_privmsg(Client *client, std::vector<std::string> collections)
{
  if (collections.size() < 2 || collections[1].empty())
  {
    std::string rply = ERR_NORECIPIENT(client->get_nick_name());
    send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
    std::cout << rply;
    return;
  }
  std::string text = extract_text(collections);
  if (text.empty())
  {
    std::string rply = ERR_NOTEXTTOSEND(client->get_nick_name());
    send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
    std::cout << rply;
    return;
  }
  bool malformed = false;
  std::vector<std::string> targets = split_privmsg_targets(collections[1], malformed);
  if (malformed || targets.empty())
  {
    std::string rply = ERR_NORECIPIENT(client->get_nick_name());
    send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
    std::cout << rply;
    return;
  }

  std::string prefix = ":" + client->get_nick_name() + "!" + client->get_user_name() + "@localhost";

  for (size_t i = 0; i < targets.size(); ++i)
  {
    std::string all_msg = prefix + " PRIVMSG " + targets[i] + " :" + text + "\r\n";

    if (targets[i][0] == '#' || targets[i][0] == '&')
    {
      if (!has_this_channel(targets[i]))
      {
        std::string rply = ERR_NOSUCHCHANNEL(client->get_nick_name(), targets[i]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        continue;
      }
      if (!is_member_of_channel(client, targets[i]))
      {
        std::string rply = ERR_CANNOTSENDTOCHAN(client->get_nick_name(), targets[i]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        continue;
      }
      chnnls[targets[i]]->broadcast_msg(all_msg, client->get_client_fd());
    }
    else
    {
      Client *c = get_client_for_fd(targets[i]);
      if (!c)
      {
        std::string rply = ERR_NOSUCHNICK(client->get_nick_name(), targets[i]);
        send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
        std::cout << rply;
        continue;
      }
      send(c->get_client_fd(), all_msg.c_str(), all_msg.length(), 0);
    }
  }
}

void Server::handle_quit(Client *client, std::vector<std::string> collections) 
{
  std::string nick = client->get_nick_name().empty() ? "*" : client->get_nick_name();
  std::string user = client->get_user_name().empty() ? "*" : client->get_user_name();
  std::string prefix = ":" + nick + "!" + user + "@localhost";
  std::string msg;
  if (collections.size() >= 2)
    msg = prefix + " QUIT :Quit: " + collections[1] + "\r\n";
  else
    msg = prefix + " QUIT :Quit: " + nick + "\r\n";

  std::set<int> notify_fds;
  for (std::map<std::string, Channel *>::iterator it = chnnls.begin(); it != chnnls.end(); ++it) 
  {
    if (is_member_of_channel(client, it->first)) 
    {
      std::vector<int> member_fds = it->second->get_member_fds();
      for (size_t i = 0; i < member_fds.size(); i++) 
      {
        if (member_fds[i] != client->get_client_fd())
          notify_fds.insert(member_fds[i]);
      }
    }
  }
  for (std::set<int>::iterator it = notify_fds.begin(); it != notify_fds.end();
       ++it) {
    send(*it, msg.c_str(), msg.length(), 0);
  }

  std::string error_msg = "ERROR :Closing Link: " + nick + " (Client Quit)\r\n";
  send(client->get_client_fd(), error_msg.c_str(), error_msg.length(), 0);

  std::cout <<"Client quit! [FD: "<<client->get_client_fd()<<" , NICK: "<<client->get_nick_name()<<"]"<<std::endl;
  
  client->set_quit_requested(true);
}

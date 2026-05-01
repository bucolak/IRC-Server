#include "../libs/ProtocolMessages.hpp"
#include "../libs/Server.hpp"
#include "../libs/DebugLogger.hpp"
#include <set>

void Server::handle_privmsg(Client *client,
                            std::vector<std::string> collections) {
  if (collections.size() < 2) {
    std::string rply = ERR_NORECIPIENT(client->get_nick_name());
    send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
    return;
  }
  if (collections[2].empty()) {
    std::string rply = ERR_NOTEXTTOSEND(client->get_nick_name());
    send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
    return;
  }
  std::string prefix =
      ":" + client->get_nick_name() + "!" + client->get_user_name() +
      "@localhost"; // // Mesajı gönderen kişinin kimlik bilgisi (Prefix)
  std::string all_msg =
      prefix + " PRIVMSG " + collections[1] + " :" + collections[2] + "\r\n";
  ;
  if (collections[1][0] == '#' || collections[1][0] == '&') {
    if (!has_this_channel(collections[1])) {
      std::string rply =
          ERR_NOSUCHCHANNEL(client->get_nick_name(), collections[1]);
      send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
      return;
    }
    if (!is_member_of_channel(client, collections[1])) {
      std::string rply =
          ERR_CANNOTSENDTOCHAN(client->get_nick_name(), collections[1]);
      send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
      return;
    }
    chnnls[collections[1]]->broadcast_msg(all_msg, client->get_client_fd());
  } else {
    Client *c = get_client_for_fd(collections[1]);
    if (!c) {
      std::string rply =
          ERR_NOSUCHNICK(client->get_nick_name(), collections[1]);
      send(client->get_client_fd(), rply.c_str(), rply.length(), 0);
      return;
    }
    send(c->get_client_fd(), all_msg.c_str(), all_msg.length(), 0);
  }
}

void Server::handle_quit(Client *client, std::vector<std::string> collections) {
  // Already processing a quit for this client
  if (client->get_quit_requested())
    return;

  std::vector<std::string> deleted;
  std::string nick =
      client->get_nick_name().empty() ? "*" : client->get_nick_name();
  std::string user =
      client->get_user_name().empty() ? "*" : client->get_user_name();
  std::string prefix = ":" + nick + "!" + user + "@localhost";
  std::string msg;
  if (collections.size() >= 2)
    msg = prefix + " QUIT :Quit: " + collections[1] + "\r\n";
  else
    msg = prefix + " QUIT :Quit: " + nick + "\r\n";

  // #region agent log
  {
    std::ostringstream oss;
    oss << "fd=" << client->get_client_fd() << ", nick=" << nick
        << ", user=" << user << ", argc=" << collections.size();
    debug_log("srcs/Server_messg.cpp:66", "pre-fix", "H1|H2",
              "handle_quit entered", oss.str());
  }
  // #endregion

  // Notify other channel members
  std::set<int> notify_fds;
  for (std::map<std::string, Channel *>::iterator it = chnnls.begin();
       it != chnnls.end(); ++it) {
    if (is_member_of_channel(client, it->first)) {
      deleted.push_back(it->first);
      std::vector<int> member_fds = it->second->get_member_fds();
      for (size_t i = 0; i < member_fds.size(); i++) {
        if (member_fds[i] != client->get_client_fd())
          notify_fds.insert(member_fds[i]);
      }
    }
  }
  for (std::set<int>::iterator it = notify_fds.begin(); it != notify_fds.end();
       ++it) {
    ssize_t notify_result = send(*it, msg.c_str(), msg.length(), 0);
    // #region agent log
    {
      std::ostringstream oss;
      oss << "target_fd=" << *it << ", bytes=" << msg.length()
          << ", send_result=" << notify_result;
      debug_log("srcs/Server_messg.cpp:85", "pre-fix", "H3",
                "broadcast quit result", oss.str());
    }
    // #endregion
  }

  // Remove client from channels
  for (std::vector<std::string>::iterator it = deleted.begin();
       it != deleted.end(); ++it) {
    chnnls[*it]->remove_members(client);
    if (chnnls[*it]->get_members().empty()) {
      Channel *tmp = chnnls[*it];
      chnnls.erase(*it);
      delete tmp;
    }
  }

  // Send ERROR to client so its terminal sees the connection closing
  std::string error_msg = "ERROR :Closing Link: " + nick + " (Client Quit)\r\n";
  ssize_t error_send =
      send(client->get_client_fd(), error_msg.c_str(), error_msg.length(), 0);
  // #region agent log
  {
    std::ostringstream oss;
    oss << "fd=" << client->get_client_fd() << ", bytes=" << error_msg.length()
        << ", send_result=" << error_send;
    debug_log("srcs/Server_messg.cpp:107", "pre-fix", "H1|H3",
              "sent closing error to quitting client", oss.str());
  }
  // #endregion

  // Mark for removal — actual fd/memory cleanup happens in execute_server
  client->set_quit_requested(true);
  int shutdown_result = shutdown(client->get_client_fd(), SHUT_WR);
  // #region agent log
  {
    std::ostringstream oss;
    oss << "fd=" << client->get_client_fd()
        << ", shutdown_result=" << shutdown_result
        << ", quit_requested=" << client->get_quit_requested();
    debug_log("srcs/Server_messg.cpp:117", "pre-fix", "H1|H2|H3",
              "half-closed quitting client socket", oss.str());
  }
  // #endregion
}

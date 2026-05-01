#include "../libs/Server.hpp"
#include "../libs/ProtocolMessages.hpp"
#include "../libs/DebugLogger.hpp"

Server::Server(int port, std::string password) {
  this->port = port;
  this->password = password;
}

std::vector<pollfd> Server::get_fds() { return fds; }

std::string Server::get_password() { return password; }

int Server::get_server_fd() { return server_fd; }

int Server::get_port() { return port; }

void Server::init_server() {
  this->server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1)
    throw std::runtime_error("Error: Socket could not be created!");

  int optval = 1;
  if ((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval,
                  sizeof(optval))) == -1)
    throw std::runtime_error("Error: setsockopt!");

  fcntl(server_fd, F_SETFL, O_NONBLOCK);

  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_fd, (sockaddr *)&addr, sizeof(addr)) == -1)
    throw std::runtime_error("Error: bind");

  if (listen(server_fd, 10) == -1)
    throw std::runtime_error("Error: listen");

  pollfd server_poll = {};
  server_poll.fd = server_fd;
  server_poll.events = POLLIN;
  fds.push_back(server_poll);
}

void Server::find_first_msg(int wc) {
  // Save the fd upfront — handle_quit may mark the client for removal mid-loop
  int fd = fds[wc].fd;
  size_t i;
  // IRC standard delimiter is \r\n, but accept bare \n to support simple
  // clients (e.g. netcat without -C).
  while (clients.count(fd) && !clients[fd]->get_quit_requested()) {
    std::string msg = clients[fd]->get_buffer();
    if ((i = msg.find('\n')) == std::string::npos)
      break;
    std::string line = msg.substr(0, i);
    if (!line.empty() && line[line.length() - 1] == '\r')
      line.erase(line.length() - 1);
    clients[fd]->set_buffer(msg.erase(0, i + 1));
    // #region agent log
    {
      std::ostringstream oss;
      oss << "fd=" << fd << ", line=" << line;
      debug_log("srcs/Server.cpp:62", "pre-fix", "H4",
                "parsed full client line", oss.str());
    }
    // #endregion
    parse_msg(line, clients[fd]);
  }
}

void Server::parse_msg(std::string str, Client *client) {
  std::vector<std::string> collection;
  size_t i;
  std::string before_delimiter;
  std::string after_delimiter;
  std::string new_str;
  if ((i = str.find(":")) != std::string::npos) {
    before_delimiter = str.substr(0, i);
    after_delimiter = str.substr(i, str.length() - i);
    std::stringstream ss(before_delimiter);
    while (ss >> new_str) {
      collection.push_back(new_str);
    }
    collection.push_back(after_delimiter);
  } else {
    std::stringstream ss(str);
    while (ss >> new_str) {
      collection.push_back(new_str);
    }
  }
  // TESTING
  //  std::cout<<"---------"<<std::endl;
  //  for (size_t k = 0; k < collection.size(); k++) {
  //      std::cout << "c[" << k << "] : " << collection[k] << std::endl;
  //  }
  //  std::cout<<"---------"<<std::endl;

  if (collection.empty())
    return;

  std::string cmd = collection[0];
  if (cmd == "PASS")
    handle_pass(client, collection);
  else if (cmd == "NICK")
    handle_nick(client, collection);
  else if (cmd == "USER")
    handle_user(client, collection);
  else if (cmd == "JOIN")
    handle_join(client, collection);
  else if (cmd == "PRIVMSG")
    handle_privmsg(client, collection);
  else if (cmd == "PART")
    handle_part(client, collection);
  else if (cmd == "QUIT")
    handle_quit(client, collection);
  else if (cmd == "KICK")
    handle_kick(client, collection);
  else if (cmd == "TOPIC")
    handle_topic(client, collection);
  else if (cmd == "INVITE")
    handle_invite(client, collection);
  else if (cmd == "MODE")
    handle_mode(client, collection);
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

void Server::execute_server() {
  while (1) {
    if (poll(&fds[0], fds.size(), -1) ==
        -1) // -1 sonsuza dek bekle demektir, CPU'yu %0 yorar.
      throw std::runtime_error("Error: poll!");

    for (size_t i = 0; i < fds.size(); i++) {
      if (fds[i].fd == server_fd) {
        if (fds[i].revents & POLLIN) {
          sockaddr_in client_addr = {};
          socklen_t client_len = sizeof(client_addr);
          int client_fd =
              accept(server_fd, (sockaddr *)&client_addr, &client_len);
          fcntl(client_fd, F_SETFL, O_NONBLOCK);
          if (client_fd != -1) {
            std::cout << "We have a new clint! Fd : " << client_fd << std::endl;
            Client *new_client = new Client(client_fd);
            clients[client_fd] = new_client;

            pollfd client_poll = {};
            client_poll.fd = client_fd;
            client_poll.events = POLLIN;
            fds.push_back(client_poll);
          }
        }
      } else {
        if (fds[i].revents & POLLIN) {
          char messg[1000] = {};
          int result_recv = recv(fds[i].fd, messg, sizeof(messg), 0);
          // #region agent log
          {
            std::ostringstream oss;
            oss << "fd=" << fds[i].fd << ", recv_result=" << result_recv;
            if (result_recv > 0)
              oss << ", payload=" << std::string(messg, result_recv);
            debug_log("srcs/Server.cpp:169", "pre-fix", "H2|H4",
                      "recv completed for client fd", oss.str());
          }
          // #endregion
          if (result_recv > 0) {
            clients[fds[i].fd]->buffer_append(messg);
            find_first_msg(i);
            // After processing, check if QUIT was requested during
            // find_first_msg
            if (clients.count(fds[i].fd) &&
                clients[fds[i].fd]->get_quit_requested()) {
              int dead_fd = fds[i].fd;
              // #region agent log
              {
                std::ostringstream oss;
                oss << "dead_fd=" << dead_fd
                    << ", nick=" << clients[dead_fd]->get_nick_name();
                debug_log("srcs/Server.cpp:182", "pre-fix", "H2",
                          "closing quit-requested client in poll loop",
                          oss.str());
              }
              // #endregion
              std::cout << "Client quit (fd=" << dead_fd
                        << ", nick=" << clients[dead_fd]->get_nick_name() << ")"
                        << std::endl;
              delete clients[dead_fd];
              clients.erase(dead_fd);
              close(dead_fd);
              fds.erase(fds.begin() + i);
              i--;
            }
          } else if (result_recv == 0) {
            // Client disconnected without QUIT (e.g. netcat Ctrl+C)
            int dead_fd = fds[i].fd;
            std::cout << "Client went (fd=" << dead_fd << ")" << std::endl;
            // Remove from all channels
            std::vector<std::string> to_delete;
            for (std::map<std::string, Channel *>::iterator it = chnnls.begin();
                 it != chnnls.end(); ++it) {
              if (is_member_of_channel(clients[dead_fd], it->first))
                to_delete.push_back(it->first);
            }
            for (size_t j = 0; j < to_delete.size(); j++) {
              chnnls[to_delete[j]]->remove_members(clients[dead_fd]);
              if (chnnls[to_delete[j]]->get_members().empty()) {
                delete chnnls[to_delete[j]];
                chnnls.erase(to_delete[j]);
              }
            }
            delete clients[dead_fd];
            clients.erase(dead_fd);
            shutdown(dead_fd, SHUT_RDWR);
            close(dead_fd);
            fds.erase(fds.begin() + i);
            i--;
          } else
            throw std::runtime_error("Error: recv");
        }
      }
    }
  }
}

Server::~Server() {
  for (std::map<int, Client *>::iterator it = clients.begin();
       it != clients.end(); ++it)
    delete it->second;
  if (server_fd != -1)
    close(server_fd);
}
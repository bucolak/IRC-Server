#include "../libs/Server.hpp"
#include "../libs/ProtocolMessages.hpp"
#include <cctype>

volatile sig_atomic_t g_shutdown_requested = 0;

void handle_sigint(int signal) {
  (void)signal;
  g_shutdown_requested = 1;
}

Server::Server(int port, std::string password) {
  this->port = port;
  this->password = password;
  this->server_fd = -1;
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
  std::cout<<"---ircserv is listening---"<<std::endl;
}

void Server::find_first_msg(int wc) {
  int fd = fds[wc].fd;
  size_t i;

  while (clients.count(fd) && !clients[fd]->get_quit_requested()) {
    std::string msg = clients[fd]->get_buffer();
    if ((i = msg.find('\n')) == std::string::npos)
      break;
    std::string line = msg.substr(0, i);
    if (!line.empty() && line[line.length() - 1] == '\r')
      line.erase(line.length() - 1);
    clients[fd]->set_buffer(msg.erase(0, i + 1));
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
  if (collection.empty())
    return;

  std::string cmd = collection[0];
  for (size_t ci = 0; ci < cmd.size(); ++ci) {
    cmd[ci] = static_cast<char>(
        std::toupper(static_cast<unsigned char>(cmd[ci])));
  }
  if (cmd == "PASS")
    handle_pass(client, collection);
  else if (cmd == "PING")
    handle_ping(client, collection);
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
  else if (cmd == "WHO")
    handle_who(client, collection);
}

Server::~Server() {
  for (std::map<int, Client *>::iterator it = clients.begin();
       it != clients.end(); ++it)
    delete it->second;
  clients.clear();
  for (std::map<std::string, Channel *>::iterator it = chnnls.begin();
       it != chnnls.end(); ++it)
    delete it->second;
  chnnls.clear();
  for (size_t i = 0; i < fds.size(); ++i) {
    if (fds[i].fd != -1)
      close(fds[i].fd);
  }
  fds.clear();
  server_fd = -1;
}

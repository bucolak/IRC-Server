#include "../libs/Server.hpp"

void Server::handle_client_data(size_t &i, size_t &ready_count) {
  char messg[MSG_SIZE] = {};
  int result_recv = recv(fds[i].fd, messg, sizeof(messg), 0);
  
  if (result_recv > 0)  
  {
    clients[fds[i].fd]->buffer_append(std::string(messg, result_recv));
    find_first_msg(i);
    
    if (clients.count(fds[i].fd) && clients[fds[i].fd]->get_quit_requested()) 
    {
      disconnect_client(fds[i].fd, i, ready_count);
    }
  } 
  else if (result_recv == 0) 
  {
    std::cout << "Client went (fd=" << fds[i].fd << ")" << std::endl;
    disconnect_client(fds[i].fd, i, ready_count);
  } 
  else
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
      return;
    throw std::runtime_error("Error: recv");
  }
}

void Server::disconnect_client(int dead_fd, size_t &i, size_t &ready_count) {
  std::vector<std::string> to_delete_chan;
  for (std::map<std::string, Channel *>::iterator it = chnnls.begin(); it != chnnls.end(); ++it) 
  {
    if (is_member_of_channel(clients[dead_fd], it->first))
      to_delete_chan.push_back(it->first);
  }
  for (size_t j = 0; j < to_delete_chan.size(); j++) 
  {
    chnnls[to_delete_chan[j]]->remove_members(clients[dead_fd]);
    if (chnnls[to_delete_chan[j]]->get_members().empty()) 
    {
      delete chnnls[to_delete_chan[j]];
      chnnls.erase(to_delete_chan[j]);
    }
  }

  delete clients[dead_fd];
  clients.erase(dead_fd);
  close(dead_fd);
  fds.erase(fds.begin() + i);
  ready_count--;
  i--;
}

void Server::accept_new_client() {
  sockaddr_in client_addr = {};
  socklen_t client_len = sizeof(client_addr);
  int client_fd = accept(server_fd, (sockaddr *)&client_addr, &client_len);
  
  if (client_fd != -1) 
  {
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) 
    {
      close(client_fd);
      return;
    }
    Client *new_client = new Client(client_fd);
    clients[client_fd] = new_client;

    pollfd client_poll = {};
    client_poll.fd = client_fd;
    client_poll.events = POLLIN;
    fds.push_back(client_poll);
  }
}

void Server::execute_server() {
  while (!g_shutdown_requested) {
    int poll_result = poll(&fds[0], fds.size(), -1);
    if (poll_result == -1 && g_shutdown_requested) break;
    if (poll_result == -1 && errno == EINTR) continue;
    if (poll_result == -1) throw std::runtime_error("Error: poll!");

    size_t ready_count = fds.size();
    for (size_t i = 0; i < ready_count; i++) 
    {
      if (fds[i].fd == server_fd) 
      {
        if (fds[i].revents & POLLIN) 
          accept_new_client();
      } 
      else 
      {
        if (fds[i].revents & POLLIN) 
          handle_client_data(i, ready_count);
      }
    }
  }
}


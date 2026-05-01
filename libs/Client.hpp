#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <vector>

class Client {

private:
  int client_fd;
  std::string user_name;
  std::string nick_name;
  std::string buffer;
  int is_registered;
  bool is_authenticated;
  bool quit_requested;

public:
  Client(int client_fd);
  int get_client_fd();
  int get_is_registered();
  bool get_is_authenticated();
  bool get_quit_requested();
  std::string get_user_name();
  std::string get_nick_name();
  std::string get_buffer();
  void set_buffer(std::string str);
  void set_is_authenticated(bool state);
  void set_is_registered(bool state);
  void set_nick_name(std::string nick);
  void set_user_name(std::string user);
  void set_quit_requested(bool state);

  void buffer_append(std::string newstr);

  // ~Client();
};

#endif
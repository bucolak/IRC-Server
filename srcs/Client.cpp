#include "../libs/Client.hpp"

Client::Client(int client_fd) {
  this->client_fd = client_fd;
  this->is_registered = 0;
  this->is_authenticated = false;
  this->quit_requested = false;
}

int Client::get_client_fd() { return client_fd; }

int Client::get_is_registered() { return is_registered; }

std::string Client::get_user_name() { return user_name; }

std::string Client::get_nick_name() { return nick_name; }

std::string Client::get_buffer() { return buffer; }

bool Client::get_is_authenticated() { return is_authenticated; }

bool Client::get_quit_requested() { return quit_requested; }

void Client::set_buffer(std::string str) { buffer = str; }

void Client::set_is_authenticated(bool state) { is_authenticated = state; }

void Client::set_is_registered(bool state) { is_registered = state; }

void Client::set_nick_name(std::string nick) { nick_name = nick; }

void Client::set_user_name(std::string user) { user_name = user; }

void Client::set_quit_requested(bool state) { quit_requested = state; }

void Client::buffer_append(std::string newstr) { buffer += newstr; }
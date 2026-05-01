#ifndef PROTOCOLMESSAGES_HPP
#define PROTOCOLMESSAGES_HPP

#include <iostream>

#define ERR_NEEDMOREPARAMS(client, command) (std::string(":") + "ircserv 461 " + client + " " + command + " :Not enough parameters\r\n") //ALSO FOR USER

#define ERR_ALREADYREGISTERED(client) (std::string(":") + "ircserv 462 " + client + " :You may not reregister\r\n") //ALSO FOR USER

#define ERR_PASSWDMISMATCH(client) (std::string(":") + "ircserv 464 " + client + " :Password incorrect\r\n")

#define ERR_NONICKNAMEGIVEN(client) (std::string(":") + "ircserv 431 " + client + " :No nickname given\r\n")

#define ERR_ERRONEUSNICKNAME(client,nick) (std::string(":") + "ircserv 432 " + client + " " + nick + " :Erroneus nickname\r\n")

#define ERR_NICKNAMEINUSE(client,nick) (std::string(":") + "ircserv 433 " + client + " " + nick + " :Nickname is already in use\r\n")

#define ERR_NICKCOLLISION(client,nick) (std::string(":") + "ircserv 436 " + client + " " + nick + " :Nickname collision KILL from <user>@<host>\r\n")

#define ERR_NOSUCHCHANNEL(client,channel) (std::string(":") + "ircserv 403 " + client + " " + channel + " :No such channel\r\n")

#define ERR_BADCHANNELKEY(client,channel) (std::string(":") + "ircserv 475 " + client + " " + channel + " :Cannot join channel (+k)\r\n")

#define ERR_NOTONCHANNEL(client,channel) (std::string(":") + "ircserv 442 " + client + " " + channel + " :You're not on that channel\r\n")

#define ERR_CHANOPRIVSNEEDED(client,channel) (std::string(":") + "ircserv 482 " + client + " " + channel + " :You're not channel operator\r\n")

#define ERR_USERNOTINCHANNEL(client,nick,channel) (std::string(":") + "ircserv 441 " + client + " " + nick + " " + channel + " :They aren't on that channel\r\n")

#define ERR_NOSUCHNICK(client,nick) (std::string(":") + "ircserv 401 " + client + " " + nick + " :No such nick/channel\r\n")

#define ERR_CANNOTSENDTOCHAN(client,channel) (std::string(":") + "ircserv 404 " + client + " " + channel + " :Cannot send to channel\r\n")

#define ERR_NORECIPIENT(client) (std::string(":") + "ircserv 411 " + client + " :No recipient given PRIVMSG\r\n")

#define ERR_NOTEXTTOSEND(client) (std::string(":") + "ircserv 412 " + client + " :No text to send\r\n")

#define RPL_WELCOME(nick, user, host) (std::string(":") + "ircserv 001 " + nick + " :Welcome to the ircserv Network, " + nick + "!" + user + "@" + host + "\r\n")

#define ERR_NOTREGISTERED(client) (":ircserv 451 " + client + " :You have not registered\r\n")

#define RPL_NOTOPIC(client, channel) (":ircserv 331 " + client + " " + channel + " :No topic is set\r\n")

#define RPL_TOPIC(client, channel, topic) (":ircserv 332 " + client + " " + channel + " :" + topic + "\r\n")

#define RPL_NAMREPLY(client, channel, list) (":ircserv 353 " + client + " = " + channel + " :" + list + "\r\n")

#define RPL_ENDOFNAMES(client, channel) (":ircserv 366 " + client + " " + channel + " :End of /NAMES list\r\n")

#define ERR_USERONCHANNEL(client, nick, channel) (":ircserv 443 " + client +" "+ nick+" "+ channel+" :is already on channel\r\n")

#define RPL_INVITING(client, nick, channel) (":ircserv 341 " + client +" "+ nick+" "+ channel+"\r\n")

#define RPL_CHANNELMODEIS(client, channel, modes) (":ircserv 324 " + client +" "+ channel+" "+ modes+"\r\n")

#define ERR_UNKNOWNMODE(client, modechar) (":ircserv 472 " + client + " " + modechar + " :is unknown mode char to me\r\n")

#endif

_This project has been created as part of the 42 curriculum by bucolak, aaydogdu_

## Description: What is IRC? 
IRC stands for "Internet Relay Chat" and it is an text-based internet application that provides communicate quickly with other users. IRC, is one of the earliest network protocols for text messaging and multi-participant chatting. It was created in 1988 by Jakko Oikarinen. It is generally designed for chat, one-to-one private messages and group communication through public channels.

As a part of 42 curriculum, we developed an internet relay chat called "ft_irc". Our project "ft_irc" provides very basics of a real internet relay chat. By compiling the project you run the server and you can connect to this server as a client. We took "irssi client" as a reference in this project. after connecting as a client to the server, you can be able to set a nickname, set a username, chat privately with other users or join to a channel and chat publicly etc.  

## Useful Terms for Understanding IRC

**`Server:`** Server is a system that manages network resources, stores data, and delivers services to other devices known as "clients," over a network.

**`Client:`** The software or device that connects to an IRC server to send and receive messages.

**`Nick:`** The unique identity chosen by a user to be recognized by others on the network.

**`Handshake:`** The initial communication process where a client identifies itself to the server to establish a connection.

**`TCP/IP Protocol:`** IP is an address that decides where the data will go. e.g. It allows us to accept data over localhost (127.0.0.1) address. TCP packages the data like an envelope and sends to the correct IP address. TCP ensures that packets reach the recipient (the server) in the exact order they were sent, and without any loss. When a user types “Hello, how are you?”, the server does not receive ‘how’ first and then “Hello.” The words and characters arrive in the exact order they were sent without any loss of data.

**`Socket:`** Every computer has an IP adress and socket, handles the connection through IP address using specific port number and provides data transfer. We use one socket function in our project. Socket function takes 3 parameters: `socket(domain, type, protocol)`.
* `domain` (Address Family): Specifies the protocol family used for communication.
    * `AF_INET`: IPv4 Internet protocols.
    * `AF_INET6`: IPv6 Internet protocols.
* `type`: Specifies socket type. Such as `SOCK_STREAM` TCP (Secure connection based) socket, `SOCK_DGRAM` UDP (Unsecure).
* `Protocol`: Specifies the socket protocol. It is usually given as 0 and find best protocol automatically.

**`Bind Function:`** Connects the opened socket to the port given as a parameter.

**`Listen Fuction:`** The bind function binds the socket to a port, but it is not yet in connection-accepting mode. The listen function tells the socket to start listening for incoming connections.

**`Poll Function:`** It is used to enable an application to multiplex I/O over a set of descriptors. The poll() function will determine which descriptors can read or write data, or whether certain events have occured on the descriptors within the timeout period. Poll function takes 3 parameters: `poll(fds, nfds, timeout)`
* `fds`: pointer to an array of pollfd structures monitoring specific descriptors. (e.g. socket ID, events, revents)
* `nfds`: the exact number of items (structures) in the fds array.
* `timeout`: maximum time to wait in miliseconds. 0 returns immediately, -1 blocks indefinitely.

**`Accept Function:`** Accept new clients. Opens new fds for them, creates Client objects and add them to the poll list.

**`Recv Function:`** Reads data coming from clients.

### Applicable Commands on Our "ft_irc":
* NICK : Set a nickname
* PRIVMSG :Sends private message to specified user(s). (Try `/msg` for irrsi client)
* JOIN : Helps join to a channel
* KICK : Eject a client from the channel
* INVITE : Invite a client to a channel
* TOPIC : Change or view the channel topic
* MODE : Change the channels's mode:
  * i: set/remove invite only channel
  * t: set/remove the restrictions of the TOPIC command to channel operators
  * k: set/remove the channel password
  * o : give/take channel operator privilege
  * l : set/remove the user limit to channel


## Instructions: How to run ft_irc?
After cloning the repo, run this command in project's directory
``` bash
  make
  ```
Project is compiled. Now we need to start the server. The syntax must be in format of `./ircserv <port> <password>`. For example;
``` bash
  ./ircserv 6668 1234
  ```

In an other terminal, connect to the server through a client you chose. We took irssi client as a reference so we suggest you to use irssi or netcat client. In this file, there are instructions for irssi client.

Type "irssi" on your terminal. Install if it is necessary.
``` bash
  irssi
```

To connect ircserv on opened irssi client window, type `/connect <IP-address> <port-number> <password>`. Don't forget to enter the password, otherwise connection will be refused. Giving nick is optional, irssi automatically declares your name. (You can find and change in `~/.irssi/config` file);
```
  /connect 127.0.0.1 6667 1234
```
or
``` bash
  /connect localhost 6667 1234 my_nick123
```
After completing these steps, you will be ready to chat with other users. You can apply and test the commands above (join, topic, kick etc.)

## AI Usage Description:
We positioned AI not as a code generator, we used as a learning and optimization tool. This allowed us to treat it as a guide for understanding.
- **Learning Concepts:** Used for explaining complex networking concepts like I/O Multiplexing, how the poll() system call actually works behind the scenes, and the overall lifecycle of sockets. 

- **Debugging & Error Analysis:** Utilized to compare outputs from different client and confirm correct outputs. Helped us to learn correct syntax forms for different clients such as netcat, irssi etc.
- **Testing:** We used AI for testing our code. It gave us better test cases and led us where the error and bugs are. It acted like a second pair of eyes to double-check our code's edge cases.

## Resources:
```
 https://www.rfc-editor.org/rfc/rfc2812
 https://medium.com/@talhadmr/ft-irc-server-92ffcd1d4338
 http://chi.cs.uchicago.edu/chirc/irc.html#replies
 https://www.corero.com/what-is-internet-relay-chat/
 https://www.geeksforgeeks.org/computer-networks/internet-relay-chat-irc/
 https://www.ibm.com/docs/en/i/7.4.0?topic=ssw_ibm_i_74/apis/poll.htm
 https://www.mirc.com/help/html/index.html?basic_irc_commands.html
 https://dd.ircdocs.horse/refs/commands/
```

## Contributers:
  - https://github.com/bucolak  - bucolak
  - https://github.com/zehraxaa - aaydogdu

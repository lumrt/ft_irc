#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
    int _fd;
    std::string _nickname;
    std::string _username;
    std::string _buffer;
    bool _registered;

public:
    Client(int fd);
    int getFd() const;
    std::string &getBuffer(); // getter du messag envoye par le client
};

#endif
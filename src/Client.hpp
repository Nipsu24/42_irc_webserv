/* **************************************************************************************** */
/*                                                                                          */
/*                                                        ::::::::::: :::::::::   ::::::::  */
/*                                                           :+:     :+:    :+: :+:    :+:  */
/*                                                          +:+     +:+    +:+ +:+          */
/*                                                         +#+     +#++:++#:  +#+           */
/*  By: Timo Saari<tsaari@student.hive.fi>,               +#+     +#+    +#+ +#+            */
/*      Matti Rinkinen<mrinkine@student.hive.fi>,        #+#     #+#    #+# #+#    #+#      */
/*      Marius Meier<mmeier@student.hive.fi>        ########### ###    ###  ########        */
/*                                                                                          */
/* **************************************************************************************** */

#pragma once

#include <string>
#include <netinet/in.h>
#include "Channel.hpp"

enum clientState
{
    REGISTERING,
    REGISTERED,
    DISCONNECTED
};

class Channel;

class Client
{
public:
    // Constructors
    Client();
    Client(int fd, const sockaddr_in &client_addr);
    Client(const Client &other);
    Client &operator=(const Client &other);
    ~Client();

    void setFd(int value);
    int getFd() const;
    void setNick(const std::string &value);
    std::string getNick() const;
    void setUsername(const std::string &value);
    std::string getUsername() const;
    sockaddr_in getAddr() const;
    void setState(clientState state);
    clientState getState() const;
    
    bool cap_status;

private:
    int _fd;
    int _state;
    sockaddr_in _addr;
    std::string _nick;
    std::string _userName;
};

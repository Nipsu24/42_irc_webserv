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

#include "Channel.hpp"
#include "response.hpp"
#include <algorithm>
#include <chrono>

/* ************************************************Constructor Section START*************************************** */
Channel::Channel() {}

Channel::Channel(std::string name)
	: _channelName(name), _channelPassw(""), _userLimit(-1), _topicOperatorsOnly(false), _inviteOnlyEnabled(false) {}

Channel::Channel(const Channel &other)
{
	this->_channelName = other._channelName;
	this->_channelPassw = other._channelPassw;
	this->_userLimit = other._userLimit;
	this->_topicOperatorsOnly = other._topicOperatorsOnly;
	this->_inviteOnlyEnabled = other._inviteOnlyEnabled;
}

Channel &Channel::operator=(const Channel &other)
{
	if (this != &other)
	{ 	this->_channelName = other._channelName;
		this->_channelPassw = other._channelPassw;
		this->_userLimit = other._userLimit;
		this->_topicOperatorsOnly = other._topicOperatorsOnly;
		this->_inviteOnlyEnabled = other._inviteOnlyEnabled;
	}
	return *this;
}

Channel::~Channel() {}

/* ************************************************Constructor Section END*************************************** */

/*Returns the channel name of respective channel class*/
std::string Channel::getChannelName() const { return (_channelName); }

/*Returns channelPassw or empty string in case no password set*/
std::string Channel::getChannelPassw() const
{
	if (_channelPassw.empty())
	{
		std::cout << "No password set for #" << _channelName << std::endl;
		return ("");
	}
	return (_channelPassw);
}

/*Checks if passed client is operator of the channel*/
bool Channel::isClientOperator(Client* client) {
    for (Client* user : _chOperatorList) {
        if (user == client) {
            return true;
        }
    }
    return false;
}

/*Checks if passed client is member of channel*/
bool Channel::isClientInChannel(Client* client) {
    for (Client* user : _userList) {
        if (user == client) {
            return true;
        }
    }
    return false;
}

/*Function for setting the password of a channel*/
void	Channel::setChannelPassw(const std::string& password) { _channelPassw = password; }

/*Returns a copy of a string with the current active modes of a channel and the respective
  parameters if applicable. Inserts + sign at beginning of string. If no mode active, only
  returns string with + sign in it.*/
std::string	Channel::getMode() const
{
	std::string	activeModes;
	std::string parameters;
	
	if (_inviteOnlyEnabled)
		activeModes += 'i';
	if (!_channelPassw.empty()) {
		activeModes += 'k';
		if (parameters.empty())
			parameters = _channelPassw;
		else
			parameters += " " + _channelPassw;
	}
	if (_userLimit != -1) {
		activeModes += 'l';
		if (parameters.empty())
			parameters = std::to_string(_userLimit);
		else
			parameters += " " + std::to_string(_userLimit);
	}
	if (_topicOperatorsOnly)
		activeModes += 't';
	activeModes.insert(0, 1, '+');
	if (!parameters.empty())
		activeModes += " " + parameters;
	return (activeModes);
}

/*Let the user if having operator rights (or if set topic is enabled for all members)
  set the topic of a channel.*/
void Channel::setTopic(Client *client, const std::string& topic)
{
	if (!isClientInChannel(client))
		throw ClientNotInChannelException();
	if (_topicOperatorsOnly)
		throw ClientNotOperatorException();
	else {
		_topic = topic;
	}
}
/*First checks if the client conducting the kick command is in channel and has operator rights. Then
  loops through member list of channel in order to find the matching nick of the user to be kicked out.
  If a match is found, calls removeClient method to kick the user otherwise throws exception.*/
void Channel::setKick(Client *client, const std::string& channelName, std::string& nick)
{
	if (!isClientInChannel(client))
		throw ClientNotInChannelException();
	if (!isClientOperator(client))
		throw ClientNotOperatorException();
	auto it = std::find_if(_userList.begin(), _userList.end(), [&](Client* client) {
		return client->getNick() == nick;
	});
	if (it != _userList.end()) {
		removeClient(*it);
	}
	else {
		throw NickNotExistException();
	}
	std::cout << client->getNick() + " kicked out " + nick + " from " + channelName << std::endl; 
}

void Channel::setInvite(Client *client, const std::string& channelName, std::string& nick)
{
	(void)channelName;
	(void) nick;
	if (!isClientInChannel(client))
		throw ClientNotInChannelException();
	if (!isClientOperator(client))
		throw ClientNotOperatorException();
	if (isClientInChannel(getClientByNickname(nick)))
		throw ClientAlreadyInChannelException();
}

/*Retrieves all client objects from a channel*/
std::vector<Client *> &Channel::getUsers() { return (_userList); }

/*Adds new client to the channel by adding it to the vector array of clients.
  Used in joinChannel function in server.cpp*/
void	Channel::addClient(Client *client) {_userList.push_back(client);}

/*Removes client from channel*/
void Channel::removeClient(Client *client)
{
	auto it = std::find(_userList.begin(), _userList.end(), client);
	if (it != _userList.end())
	{
		_userList.erase(it);
	}
}

/*Retrieves timestamp of channel creation.*/
std::string	Channel::getTimestamp() { return (_timestampOfCreation);}

/*Calculates current time since epoch in seconds and sets respective value to member variable by
  converting value of timestamp into string.*/
void	Channel::setTimestamp() {
	auto now = std::chrono::system_clock::now();
	auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
	_timestampOfCreation = std::to_string(timestamp);
}

/*Getters and setters for different channel modes and parsing part*/
std::string	Channel::getParsedModes() { return (_parsedModes); }

void	Channel::setParsedModes(std::string modes) { _parsedModes = modes; }

std::vector<std::string>	Channel::getParsedParameters() { return (_parsedParameters); }

void	Channel::setParsedParameters(std::vector<std::string> parameters) { _parsedParameters = parameters; }

void	Channel::clearParsedParameters() { _parsedParameters.clear(); }

bool	Channel::getInviteOnlyState() { return (_inviteOnlyEnabled); }

void	Channel::setInviteOnlyState(bool status) { _inviteOnlyEnabled = status; }

int		Channel::getUserLimit() { return (_userLimit); }

void	Channel::setUserLimit(int limit) { _userLimit = limit; }

bool	Channel::getTopicOperatorsOnlyState() { return (_topicOperatorsOnly); }

void	Channel::setTopicOperatorsOnlyState(bool status) { _topicOperatorsOnly= status; }

std::vector<Client *>&	Channel::getChOperatorList() { return (_chOperatorList); }

/*Check is client is in _chOperatorList and adds it, if this is not the case. Client to be passed
  to push_back function, as class object expected (in contrast to erase function which expects an
  iterator)*/
void	Channel::setChOperator(Client* client) {
	auto it = std::find(_chOperatorList.begin(), _chOperatorList.end(), client);
	if (it == _chOperatorList.end())
		_chOperatorList.push_back(client);
}

/*Checks if client is in _chOperatorList and erases client from list if this is the case.
  'it' needs to be passed to erase function, as an iterator is expected here (in contrast to
  push_back function, which expects a class object)*/
void	Channel::unsetChOperator(Client* client) {
	auto it = std::find(_chOperatorList.begin(), _chOperatorList.end(), client);
	if (it != _chOperatorList.end())
		_chOperatorList.erase(it);
}

const char* Channel::ClientNotOperatorException::what() const noexcept {
	return ("Client is not an operator in this channel");
}

const char* Channel::NickNotExistException::what() const noexcept {
	return ("Nick does not exist in this channel");
}

const char* Channel::ChannelNotFoundException::what() const noexcept {
	return ("Channel not found");
}

const char* Channel::ClientNotInChannelException::what() const noexcept {
	return ("Client is not in this channel");
}

const char* Channel::ClientAlreadyInChannelException::what() const noexcept {
	return ("Client already in this channel");
}

/*Returns amount of users within a channel.*/
std::size_t	Channel::getNumberOfUsersInCh() const {
	std::cout << "USER AMOUNT: " <<_userList.size() << std::endl;
	return (_userList.size());
}

/*Called in handleJoin function in order to check whether any channel restrictions apply before user
  joins channel. If a restriction apply, a message is sent to the client with help of the passed
  messageFunc (in handle join MessageServerToClient function)*/
bool	Channel::checkForModeRestrictions(Client &client, std::string password, std::function<void(Client&, const std::string&)> messageFunc)
{
	std::string	response;

	if (!_channelPassw.empty()) {
		if (_channelPassw != password) {
			messageFunc(client, ERR_BADCHANNELKEY(client.getNick(), _channelName));
			return (false);
		}
	}
	if (_userLimit > -1) {
		if (getNumberOfUsersInCh() >= static_cast<std::size_t>(_userLimit)) {
			messageFunc(client, ERR_CHANNELISFULL(client.getNick(), _channelName));
			return (false);
		}
	}
	if (_inviteOnlyEnabled && !isOnInvitationList(&client)) {
			messageFunc(client, ERR_INVITEONLYCHAN(client.getNick(), _channelName));
			return (false);
	}
	return (true);
}

/*Checks if client is channel operator*/
bool	Channel::isChannelOperator(Client* client) {
	auto it = std::find(_chOperatorList.begin(), _chOperatorList.end(), client);
	if (it != _chOperatorList.end())
		return (true);
	else
		return (false);
}

/*Checks if client is on invitation list*/
bool	Channel::isOnInvitationList(Client* client) {
	auto it = std::find(_invitationList.begin(), _invitationList.end(), client);
	if (it != _invitationList.end())
		return (true);
	else
		return (false);
}

/*Checks first if user is already on the invitation list and if not the case, adds him to the list.*/
void	Channel::addToInvitationList(Client* client) {
	auto it = std::find(_invitationList.begin(), _invitationList.end(), client);
	if (it == _invitationList.end())
		_invitationList.push_back(client);
}

/*Returns list of users invited to a channel.*/
std::vector<Client *>& Channel::getInvitationList() { return(_invitationList); }

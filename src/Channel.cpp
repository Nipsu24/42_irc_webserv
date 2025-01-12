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

#include <iostream>
#include <algorithm>
#include "Channel.hpp"

Channel::Channel() {}

Channel::Channel(std::string name) 
		: _channelName(name), _userLimit(-1), _channelPassw(""), _topicCommandEnabled(false), _inviteOnlyEnabled(false) {}

Channel::Channel(const Channel& other) {
    this->_channelName = other._channelName;
}

Channel& Channel::operator=(const Channel& other) {
    if (this != &other) {  // Self-assignment check
        this->_channelName = other._channelName;
    }
    return *this;
}

Channel::~Channel() {}

std::vector<Client*>&	Channel::getUsers() {
	return (_userList);
}

std::string	Channel::getChannelName() const {return _channelName;}

/*Returns empty string in case no password set, otherwise returns channel password.*/
std::string	Channel::getChannelPassw() const {
	if (_channelPassw.empty()) {
		std::cout << "No password set for #" << _channelName << std::endl;
		return ("");
	}
	return _channelPassw;
}

/*Checks if there are any modes set for the channel. If this is the case,
  prints respective modes.*/
void	Channel::printMode() const {
	if (_modes.empty())
		std::cout << "No mode set for #" << _channelName << std::endl;
	else {
		std::string result;
		result += _modes[0];
		for (int i = 1; i < _modes.size(); ++i)
			result += _modes[i][1];
		std::cout << "mode/#" << _channelName << " [" << result << "]" << std::endl;
	}
}

/*Compares elements of _modes vector with passed newModes vector and adds the elements from 
  newModes vector to _modes vector in case they are not yet present. Sorts vector alphabethically.
  Prints at the end the respective modes set to the channel.*/
void	Channel::setMode(const std::vector<std::string>& newModes) {
	std::string	result;
	for (std::string toAddMode : newModes) {
		bool modeAddExists = false;
		bool modeRemoveExists = false;
		for (const std::string& activeMode : _modes) {
			if (toAddMode[0] == '+') {
				if (activeMode == toAddMode){
					modeAddExists = true;
					break;
				}
			}
			else if (toAddMode[0] == '-') {
				if (activeMode[1] == toAddMode[1]) {
					modeRemoveExists = true;
					break;
				}
			}
		}
		if (!modeAddExists && !modeRemoveExists) {
			_modes.push_back(toAddMode);
			result += toAddMode; //could still be improved -> grouping + and - modes if appearing after each other
		}
		if (!modeAddExists && modeRemoveExists) {
			auto it = std::find_if(
				_modes.begin(), _modes.end(),
				[&](const std::string& mode) { return mode[1] == toAddMode[1];});
			if (it != _modes.end()) {
				_modes.erase(it);
				result += toAddMode;
			}
		}
	}
	std::sort(_modes.begin(), _modes.end());
	std::cout << "mode/#" << _channelName << " [" << result << "] by INSERT USERNAME" << std::endl;
	executeMode();
}

/*First draft of function for executing set modes of a channel, used in
  setMode function, logic still to be further defined.*/
void	Channel::executeMode() {
	_inviteOnlyEnabled = false;
	_topicOperatorsOnly = false;
	for (const std::string& activeMode : _modes) {
		if (activeMode[1] == 'i')
			_inviteOnlyEnabled = true;
		if (activeMode[1] == 't')
			_topicOperatorsOnly = true;
	}
}
/*Prints topic of the channel in case it's set, otherwise prints respective hint.*/
void	Channel::printTopic() const {
	if (_topic.empty()) {
		std::cout << "No topic set for #" << _channelName << std::endl;
		return ;
	}
	std::cout << "Topic for " << _channelName << ": " << _topic << std::endl;
}

/*Sets topic of the channel and prints respective message*/
void	Channel::setTopic(const std::string& topic) {
	if (_topicOperatorsOnly == false) { // code to be added in if statement: "|| client has operator status"
		_topic = topic;
		std::cout << "INSERT USERNAME " << "changed the topic of #" << _channelName << " to: " << _topic << std::endl;
	}
}

/*Sets a limit to the number of users in a group and adds '+l' to _modes.*/
void	Channel::setUserLimit(int limit){
	_userLimit = limit;
	std::cout << "mode/#" << _channelName << " [+l" << limit << "]" << " by INSERT USERNAME" << std::endl;
	_modes.push_back("+l");
	std::sort(_modes.begin(), _modes.end());
}

/*Adds new client to the channel by adding it to the vector array of clients.
  Used in joinChannel function in server.cpp*/
void	Channel::addClient(Client* client) {
    _userList.push_back(client);
}

//Further functions TBD//

int	main(void)
{
	Channel	channel("firstCh");
	std::vector<std::string>	newModes = {"+i", "+c"};
	std::vector<std::string>	newModes2 = {"+i", "+n"};
	std::vector<std::string>	newModes3 = {"+i", "-n"};
	std::cout << channel.getChannelName() << std::endl;
	std::cout << channel.getChannelPassw() << std::endl;
	channel.printMode();
	channel.setMode(newModes);
	channel.printMode();
	channel.setMode(newModes2);
	channel.printMode();
	channel.setMode(newModes3);
	channel.printMode();
	channel.printTopic();
	channel.setTopic("Potatoes");
	channel.printTopic();
	return (0);
}
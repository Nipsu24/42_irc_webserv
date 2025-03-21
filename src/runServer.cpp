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

#include "Server.hpp"
#include <sys/socket.h>
#include <poll.h>
#include <system_error>

/*Signal handler for SIGINT, SIGTERM, SIGQUIT, and SIGSEGV*/
volatile sig_atomic_t		server_running;

/*Creates a server socket (AF_INET for IPv4), with tcp socket type (SOCK_STREAM) and sets socket
  options (SO_REUSEADDR for being able to reuse address and port if in time_wait state)*/
int Server::createServerSocket()
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		throw std::runtime_error("Socket operation failed: Connection refused");
	}
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		close(server_fd);
		throw std::system_error(errno, std::generic_category(), "setsockopt failed: Unable to set SO_REUSEADDR");
	}
	return server_fd;
}

/*Binds and listens on the server socket. Defines and stores server's socket address, uses INADDR_ANY
  to avoid binding to a particular IP but instead make server listen to all available IPs*/
void Server::bindAndListen(int server_fd)
{
	sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(_port);

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("Bind failed");
		close(server_fd);
		throw std::system_error(errno, std::generic_category(), "Failed to bind socket");
	}

	if (listen(server_fd, MAX_CLIENTS) == -1)
	{
		close(server_fd);
		throw std::system_error(errno, std::generic_category(), "Listen failed");
	}

	std::cout << "Server is listening on port " << _port << "..." << std::endl;
}

/*Handle a new client connection*/
void Server::handleNewClient(int server_fd)
{
	
	sockaddr_in client_addr = {};
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_fd == -1)
	{
		perror("Accept failed");
		return;
	}
	std::cout << "New client connected." << std::endl;
	_clients.push_back(new Client(client_fd, client_addr));
	_clients.back()->setState(REGISTERING);
}

/*Handles events on the server socket and client sockets, using revents (indicating which event
  occured on the file descriptor) and POLLIN (readable data available). Uses recv to read data
  from client socket and stores it in buffer for further processing.*/
void Server::handleEvents(std::vector<struct pollfd> &fds)
{
	for (size_t i = 1; i < fds.size(); ++i)
	{
		if (fds[i].revents & POLLIN)
		{
			char buffer[BUFFER_SIZE];
			ssize_t bytes_read = recv(fds[i].fd, buffer, BUFFER_SIZE - 1, 0);
			if (bytes_read <= 0)
			{
				std::cout << "Client disconnected." << std::endl;
				close(fds[i].fd);
				_clients.erase(_clients.begin() + i - 1);
			}
			else
			{
				buffer[bytes_read] = '\0';
				std::string message(buffer);
				handleClientMessage(*_clients[i - 1], message);
			}
		}
	}
}

void Server::cleanupResources(int server_fd)
{
	for (auto &client : _clients)
	{
		close(client->getFd());
		delete client;
	}
	_clients.clear();
	for (auto &channel : _channels)
	{
		delete channel; 
	}
	_channels.clear(); 
	close(server_fd);
}

void handle_sig(int sig) 
{
	if(sig == SIGSEGV)
		std::cerr << "Segmentation fault occurred! Exiting." << std::endl;
	else if(sig == SIGINT || sig == SIGTERM ||sig == SIGQUIT)
		std::cout << std::endl << "Shutting down server..." << std::endl;
	server_running = 0;
}

void HandleSignals()
{
	server_running = 1;
	signal(SIGINT, handle_sig); 
	signal(SIGTERM, handle_sig); 
	signal(SIGQUIT, handle_sig); 
	signal(SIGSEGV, handle_sig); 
}

/*Adds each server socket and each client socket to list of monitored fds and sets POLLIN respectively
  for checking incoming data. Uses poll() system call for monitoring multiple fds(sockets) simultaneaously.
  It checks whether one or more fds are ready for I/O operations otherwise blocks system until readyness.
  If poll not successful as disrupted by signal, skips rest of loop with continue for next iteration.
  If successful, uses revents and POLLIN for server socket, checking that a new event occured and
  that a new connection is ready to be accepted. Events on client sockets handled by handleEvents method.*/
void Server::runServer()
{

	int server_fd = createServerSocket();
	bindAndListen(server_fd);
	HandleSignals();

	std::vector<struct pollfd> fds;
	while (server_running)
	{
		
		fds.clear();
		fds.push_back({server_fd, POLLIN, 0}); 

		for (const auto &client : _clients)
		{
			fds.push_back({client->getFd(), POLLIN, 0});
		}
		int poll_result = poll(fds.data(), fds.size(), -1);
		if (poll_result == -1)
		{
			if (errno == EINTR) 
				continue;
			perror("Poll failed");
			break;
		}
		if (fds[0].revents & POLLIN )
		{
			handleNewClient(server_fd);
		}
		handleEvents(fds);
	}
	cleanupResources(server_fd);
}

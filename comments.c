#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

fd_set	ready, fd, action;														// File descriptor sets for tracking socket activity
int		clients[65000];															// Number of client connections allowed
int		fdMax, idNext = 0, s = 0;												// Variable to track the maximum socket descriptor, Identifier for the next client connection
char	buffer[200100];															// Buffer used for message exchange

void	fterror(char *str)
{
	if (s > 2)
		close(s);
	write(2, str, strlen(str));
	exit(1);
}

void	sendAll(int n)
{
	for (int i = 3; i <= fdMax; ++i)											// Check each socket for activity
		if (FD_ISSET(i, &fd) && i != n)											// Check if the activity is on the server socket
			if (send(i, buffer, strlen(buffer), 0) < 0)							// Send the message to other clients
				fterror("Fatal error\n");
	bzero(&buffer, sizeof(buffer));
}

int	main(int ac, char **av)
{
	if (ac == 1)
		fterror("Wrong number of arguments\n");
	int					sclient;
	struct sockaddr_in	servaddr, cli;											// Structure to hold the server address
	socklen_t			len = sizeof(cli);
	s = socket(AF_INET, SOCK_STREAM, 0); 
	if (s == -1)
		fterror("Fatal error\n");
	bzero(&servaddr, sizeof(servaddr));											// Set up the server address
	servaddr.sin_family = AF_INET;												// Set address family to IPv4
	servaddr.sin_addr.s_addr = htonl(2130706433);								// Set the IP address to localhost
	servaddr.sin_port = htons(atoi(av[1]));										// Set the port number from the command line argument
	if ((bind(s, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)	// Bind the server socket to the specified address
		fterror("Fatal error\n");
	if (listen(s, 10) != 0)														// Listen for incoming connections
		fterror("Fatal error\n");
	fdMax = s;																	// Initialise the active sockets set
	FD_ZERO(&action);															// Clear the set of active sockets
	FD_SET(s, &action);															// Add the server socket to the set
	while(1)
	{// Wait for activity on the sockets
		ready = fd = action;
		if (select(fdMax + 1, &ready, &fd, 0, 0) < 0)
			continue;
		if (FD_ISSET(s, &ready))												// Check if the activity is on the server socket
		{// New client connection
			sclient = accept(s, (struct sockaddr *)&cli, &len);					// Accept a new client connection accept(s, (struct sockaddr *)&address, &len)
			if (sclient < 0)
				continue;
			FD_SET(sclient, &action);											// Add the client socket to the set of active sockets
			sprintf(buffer, "server: client %d just arrived\n", idNext);		// Prepare the welcome message
			clients[sclient] = idNext++;										// Add the client socket to the array
			sendAll(sclient);
			fdMax = sclient > fdMax ? sclient : fdMax;							// Update the maximum socket descriptor
			continue;
		}
		for (int i = 3; i <= fdMax; ++i)										// Check each socket for activity
		{
			if (FD_ISSET(i, &ready))											// Check if the activity is on the server socket
			{
				int		res = 1;
				char	msg[20000];
				bzero(&msg, sizeof(msg));
				while(res == 1 && msg[strlen(msg) - 1] != '\n')
					res = recv(i, msg + strlen(msg), 1, 0);						// Receive data from the client
				if (res <= 0)													// Notify remaining clients about the disconnected client
				{// Close the socket and remove it from the active set
					sprintf(buffer, "server: client %d just left\n", clients[i]);// Prepare the disconnection message
					FD_CLR(i, &action);											// Remove the client socket from the set of active sockets	
					close(i);													// Close the client socket
				}
				else //Broadcast the received message to all other clients
					sprintf(buffer, "client %d: %s", clients[i], msg);			// Add client identifier to the message
				sendAll(i);
			}
		}
	}
	return (0);
}

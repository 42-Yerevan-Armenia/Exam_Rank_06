#include <stdlib.h>         //added to main.c of subject
#include <stdio.h>          //added to main.c of subject
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CLIENTS 128     // Maximum number of client connections allowed
#define BUFFER_SIZE 200000  // Size of the buffer used for message exchange

void	fterror(char *str)
{
	if (str)
		write(2, str, strlen(str));
	else
		write(2, "Fatal error", strlen("Fatal error"));
	write(2, "\n", 1);
	exit(1);
}

int	main(int ac, char **av)
{
	if (ac != 2)
		fterror("Wrong number of arguments");
	int	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		fterror(NULL);
	// Initialise the active sockets set
	fd_set active, ready;                                                       // File descriptor sets for tracking socket activity
	FD_SET(s, &active);                                                         // Add the server socket to the set
	FD_ZERO(&active);                                                           // Clear the set of active sockets
	int		fdMax = 0, idNext = 0;                                              // Variable to track the maximum socket descriptor, Identifier for the next client connection
    // File descriptor sets
    char	buffer[BUFFER_SIZE];                                                // Buffer for storing received messages
    int		clients[MAX_CLIENTS];                                               // Array to store client socket descriptors
    // Set up the server address
	struct sockaddr_in	address = {0};		                                    // Structure to hold the server address
	address.sin_family = AF_INET;			                                    // Set address family to IPv4
	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);                           // Set the IP address to localhost
    address.sin_port = htons(atoi(av[1]));                                      // Set the port number from the command line argument

	if (bind(s, (struct sockaddr *)&address, sizeof(address)) < 0)              // Bind the server socket to the specified address
		fterror(NULL);
	if (listen(s, 10) < 0)                                                      // Listen for incoming connections
		fterror(NULL);
	while (1)
	{   // Wait for activity on the sockets
		ready = active;                                                         // Copy the active sockets set for use with select()
		if (select(fdMax + 1, &ready, NULL, NULL, NULL) < 0)
			continue;
		// Check each socket for activity
		for (int fdI = 0; fdI <= fdMax; fdI++)
		{   // New client connection
			if (FD_ISSET(fdI, &ready) && fdI == s)                              // Check if the activity is on the server socket
			{
				int sclient = accept(s, NULL, NULL);                            // Accept a new client connection accept(s, (struct sockaddr *)&address, &len)
				if (sclient < 0)
					continue;
				// Add the new client socket to the active set
				FD_SET(sclient, &active);                                       // Add the client socket to the set of active sockets
				fdMax = (sclient > fdMax ? sclient : fdMax);                    // Update the maximum socket descriptor
				// Send a welcome message to the client
				sprintf(buffer, "server: client %d just arrived\n", idNext);    // Prepare the welcome message
				send(sclient, buffer, strlen(buffer), 0);                       // Send the welcome message to the client
				clients[idNext++] = sclient;                                    // Add the client socket to the array
			}
			if (FD_ISSET(fdI, &ready) && fdI != s)
			{   // Data received from a client
				int res = recv(fdI, buffer, sizeof(buffer) - 1, 0);             // Receive data from the client
				if (res <= 0)
				{   // Notify remaining clients about the disconnected client
					sprintf(buffer, "server: client %d just left\n", fdI);      // Prepare the disconnection message
                    for (int i = 0; i < idNext; i++) 
                        if (clients[i] != fdI) 
                            send(clients[i], buffer, strlen(buffer), 0);        // Send the disconnection message to other clients
					// Close the socket and remove it from the active set
                    close(fdI);             // Close the client socket
                    FD_CLR(fdI, &active);   // Remove the client socket from the set of active sockets					
				}
				else
                {   // Broadcast the received message to all other clients
                    buffer[res] = '\0';                                         // Null-terminate the received message
                    sprintf(buffer, "client %d: %s\n", fdI, buffer);            // Add client identifier to the message
                    for (int i = 0; i < idNext; i++) 
                        if (clients[i] != fdI) 
                            send(clients[i], buffer, strlen(buffer), 0);        // Send the message to other clients
                }
			}
		}
	}
}
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CLIENTS 128
#define BUFFER_SIZE 200000

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
    char				buffer[BUFFER_SIZE];
    int					clients[MAX_CLIENTS];
	int					fdMax = 0, idNext = 0;
	fd_set				action, ready;
	FD_ZERO(&action);
	fdMax = s;
	FD_SET(s, &action);
	struct sockaddr_in	servaddr = {0};
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port = htons(atoi(av[1]));

	if (bind(s, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
		fterror(NULL);
	if (listen(s, 10) < 0)
		fterror(NULL);
	while (1)
	{
		ready = action;
		if (select(fdMax + 1, &ready, NULL, NULL, NULL) < 0)
			continue;
		for (int	fdI = 0; fdI <= fdMax; fdI++)
		{
			if (FD_ISSET(fdI, &ready) && fdI == s)
			{	
				int	sclient = accept(s, NULL, NULL);
				if (sclient < 0)
					continue;
				FD_SET(sclient, &action);
				fdMax = (sclient > fdMax ? sclient : fdMax);
				sprintf(buffer, "server: client %d just arrived\n", idNext);
				send(sclient, buffer, strlen(buffer), 0);
				clients[idNext++] = sclient;
			}
			if (FD_ISSET(fdI, &ready) && fdI != s)
			{
				int	res = recv(fdI, buffer, sizeof(buffer) - 1, 0);
				if (res <= 0)
				{
					sprintf(buffer, "server: client %d just left\n", fdI);
                    for (int i = 0; i < idNext; i++) 
                        if (clients[i] != fdI) 
                            send(clients[i], buffer, strlen(buffer), 0);
                    close(fdI);
                    FD_CLR(fdI, &action);
				}
				else
                {
                    buffer[res] = '\0';
                    sprintf(buffer, "client %d: %s\n", fdI, buffer);
                    for (int i = 0; i < idNext; i++) 
                        if (clients[i] != fdI) 
                            send(clients[i], buffer, strlen(buffer), 0);
                }
			}
		}
	}
}
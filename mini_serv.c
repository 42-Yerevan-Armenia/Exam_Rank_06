#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

typedef struct s_clients
{
    int     id;
    char    msg[1024];
} t_clients;

t_clients   clients[1024];
fd_set      ready, fd, action;
int         fdMax = 0, idNext = 0;
char        rbuffer[120000], wbuffer[120000];

void	fterror(char *str)
{
	if (str)
		write(2, str, strlen(str));
	else
		write(2, "Fatal error", strlen("Fatal error"));
	write(2, "\n", 1);
	exit(1);
}

void    sendAll(int n)
{
    for(int i = 0; i <= fdMax; i++)
        if(FD_ISSET(i, &fd) && i != n)
            send(i, wbuffer, strlen(wbuffer), 0);
}

int	main(int ac, char **av)
{
	if (ac < 0)
		fterror("Wrong number of arguments");
	int	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		fterror(NULL);
	FD_ZERO(&action);
	bzero(&clients, sizeof(clients));
	fdMax = s;
	FD_SET(s, &action);
	struct sockaddr_in	servaddr;
	socklen_t           len;
   	bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port = htons(atoi(av[1]));
	if (bind(s, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
		fterror(NULL);
	if (listen(s, 10) < 0)
		fterror(NULL);
	while (1)
	{
		ready = fd = action;
		if (select(fdMax + 1, &ready, &fd, NULL, NULL) < 0)
			continue;
		for (int fdI = 0; fdI <= fdMax; fdI++)
		{
			if (FD_ISSET(fdI, &ready) && fdI == s)
			{
				int	sclient = accept(s, (struct sockaddr *)&servaddr, &len);
				if (sclient < 0)
					continue;
				fdMax = (sclient > fdMax ? sclient : fdMax);
				clients[sclient].id = idNext++;
				FD_SET(sclient, &action);
				sprintf(wbuffer, "server: client %d just arrived\n", idNext);
				sendAll(sclient);
				break;
			}
			if (FD_ISSET(fdI, &ready) && fdI != s)
			{
				int	res = recv(fdI, rbuffer, sizeof(rbuffer) - 1, 0);
				if (res <= 0)
				{
					sprintf(wbuffer, "server: client %d just left\n", fdI);
					sendAll(fdI);
                    close(fdI);
                    FD_CLR(fdI, &action);
				}
				else
                {
					for (int i = 0, j = strlen(clients[fdI].msg); i < res; i++, j++)
					{
						clients[fdI].msg[j] = rbuffer[i];
						if (clients[fdI].msg[j] == '\n')
						{
							clients[fdI].msg[j] = '\0';
							sprintf(wbuffer, "client %d: %s\n", clients[fdI].id, clients[fdI].msg);
							sendAll(fdI);
							bzero(&clients[fdI].msg, strlen(clients[fdI].msg));
							j = -1;
                   	    }
                   	}
				break;
				}
			}
		}
	}
}

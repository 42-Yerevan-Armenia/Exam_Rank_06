#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

fd_set	ready, fd, action; 
int		clients[1024];
int		fdMax = 0, idNext = 0, s = 0;
char	buffer[120000];

void	fterror(char *str)
{
	if (s > 2)
		close(s);
	write(2, str, strlen(str));
	exit(1);
}

void	sendAll(int n)
{
	for (int i = 3; i <= fdMax; ++i)
		if (FD_ISSET(i, &fd) && i != n)
			if (send(i, buffer, strlen(buffer), 0) < 0)
				fterror("fatal error\n");
	bzero(&buffer, sizeof(buffer));
}

int	main(int ac, char **av)
{
	if (ac == 1)
		fterror("Wrong number of arguments\n");
	s = socket(AF_INET, SOCK_STREAM, 0); 
	if (s == -1)
		fterror("fatal error\n");
	fdMax = s;
	FD_ZERO(&action);
	FD_SET(s, &action);
	struct sockaddr_in	servaddr, cli; 
	socklen_t			len = sizeof(cli);
	bzero(&servaddr, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(av[1])); 
	if ((bind(s, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		fterror("fatal error\n");
	if (listen(s, 10) != 0)
		fterror("fatal error\n");
	while(1)
	{
		ready = fd = action;
		if (select(fdMax + 1, &ready, &fd, 0, 0) < 0)
			continue;
		if (FD_ISSET(s, &ready))
		{
			int sclient = accept(s, (struct sockaddr *)&cli, &len);
			if (sclient < 0)
				fterror("fatal error\n");
			FD_SET(sclient, &action);
			sprintf(buffer, "server: client %d just arrived\n", idNext);
			clients[sclient] = idNext++;
			sendAll(sclient);
			fdMax = sclient > fdMax ? sclient : fdMax;
			continue;
		}
		for (int i = 3; i <= fdMax; ++i)
		{
			if (FD_ISSET(i, &ready))
			{
				int		res = 1;
				char	msg[1024];
				bzero(&msg, sizeof(msg));
				while(res == 1 && msg[strlen(msg) - 1] != '\n')
					res = recv(i, msg + strlen(msg), 1, 0);
				if (res <= 0)
				{
					sprintf(buffer, "server: client %d just left\n", clients[i]);
					FD_CLR(i, &action);
					close(i);
				}
				else
					sprintf(buffer, "client %d: %s", clients[i], msg);
				sendAll(i);
			}
		}
	}
	return (0);
}

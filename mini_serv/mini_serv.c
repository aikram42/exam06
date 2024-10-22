#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*add these headers */
#include <sys/select.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

/*add these two helper functions*/
void	error(bool arg) {
	if (!arg)
		write(2, "Error error\n", 12);
	else
		write(2, "Wrong number of arguments\n", 26);
	exit(1);
}

void	relayMsg(int sender, int maxfdz, char *msg, fd_set* wrtfd) {
	for (int fd = 0; fd <= maxfdz; fd++)
		if (fd != sender && FD_ISSET(fd, wrtfd))
			send(fd, msg, strlen(msg), 0);
}

int main(int c, char *v[]) {
	/*  these are given
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;
 	*/

	//modify the provided variables to:
	int sockfd, connfd, maxfdz, clientz[65365], id = 0;
	struct sockaddr_in servaddr, cli;
	socklen_t len;
	fd_set fdz, rdfd, wrtfd;
	char *msgz[65365], alertz[50], buff[512];


	if (c != 2)
		error(!true);
	
	//! clean the following code provided!
	/*
	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(8081); 

	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n");
	if (listen(sockfd, 10) != 0) {
		printf("cannot listen\n"); 
		exit(0); 
	}
	*/
	//it should look like this after u get rid of the printfz:
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
		error(true);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(v[1]));

	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		error(true);

	if (listen(sockfd, 10) != 0)
		error(true);

	FD_ZERO(&fdz);
	FD_SET(sockfd, &fdz);
	maxfdz = sockfd;
	while (true) {
		rdfd = wrtfd = fdz;
		if (select(maxfdz + 1, &rdfd, &wrtfd, 0, 0) < 0)
			error(true);
		for (int fd = 0; fd <= maxfdz; fd++)
		{
			if (!FD_ISSET(fd, &rdfd))
				continue ;
			if (fd == sockfd) {
				// change the following provided code:
				/*
				len = sizeof(cli);
				connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
				if (connfd < 0) { 
      					printf("server acccept failed...\n"); 
      				exit(0); 
   				} 
   				else
   				     printf("server acccept the client...\n");
				*/
				// to:
				len = sizeof(cli);
				connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
				if (connfd < 0)
					error(true);

				if (maxfdz < connfd)	
					maxfdz = connfd;
		
				clientz[connfd] = id++;
				msgz[connfd] = NULL;

				sprintf(alertz, "server: client %d just arrived\n", clientz[connfd]);
				relayMsg(connfd, maxfdz, alertz, &wrtfd);

				FD_SET(connfd, &fdz);
				//now add the last remaing part
			} else {
				int rcv = recv(fd, buff, 511, 0);
				if (rcv <= 0) {
					sprintf(alertz, "server: client %d just left\n", clientz[fd]);
					relayMsg(fd, maxfdz, alertz, &wrtfd);
					close(fd);
					if (msgz[fd])
						free(msgz[fd]);
					msgz[fd] = NULL;
					FD_CLR(fd, &fdz);
					continue ;
				}

				buff[rcv] = '\0';
				msgz[fd] = str_join(msgz[fd], buff);

				for (char *msg = NULL; extract_message(&msgz[fd], &msg); msg = NULL)
				{
					sprintf(alertz, "client %d: ", clientz[fd]);
					relayMsg(fd, maxfdz, alertz, &wrtfd);
					relayMsg(fd, maxfdz, msg, &wrtfd);
					free(msg);
				}
			}

		}

	}

}

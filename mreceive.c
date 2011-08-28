#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "mreceive.h"

#define BUFFERSIZE 1452

int main (int argc, char * argv[]) 
{
	char buffer[BUFFERSIZE];
	int port = atoi(argv[1]);
	struct mheader header;

	int socket = socket_create_listen("0.0.0.0", port, 4);
	
	struct sockaddr_in si;
	socklen_t len = sizeof(si);

	while (true)
	{
		int client = accept(socket, (void *)&si, &len);
		if(client < 0)
		{
			perror("accept");
			return -1;
		}

		if(read(client, &header, sizeof(struct mheader)) < 0)
		{
			perror("read");
			return -1;
		}

		int file = open(header.filename, O_WRONLY| O_TRUNC | O_CREAT, 0644);
		if(file < 0)
		{
			perror("open");
			return -1;
		}

		int i, bytes;
		for(i = header.length, bytes = 0; i > 0; i -= bytes)
		{
			bytes = read(client, buffer, BUFFERSIZE);
			if(bytes < 0)
			{
				perror("read");
				return -1;
			}

			if(write(file, buffer, bytes) < 0)
			{
				perror("write");
				return -1;
			}
		}

		close(file);
		socket_close(client);
		printf("%s received (%i bytes)\n", header.filename, header.length);
	}

	socket_close(socket);
	
	return 0;
}

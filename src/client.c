#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main(int argc, char **argv) {
	int socketFd;
	struct sockaddr_un socketAddress;
	char buffer[256];
  char * response;

	/* Initialize the sockaddr_un struct */
	socketAddress.sun_family = AF_UNIX;
	strcpy(socketAddress.sun_path, argv[1]);

	/* Setup the socket, Connect, and Send the send CL Arg */
	socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
	connect(socketFd, (struct sockaddr *)&socketAddress, sizeof(socketAddress));
	send(socketFd, argv[2], strlen(argv[2]), 0);

	/* Check if we recieved a response */
  while (recv(socketFd, buffer, sizeof(buffer), 0) > 0)
    strcat(response, buffer);

  printf("%s", response);

	/* Close and return */
	close(socketFd);
	return 0;
}

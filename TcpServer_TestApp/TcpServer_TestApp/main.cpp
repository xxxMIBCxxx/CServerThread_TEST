#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <memory.h>


int main()
{
	int sock0;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	socklen_t len;
	int sock;
	int n;
	char buf[32];

	sock0 = socket(AF_INET, SOCK_STREAM, 0);
	if (sock0 < 0) 
	{
		perror("socket");
		return 1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock0, (struct sockaddr*) & addr, sizeof(addr)) != 0) 
	{
		perror("bind");
		return 1;
	}

	if (listen(sock0, 5) != 0) 
	{
		perror("listen");
		return 1;
	}

	while (1) 
	{
		len = sizeof(client);
		sock = accept(sock0, (struct sockaddr*) & client, &len);
		if (sock < 0) {
			perror("accept");
			break;
		}

		printf("accepted connection from %s, port=%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

		n = write(sock, "HELLO Client!", 13);
		if (n < 1) {
			perror("write");
			break;
		}

		memset(buf, 0, sizeof(buf));
		n = read(sock, buf, sizeof(buf));
		printf("%d, %s\n", n, buf);

		close(sock);
	}

	close(sock0);

	return 0;
}

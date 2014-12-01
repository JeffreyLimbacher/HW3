// Varatep Buranintu
// Network Compression Detection
// Client implementation
// 11/26/2014

// client.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFFER_LIM 512

int num_packets = 0;
char *host = 0;

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int send_num_packets(char *argv[]) {
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[256];
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("error opening udp socket");
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"error host doesnt exist\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("error connecting to server");
	// printf("Please enter the message: ");
	// bzero(buffer,256);
	// fgets(buffer, 255, num_packets);
	// (void) strncpy(buffer, itoa(num_packets), BUFFER_LIM);
	// itoa(num_packets, buffer, 10);
	snprintf(buffer, 2, "%d", num_packets);
	n = write(sockfd, buffer, strlen(buffer));
	if (n < 0) 
		error("error writing to udp socket\n");
	bzero(buffer, BUFFER_LIM);
	n = read(sockfd,buffer,255);
	if (n < 0) 
		error("error reading from udp socket\n");
	printf("%s\n",buffer);
	close(sockfd);
	return 0;
}

int send_datagram(char *argv[]) {
	int sock, n;
	unsigned int length;
	struct sockaddr_in server, from;
	struct hostent *hp;
	char buffer[BUFFER_LIM];

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) error("socket error");

	server.sin_family = AF_INET;
	hp = gethostbyname(host);
	if (hp==0) error("Unknown host");

	bcopy((char *)hp->h_addr, 
			(char *)&server.sin_addr,
			hp->h_length);
	server.sin_port = htons(atoi(argv[2]));
	length=sizeof(struct sockaddr_in);
	// printf("message to send: ");
	bzero(buffer, BUFFER_LIM);
	// fgets(buffer, BUFFER_LIM, argv[3]);
	// buffer = argv[3];
	for (int count = 1; count <= atoi(argv[4]); count++) {
		n=sendto(sock, argv[3],
				strlen(argv[3]),0,(const struct sockaddr *)&server,length);
		if (n < 0) error("Sendto");
		n = recvfrom(sock,buffer,256,0,(struct sockaddr *)&from, &length);
		if (n < 0) error("recvfrom");
		write(1,"server response: ", 17);
		write(1, buffer, n);
	}
	close(sock);
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: ./client host port message count\n");
		exit(0);
	}

	host = argv[1];
	num_packets = atoi(argv[4]);
	return send_num_packets(argv) + send_datagram(argv);
	// send_datagram(argv);

	return 0;

}

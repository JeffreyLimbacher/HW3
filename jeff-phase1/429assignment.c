#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 32323
#define MAX_BUFFER 8092

char mode = 'c';
char *host = 0;
int num_packets = 50;

int arg_parse(int argc, char **argv){
	if(argc < 2){
		return 1;
	}
	int i;
	for(i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			char flag = argv[i][1];
			switch(flag){
				case 'c':
				case 's':
					mode = flag;
					break;
				case 'h':
					host = argv[++i];
					break;
				case 'n':
					num_packets = atoi(argv[++i]);
					break;
				default:
					return 1;
			}
		}
		else{
			return 1;
		}
	}
	if(mode == '\0'){
		return 1;
	}
	return 0;
}

int get_number_serv() {
	int listenfd, connfd;
	char buff[MAX_BUFFER];
	struct sockaddr_in servaddr = {0};
	struct sockaddr_in cliaddr = {0};
	
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Server: %s\n", strerror(errno));
		return -1;
	}
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	if(bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr))){
		fprintf(stderr, "Server: %s\n", strerror(errno));
		return -2;
	}

	if(listen(listenfd, 2)){
		fprintf(stderr, "Server: %s\n", strerror(errno));
		return -3;
	}

	int clilen = sizeof(cliaddr);
	connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen);
	if(connfd == -1){
		fprintf(stderr, "Server: %s\n", strerror(errno));
		return -4;
	}
	int num;
	int bytes = read(connfd, &num, sizeof(num));
	num = ntohl(num);
	if(bytes == -1){
		fprintf(stderr, "Server: %s\n", strerror(errno));
		return -5;
	}
	if(bytes != 4){
		fprintf(stderr, 
			"Server: Received non-integer: size %d. Exiting...\n", 
			bytes);
		return -6;
	}

	printf("Server: received number of packets to be sent: %d\n", num);
	close(connfd);
	close(listenfd);
	return num;
}

int receive_packets(int num) {
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)
	{
		fprintf(stderr, "Server: Failed to set timeout.\n");
	}

	int recv_n, i, pack_count = 0;
	char buf[MAX_BUFFER + 1];

	for(i = 0; i < num; i++)
	{
		int len = sizeof(cliaddr);
		recv_n = recvfrom(sockfd, buf, MAX_BUFFER, 
			0, (struct sockaddr*) &cliaddr, &len);

		if(recv_n < 0) {
			if(errno == EAGAIN || EWOULDBLOCK){
				break;
			}
			else {
				fprintf(stderr, "Server: error reading from UDP socket\n.");
				return -1;
			}
		}
		else {
			pack_count++;
			printf("%s\n", buf);
		}
	}
	fprintf(stdout, "Server: Received %d packets out of %d.\n", pack_count, num_packets);
	return i;
}

int be_server(){

	num_packets = get_number_serv();
	if(num_packets < 0) {
		return num_packets;
	}

	int ret_val = receive_packets(num_packets);

	return 0;
}

/*****************************************************************************/

/*****************************************************************************/

int send_number()
{

	int sockfd;
	struct sockaddr_in servaddr;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Client: %s\n", strerror(errno));
		return -1;
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	inet_pton(AF_INET, host, &servaddr.sin_addr);

	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))){
		fprintf(stderr, "Client: %s\n", strerror(errno));
		return -2;
	}

	int byte_ordered = htonl(num_packets);

	int ret = write(sockfd, &byte_ordered, sizeof(byte_ordered));
	printf("Client: sent out number of packets to be sent: %d\n", num_packets);

	close(sockfd);
	return 0;
}

int send_packets(){
	int sockfd;
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	inet_pton(AF_INET, host, &servaddr.sin_addr);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	char *buf = "Hello World";
	int i;
	for(i = 0; i < num_packets; i++) {
		write(sockfd, buf, strlen(buf));
	}
	fprintf(stdout, "Client: done.\n");
	return 0;
}

int be_client(){
	int ret_val = send_number();

	if(ret_val < 0) {
		return ret_val;
	}
	usleep(100);
	ret_val = send_packets();

	return 0;
}


int main(int argc, char** argv){
	if(arg_parse(argc, argv)){
		fprintf(stderr, "nope\n");
		return 1;
	}
	if(host == NULL){
		host = "localhost";
	}
	if(mode=='s'){
		return be_server();
	}
	else{
		return be_client();
	}
}
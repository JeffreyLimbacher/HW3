#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>


//GLOBALS-------------------
#define MAX_BUF_SIZE 512 //random large number sufficient enough for payload of datagram: "Hello World!"

char r_or_s = 'r'; //r = reciever, s = sender. default to reciever.

uint32_t num_packs_to_send = 100; //sent by sender to reciever. default 100.
uint32_t num_packs_to_expect = 0; //reciever will set when it gets a value from sender 
uint32_t total_packs_recieved = 0; //count's reciever's total packets recieved

char* sendr_ip = "127.0.0.1"; //default to localhost
char* recvr_ip = "127.0.0.1"; //default to localhost

int port = 9876;
//END GLOABLS---------------

void set_args(int argc, char* argv[]) {

	while ((argc > 1) && (argv[1][0] == '-'))
	{
		switch (argv[1][1])
		{
			case 'r':
			r_or_s = 'r';
			printf("I will be reciever.\n");
			break;

			case 's':
			r_or_s = 's';
			printf("I will be sender.\n");
			break;

			case 'n':
			num_packs_to_send = atoi(&argv[1][2]);
			printf("I will send %d packets.\n", num_packs_to_send);
			break;

			default:
			printf("Bad Argument: %s\n", argv[1]);
		}

		++argv;
		--argc;
	}
}


//===============================START CORE SENDER
int send_number_tcp(int num_to_send)
{
	struct sockaddr_in reciever;
	int tcp_sock;

	if( (tcp_sock = socket(AF_INET, SOCK_STREAM , 0)) == -1 )
	{
		printf("Could not create socket");
		return -1;
	}

	//construct the information about the reciever we want to send data to
	reciever.sin_addr.s_addr = inet_addr(recvr_ip);
	reciever.sin_family = AF_INET;
	reciever.sin_port = htons( port );

	if( connect(tcp_sock, (struct sockaddr *)&reciever , sizeof(reciever)) < 0 )
	{
		perror("connect failed. Error");
		return -1;
	}

	uint32_t n = htonl(num_to_send);
	if( send(tcp_sock , &n , sizeof(uint32_t) , 0) < 0 )
	{
		puts("Send failed");
		return -1;
	}

	close(tcp_sock);
	return 0;
}


int send_pack_train_udp()
{
	struct sockaddr_in reciever;
	int udp_sock;
	char buf[MAX_BUF_SIZE] = "Hello World!";

	if ((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		puts("error on socket\n");
		return -1;
	}

	//construct the information about the reciever we want to send data to
	reciever.sin_addr.s_addr = inet_addr(recvr_ip);
	reciever.sin_family = AF_INET;
	reciever.sin_port = htons(port);

	int i;
	for(i = 0; i < num_packs_to_send; i++)
	{
		if (sendto(udp_sock, buf, MAX_BUF_SIZE, 0, (struct sockaddr *)&reciever, (socklen_t) sizeof(reciever)) == -1)
		{
			puts("sendto() error\n");
			return -1;
		}

		printf("%d sent.", i+1);
	}

	printf("\n");
	return 0;
}

int start_sender()
{
	if(send_number_tcp(num_packs_to_send) == -1)
	{
		puts("error sending num packs\n");
		return -1;
	}
	else
	{
		usleep(100);
		if(send_pack_train_udp() == -1) 
		{
			puts("error sending pack train\n");
			return -1;
		}
	}

	return 0;
}
//===============================END CORE SENDER


//===============================START CORE RECIEVER
int recv_number_tcp()
{
	int tcp_sock, client_sock, read_size;
	struct sockaddr_in reciever, sender;

	if( (tcp_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("Could not create socket");
	}

	reciever.sin_family = AF_INET;
	reciever.sin_addr.s_addr = htonl(INADDR_ANY);;
	reciever.sin_port = htons( port );

	if( bind(tcp_sock,(struct sockaddr *)&reciever , sizeof(reciever)) < 0)
	{
		perror("bind failed. Error");
		return -1;
	}

	listen(tcp_sock , 3);

	puts("Waiting for incoming connections...");

    //accept connection from an incoming sender
	socklen_t len = sizeof(struct sockaddr_in);
	client_sock = accept(tcp_sock, (struct sockaddr *)&sender, (socklen_t*)&len );
	if (client_sock < 0)
	{
		perror("accept failed");
		return -1;
	}

	puts("Connection made.");

	uint32_t num_packs_input;
    //Receive a message from sender
	while( (read_size = recv(client_sock , &num_packs_input, sizeof(uint32_t) , 0)) > 0 )
	{
		num_packs_to_expect = ntohl(num_packs_input);
		printf("Expecing %d packets.\n", num_packs_to_expect);
	}

	if(read_size == 0)
	{
		puts("Sender disconnected its TCP connection");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		perror("recv failed");
		return -1;
	}

	close(tcp_sock);
	return 0;
}

int recv_pack_train_udp()
{
	int udp_sock;
	char buf[MAX_BUF_SIZE];

	struct sockaddr_in reciever;
	struct sockaddr_storage addr; //gets filled with info of connector
	socklen_t fromlen = sizeof addr;

	if ((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		puts("error on socket\n");
		return -1;
	}

	reciever.sin_family = AF_INET;
	reciever.sin_port = htons(port);
	reciever.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(udp_sock, (struct sockaddr *)&reciever, sizeof(reciever)) == -1) 
	{
		puts("recv_pack_train_udp: bind error\n");
		return -1;
	}

	//set a timeout on the udp_sock. if no data is recieved within timeout, break waiting.
	struct timeval tv;
	tv.tv_sec = 5; //seconds
	tv.tv_usec = 0; //mircoseconds
	if (setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		perror("Error");
	}

	int i = 0;
	while(1) 
	{
		if (recvfrom(udp_sock, buf, MAX_BUF_SIZE, 0, (struct sockaddr *)&addr, &fromlen) < 0)
		{
			if(errno == EAGAIN || EWOULDBLOCK){
				break;
			}
			else {
				perror("err");
				return -1;
			}
		}
		else 
		{
			i++;
			printf("Received packet. Data %d: %s\n", i, buf);
		}
	}

	total_packs_recieved = i;
	return 0;
}

int start_reciever()
{
	if(recv_number_tcp() == -1)
	{
		puts("error sending num packs\n");
		return -1;
	}
	else
	{
		if(recv_pack_train_udp() == -1)
		{
			puts("error sending pack train\n");
			return -1;
		}
		else
		{
			printf("\nExpected: %d, Recieved: %d\n\n", num_packs_to_expect, total_packs_recieved);
		}	
	}

	return 0;
}
//===============================END CORE RECIEVER



int main(int argc, char* argv[]) 
{
	set_args(argc, argv);

	if(r_or_s == 'r')
	{
		if(start_reciever() == -1)
		{
			return -1;
		}
	}
	else
	{
		if(start_sender() == -1)
		{
			return -1;
		}
	}

	return 0;
}
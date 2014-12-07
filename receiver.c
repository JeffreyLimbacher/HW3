
#include "receiver.h"
#include <pthread.h>

#define ICMP_DEST_UNREACH 3
#define ICMP_PORT_UNREACH 3

void set_timeout(long int time_out, int sockfd);
int get_packet_type(char *buffer);
void get_icmp_header(char *buffer, struct icmp_hd *out); 



/**
 * This function will run on its own thread. The idea is that it will listen 
 * for the ICMP echo replies. Once it receives two echo replies, then it will
 * perform the calculations needed.
 */
void receiver(void *p_data) 
{
	struct pgrm_data data = *(struct pgrm_data *)(p_data);
	double first_echo_time = 0, second_echo_time = 0;

	// set the timeout of the socket to something reasonable
	set_timeout(100, data.sock_fd); 

	int sfd = data.sock_fd;
	struct sockaddr_in saddr;
	socklen_t addrlen = sizeof(saddr);

	//Stores the message we receive from the socket
	char recv_buf[MAX_IP_SIZE]; 
	int echo_recvd = 0;

	for( ; ; ) 
	{
		//recv_buf filled with an ip datagram
		if(recvfrom(sfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&saddr, &addrlen) < 0)
		{
			fprintf(stderr, "%s\n", strerror(errno)); //handle error;
			return;
		}

		if (get_packet_type(recv_buf) == IPPROTO_ICMP) 
		{
			struct icmp_hd hd;

			//pass the ip datagram's payload (&recv_buf[20]). hd will be the icmp header.
			get_icmp_header(&recv_buf[20], &hd); 

			if(hd.type == ICMP_ECHOREPLY)
			{
				//fprintf(stderr, "Received echo reply #%d\n", echo_recvd);
				echo_recvd++;
				
				if(echo_recvd == 1) //See what echo message # is this. 
				{
					first_echo_time = get_time();
				}
				 
				else if(echo_recvd == 2) // If it's the second, we are done. break.
				{
					second_echo_time = get_time();
					break;
				}
			}
			else if(hd.type == ICMP_DEST_UNREACH && hd.code == ICMP_PORT_UNREACH) {
				//Just for debugging...
				//fprintf(stderr, "Got port unreachable\n");
			}
		}

		memset(recv_buf, 0, MAX_IP_SIZE); //Just for safety for the next iteration
	}

	//calculate and print the time difference between the tail and head icmp pakets
	double time_difference = second_echo_time - first_echo_time;
	printf("\nCalculation: %c %f\n\n", data.p_args.entropy, time_difference);


	/*we thought the receiver might have to return data, but it can actually 
	just print from its own thread. so this return struct isn't needed...*/
	struct rcvr_return_data *return_data = malloc(sizeof(struct rcvr_return_data));
	return_data->status = 0; //return of 0 means ok
	pthread_exit(return_data);
}

// Sets the timeout to time_out on the socket. If we don't receive a message in
// time_out time, then recvfrom will error out.
void set_timeout(long int time_out, int sockfd) 
{
	struct timeval tv;
	tv.tv_sec = time_out;
	tv.tv_usec = 0;

	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)
	{
		fprintf(stderr, "%s %d\n", strerror(errno), sockfd);
	}
}

// Just returns what type of packet is stored in the buffer (assumed to be ip header).
int get_packet_type(char *buffer)
{
	return buffer[9]; //the 9th byte of an ip header is the protocol field 
}

//Moves the ICMP header in buffer to the icmp_hd. Only the checksum
//needs to be byte swapped.
void get_icmp_header(char *buffer, struct icmp_hd *out) 
{
	//ICMP packets have a fixed 8 byte header
	memcpy(out, buffer, 8); 
	out->checksum = ntohs(out->checksum);
}


#include "receiver.h"

#define ICMP_DEST_UNREACH 3
#define ICMP_PORT_UNREACH 3

void set_timeout(long int time_out, int sockfd);
int get_packet_type(char *buffer);

//Moves the ICMP header in buffer to the icmp_hd. Only the checksum
//needs to be byte swapped.
void get_icmp_header(char *buffer, struct icmp_hd *out) {
	memcpy(out, buffer, 8);
	out->checksum = ntohs(out->checksum);
}

/**
 * This function will run on its own thread. The idea is that it will listen 
 * for the ICMP echo replies. Once it receives two echo replies, then it will
 * perform the calculations needed.
 */
void receiver(void *p_data) {
	struct pgrm_data data = *(struct pgrm_data *)(p_data);

	double first_echo_time, second_echo_time;
	//Set the timeout on the socket to whatever
	set_timeout(100, data.sock_fd);

	int sfd = data.sock_fd;
	struct sockaddr_in saddr;
	socklen_t addrlen = sizeof(saddr);
	//Stores the message we receive from the socket
	char recv_buf[MAX_IP_SIZE];
	int echo_recvd = 0;

	for( ; ; ){
		if(recvfrom(sfd, recv_buf, sizeof(recv_buf), 0,
			(struct sockaddr *)&saddr, &addrlen) < 0){
			//handle error;
			fprintf(stderr, "%s\n", strerror(errno));
			return;
		}
		if (get_packet_type(recv_buf) == IPPROTO_ICMP) {
			struct icmp_hd hd;
			get_icmp_header(&recv_buf[20], &hd);
			fprintf(stderr, "%d\n", hd.type);
			if(hd.type == ICMP_ECHOREPLY){
				fprintf(stderr, "Got echo reply #%d\n", echo_recvd);
				echo_recvd++;
				//See what echo message # is this. 
				if(echo_recvd == 1){
					first_echo_time = get_time();
				}
				// If it's the second, we are done. 
				else if(echo_recvd == 2) {
					second_echo_time = get_time();
					break;
				}
			}
			else if(hd.type == ICMP_DEST_UNREACH &&
					hd.code == ICMP_PORT_UNREACH) {
				//Just for debugging...
				fprintf(stderr, "Got port unreachable\n");
			}
		}
		//Just for safety for the next iteration
		memset(recv_buf, 0, MAX_IP_SIZE);
	}

	struct rcvr_return_data *eg = malloc(sizeof(struct rcvr_return_data));
	eg->x = 777; //calculate time between first and second icmp...i think...
	pthread_exit(eg);
}
// Sets the timeout to time_out on the socket. If we don't receive a message in
// time_out time, then recvfrom will error out.
void set_timeout(long int time_out, int sockfd) {
	struct timeval tv;
	tv.tv_sec = time_out;
	tv.tv_usec = 0;
	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)
	{
		fprintf(stderr, "%s %d\n", strerror(errno), sockfd);
	}
}

// Just returns what time of packet is stored in the buffer (assumed to be ip header).
int get_packet_type(char *buffer){
	return buffer[9];
}

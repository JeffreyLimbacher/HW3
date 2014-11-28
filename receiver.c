
#include "receiver.h"

#define ICMP_DEST_UNREACH 3
#define ICMP_PORT_UNREACH 3

void set_timeout(long int time_out, int sockfd);
int get_packet_type(char *buffer);

void get_icmp_header(char *buffer, struct icmp_hd *out) {
	memcpy(out, buffer, 8);
	out->checksum = ntohs(out->checksum);
}

void receiver(void *p_data) {
	struct pgrm_data data = *(struct pgrm_data *)(p_data);

	double first_echo_time, second_echo_time;
	set_timeout(100, data.sock_fd);

	int sfd = data.sock_fd;
	struct sockaddr_in saddr;
	socklen_t addrlen = sizeof(saddr);
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
				fprintf(stderr, "Got echo reply\n");
				echo_recvd++;
				if(echo_recvd == 1){
					first_echo_time = get_time();
				}
				else if(echo_recvd == 2) {
					second_echo_time = get_time();
					break;
				}
			}
			else if(hd.type == ICMP_DEST_UNREACH &&
					hd.code == ICMP_PORT_UNREACH) {
				fprintf(stderr, "Got port unreachable\n");
			}
		}
		memset(recv_buf, 0, MAX_IP_SIZE);
	}
}

void set_timeout(long int time_out, int sockfd) {
	struct timeval tv;
	tv.tv_sec = time_out;
	tv.tv_usec = 0;
	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)
	{
		fprintf(stderr, "%s %d\n", strerror(errno), sockfd);
	}
}

int get_packet_type(char *buffer){
	return buffer[9];
}

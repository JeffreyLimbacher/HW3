
#ifndef INCLUDES_H
#define INCLUDES_H

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/icmp.h>

//This is going to store the user arguments
struct args{
	char *pgrm_name; //The program name passed into the command line
	char *host;
	int port;
	char entropy; //Should be 'H' or 'L'
	int payload_size; 
	int number_packets;
	int ttl;
	int packet_departure_time;
	int num_icmp_packets;
};

//Used to store other data besides the args that every process should know
//Can be expanded on later.
struct pgrm_data {
	struct args p_args;
	int sock_fd; //The file descriptor of the raw_socket
	struct sockaddr *dest_addr;
	struct sockaddr *source_addr;
};
#endif
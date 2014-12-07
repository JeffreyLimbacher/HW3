
#ifndef INCLUDES_H
#define INCLUDES_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in_systm.h>
#include <netinet/udp.h>
#include <errno.h>
#include <sys/time.h>


//This is going to store the user arguments
struct args{
	char *pgrm_name; //The program name passed into the command line
	// the host address
	char *host;
	// the port we want to use. most likely 9876 as assigned
	int port;
	// entropy used to calculate the priority of the packet. entropy level
	char entropy; //Should be 'H' or 'L'
	int payload_size; 
	// used to store the number of packets the end user will be sending
	int number_packets;
	// time-to-live of the communication
	int ttl;
	// what time did the packet leave?
	int packet_departure_time;
	// how many icmp packets we will be sending
	int num_icmp_packets;
};

//Used to store other data besides the args that every process should know
//Can be expanded on later.
struct pgrm_data {
	// the parameter arguments using the struct that we defined earlier
	struct args p_args;
	// the file descriptor of the raw_socket
	int sock_fd; //The file descriptor of the raw_socket
	// the destination address of the socket
	struct sockaddr_in *dest_addr;
	// the source address of the socket
	struct sockaddr_in *source_addr;
};

struct rcvr_return_data {
	int status;
	// other possible implementations for the receiver return data here
};

#endif

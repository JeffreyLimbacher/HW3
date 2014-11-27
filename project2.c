/*
*	COMP429 Fall 2014 Project Skeleton
*/

#include <pthread.h>

#include "includes.h"
#include "network.h"

void usage(){
	fprintf(stderr, "%s %s %s %s %s %s %s %s\n",
		"[hostname]",
		"[port]",
		"[H|L]",
		"[payload size]",
		"[number of udp packets]",
		"[TTL]",
		"[tail time seperation]",
		"[number of icmp packets]");
}

int processArgs(int argc, char **argv, struct args *params) {
	memset(params, 0, sizeof(params));

	params->pgrm_name = argv[0];
	params->host = argv[1];
	params->port = atoi(argv[2]);
	params->entropy = argv[3][0];
	params->payload_size = atoi(argv[4]);
	params->number_packets = atoi(argv[5]);
	params->ttl = atoi(argv[6]);
	params->packet_departure_time = atoi(argv[7]);
	params->num_icmp_packets = atoi(argv[8]);

	//TODO: input checks. This will probably never get done.
	return 0;
}

/**
 * This program will work by first 
 * 1. processing the args,
 * 2. creating the socket information and storing in the pgrm_data struct
 * 3. threading into the sender and receiver
 * 4. joining at the end to do the final calculations before exiting.
 */


int main (int argc, char **argv) {
	struct args params;
	if(argc != 9 || processArgs(argc, argv, &params)) {
		usage();
		return 0;
	}

	struct pgrm_data data;
	data.p_args = params;

	//Before multithreading, let's create sock_fd

	
	pthread_t recv_thread;
	int recv_ret;
	// if(recv_ret = pthread_create(&recv_thread, NULL, receiver_init, (void*)&params)) {
	// 	fprintf(stderr, "%s: %s", params.pgrm_name, strerror(errno));
	// 	return 1;
	// }

	send_thread(data);

	

    return 0; 
}
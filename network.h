
#ifndef NETWORK_H
#define NETWORK_H
#include "includes.h"

#define MAX_IP_SIZE 65535


struct icmp_hd{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint32_t data;
};

/**
 * Called before the multi threading to get the socket information and
 * pass it down to the sender and receiver threads. 
 */
int build_raw_sock(struct pgrm_data *out);

/**
 * Fills out the ip header using the pgrm data struct.
 * The caller is expected to know which fields to fill out.
 */
int fill_out_iphdr(const struct pgrm_data *in, 
					char protocol,
					char ttl,
					short int length,
					struct ip *out);

//Fills out the UDP header. This should always return the correct data.
int fill_out_udphdr(struct pgrm_data *in,
					short int len,
					struct udphdr *out);

int fill_out_icmphdr(int type, int code, struct icmp_hd *out);

int pack_icmp(const struct ip *iphd, const struct icmp_hd *icmphd, char *buffer);

int pack_udp(const struct ip *iphd, 
			 const struct udphdr *udphd, 
			 char *data, 
			 size_t len, 
			 char *buffer);

//From vahab's code. Moving the definition to another file
uint16_t ip_checksum(void* vdata, size_t length);

double get_time (void);

#endif
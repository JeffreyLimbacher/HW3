
#ifndef NETWORK_H
#define NETWORK_H
#include "includes.h"

/**
 * Called before the multi threading to get the socket information and
 * pass it down to the sender and receiver threads. 
 */
int buildRawSocket(struct pgrm_data *out);

/**
 * Fills out the ip header using the pgrm data struct.
 * The caller is expected to know which fields to fill out.
 */
int fillOutIpHeader(struct pgrm_data *in, struct ip *out);

//Fills out the UDP header. This should always return the correct data.
int fillOutUdpHeader(struct pgrm_data *in, struct udphdr *out);

//From vahab's code. Moving the definition to another file
uint16_t ip_checksum(void* vdata, size_t length);

#endif
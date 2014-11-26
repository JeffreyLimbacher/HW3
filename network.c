
#include "network.h"

//out->p_args is assumed to be valid here.
int buildRawSocket(struct pgrm_data *out) {
	int sd;
	if(socket(PF_INET, SOCK_RAW, IPPROTO_RAW)){
		return -1;
	}

	//TODO: return socket
}


int fillOutIpHeader(struct pgrm_data *in, struct ip *out){

}


int fillOutUdpHeader(struct pgrm_data *in, struct udphdr *out){

}

#include "network.h"

//out->p_args is assumed to be valid here.
int buildRawSocket(struct pgrm_data *out) {

	//source:http://sock-raw.org/papers/sock_raw
	int sd;
	if(socket(PF_INET, SOCK_RAW, IPPROTO_RAW) < 0){
		return errno;
	}

	int one = 1;
	if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
		return errno;
	}


	//destination information
	struct sockaddr_in *d_addr = 
		(struct sockaddr_in*)calloc(1, sizeof(sockaddr_in));
	struct args p = out->p_args;

	inet_pton(AF_INET, p.host, (struct in_addr*)&d_addr.sin_addr.s_addr);
	d_addr->sin_port = htons(p.port);
	d_addr->sin_family = AF_INET;

	out->dest_addr = d_addr;
	return 0;
}


int fillOutIpHeader(struct pgrm_data *in, 
					char protocol,
					char ttl,
					short int length,
					struct ip *out){
	out->tos = 4;
	out->tos = 5 << 4;
	out->tot_len = length;
	out->ttl = ttl;
	out->protocol = protocol;
	out->check = 0;
	out->saddr = 0; //Filled in when zero
	out->daddr = in->dest_addr.in_addr.s_addr;
	return 0;
}


int fillOutUdpHeader(struct pgrm_data *in,
					 short int len,
					 struct udphdr *out){
	out->dest_port = in->dest_addr.sin_port;
	out->source = 0; //flub this, I don't care
	out->len = len;
	return 0;
}

uint16_t ip_checksum(void* vdata,size_t length) {
    // Cast the data pointer to one that can be indexed.
    char* data=(char*)vdata;

    // Initialise the accumulator.
    uint64_t acc=0xffff;

    // Handle any partial block at the start of the data.
    unsigned int offset=((uintptr_t)data)&3;
    if (offset) {
        size_t count=4-offset;
        if (count>length) count=length;
        uint32_t word=0;
        memcpy(offset+(char*)&word,data,count);
        acc+=ntohl(word);
        data+=count;
        length-=count;
    }

    // Handle any complete 32-bit blocks.
    char* data_end=data+(length&~3);
    while (data!=data_end) {
        uint32_t word;
        memcpy(&word,data,4);
        acc+=ntohl(word);
        data+=4;
    }
    length&=3;

    // Handle any partial block at the end of the data.
    if (length) {
        uint32_t word=0;
        memcpy(&word,data,length);
        acc+=ntohl(word);
    }

    // Handle deferred carries.
    acc=(acc&0xffffffff)+(acc>>32);
    while (acc>>16) {
        acc=(acc&0xffff)+(acc>>16);
    }

    // If the data began at an odd byte address
    // then reverse the byte order to compensate.
    if (offset&1) {
        acc=((acc&0xff00)>>8)|((acc&0x00ff)<<8);
    }

    // Return the checksum in network byte order.
    return htons(~acc);
}

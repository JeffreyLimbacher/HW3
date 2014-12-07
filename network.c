
#include "network.h"

//out->p_args is assumed to be valid here.
int build_raw_sock(struct pgrm_data *out) {
	//source:http://sock-raw.org/papers/sock_raw
	int sd;

	// create a raw socket and check for failure
	if((sd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0){
		return errno;
	}

	int one = 1;
	
	// set socket options pointing to socket sd and check for failure
	if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
		return errno;
	}


	// destination information
	// 
	struct sockaddr_in *d_addr = 
		(struct sockaddr_in*)calloc(1, sizeof(struct sockaddr_in));
	struct args p = out->p_args;

	d_addr->sin_addr.s_addr = inet_addr(p.host);
	d_addr->sin_port = htons(p.port);
	d_addr->sin_family = AF_INET;

	out->dest_addr = d_addr;
	out->sock_fd = sd;
	return 0;
}

// method to enter information into the header
int fill_out_iphdr(const struct pgrm_data *in, 
					char protocol,
					char ttl,
					short int length,
					struct ip *out){
	out->ip_v = 4;
	out->ip_hl = 5;
	out->ip_len = htons(length);
	out->ip_ttl = ttl;
	out->ip_p = protocol;
	out->ip_sum = 0;
	memset(&(out->ip_src), 0, sizeof (out->ip_src)); //Filled in when zero
	out->ip_dst = in->dest_addr->sin_addr;
	return 0;
}


// method to enter information into the udp header
int fill_out_udphdr(const struct pgrm_data *in,
					short int len,
					struct udphdr *out){
	#ifdef HAVE_DUMB_UDPHDR
	out->dest = in->dest_addr->sin_port;
	out->source = 0; //flub this, I don't care
	out->len = htons(len);
	#else
	out->uh_dport = in->dest_addr->sin_port;
	out->uh_sport = in->dest_addr->sin_port;
	out->uh_ulen = htons(len);
	#endif
	return 0;
}


// method to enter information into the icmp header
int fill_out_icmphdr(int type, int code, struct icmp_hd *out) {
	memset(out, 0, sizeof(struct icmp_hd));
	out->type = type;
	out->code = code;
	out->checksum = ip_checksum((void *)out, sizeof(struct icmp_hd));

	return 0;
}


// enter information into the icmp
int pack_icmp(const struct ip *iphd, 
			  const struct icmp_hd *icmphd, 
			  char *buffer) {
	int ptr_idx = 0;
	memcpy(buffer + ptr_idx, iphd, sizeof(struct ip));
	ptr_idx += sizeof(struct ip);
	memcpy(buffer + ptr_idx, icmphd, sizeof(struct icmp_hd));
	ptr_idx += sizeof(struct icmp);
	return ptr_idx;
}


//  enter information into the udp
int pack_udp(const struct ip *iphd, 
			 const struct udphdr *udphd, 
			 char *data, 
			 size_t len, 
			 char *buffer){
	int ptr_idx = 0;
	memcpy(buffer + ptr_idx, iphd, sizeof(struct ip));
	ptr_idx += sizeof(struct ip);
	memcpy(buffer + ptr_idx, udphd, sizeof(struct udphdr));
	ptr_idx += sizeof(struct udphdr);
	memcpy(buffer + ptr_idx, data, len);
	ptr_idx += len;
	return ptr_idx;
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

/*  returns current time as double, with most possible precision  */
double get_time (void) {
	struct timeval tv;
	double d;
	gettimeofday (&tv, NULL);
	d = ((double) tv.tv_usec) / 1000000 + (unsigned long) tv.tv_sec;
	return d;
}



#include "sender.h"

int make_random(char *buffer, size_t size);
int send_message(const struct pgrm_data data, const char *buffer, size_t len);
int send_udp_train(const struct pgrm_data data);
int send_icmp(const struct pgrm_data data);


// main part of the sender file
void sender(struct pgrm_data data) {
	send_icmp(data);

	double time_stamp = get_time();

	send_udp_train(data);
}


int make_random(char *buffer, size_t size) {

	//I'm using dev/urandom because dev/random is way too slow on my computer
	int random = open("/dev/urandom", O_RDONLY);
	int bytes_read = 0;
	while(bytes_read < size) {
		int temp = read(random, &(buffer[bytes_read]), size);
		if(temp < 0) {
			return errno;
		}
		bytes_read += temp;
	}
	return 0;
}


// send icmp through
int send_icmp(const struct pgrm_data data){
	int pack_size = sizeof(struct ip) + sizeof(struct icmp_hd);
	char buffer[sizeof(struct ip) + sizeof(struct icmp_hd)];
	memset(buffer, 0, sizeof(buffer));
	struct ip *iphdr = (struct ip*)buffer;
	struct icmp_hd *icmphdr = 
		(struct icmp_hd*)(buffer + sizeof(struct ip));

	fill_out_iphdr(&data, IPPROTO_ICMP, 255, pack_size, iphdr);

	fill_out_icmphdr(ICMP_ECHO, 0, icmphdr);

	send_message(data, buffer, pack_size);
	return 0;
}

// send the message
int send_message(const struct pgrm_data data, const char *buffer, size_t len){
	int sfd = data.sock_fd;
	socklen_t sock_size = sizeof(struct sockaddr_in);
	if(sendto(sfd, buffer, len, 0, 
		(struct sockaddr *) data.dest_addr, sock_size) < 0) {
		return -1;
	}
	return 0;
}


// begin the udp train
int send_udp_train(const struct pgrm_data data){
	struct args p = data.p_args;
	char *udp_buffer = (char *)calloc(p.number_packets, p.payload_size);
	if(data.p_args.entropy == 'H') {
		if(make_random(udp_buffer, p.number_packets * p.payload_size)){
			fprintf(stderr, "%s: %s\n", p.pgrm_name, strerror(errno));
		}
	}

	short int tot_len = p.payload_size + sizeof(struct ip) + sizeof(struct udphdr);
	char *buffer = (char *)calloc(tot_len, 1);
	struct ip *iphd = (struct ip *)buffer;
	struct udphdr *udphd = (struct udphdr *)(buffer + sizeof(struct ip));
	char *udp_payload = (char *)udphd + sizeof(struct udphdr);
	fill_out_iphdr(&data, IPPROTO_UDP, data.p_args.ttl, tot_len, iphd);
	int udp_size = p.payload_size + sizeof(struct udphdr);
	fill_out_udphdr(&data, udp_size, udphd);
	int i;
	for(i = 0; i < p.number_packets; i++){
		memcpy(udp_payload, udp_buffer + i*p.payload_size, p.payload_size);
		udphd->check = ip_checksum(udphd, udp_size);
		send_message(data, buffer, tot_len);
	}

	return 0;
}

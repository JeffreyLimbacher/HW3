
#include "sender.h"

#include "network.h"

int make_random(char *buffer, size_t size);
int send_message(const struct pgrm_data data, const char *buffer, size_t len);
int send_udp_train(const struct pgrm_data data);

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

int send_icmp(const struct pgrm_data data){
	int pack_size = sizeof(struct ip) + sizeof(struct icmp_hd);
	char buffer[sizeof(struct ip) + sizeof(struct icmp_hd)];
	struct ip iphdr;
	int len = sizeof(struct ip) 
			+ sizeof(struct icmp_hd) 
			+ sizeof(data.p_args.payload_size);
	fill_out_iphdr(&data, IPPROTO_ICMP, 255, len, &iphdr);

	struct icmp_hd icmphd;
	fill_out_icmphdr(ICMP_ECHO, 0, &icmphd);
	pack_icmp(&iphdr, &icmphd, buffer);

	send_message(data, buffer, pack_size);
	return 0;
}

int send_message(const struct pgrm_data data, const char *buffer, size_t len){
	int sfd = data.sock_fd;
	socklen_t sock_size = sizeof(struct sockaddr_in);
	if(sendto(sfd, buffer, len, 0, 
		(struct sockaddr *) data.dest_addr, sock_size) < 0) {
		return -1;
	}
	return 0;
}

int send_udp_train(const struct pgrm_data data){
	struct args p = data.p_args;
	char *udp_buffer = (char *)calloc(p.number_packets, p.payload_size);
	if(data.p_args.entropy == 'H') {
		if(make_random(udp_buffer, p.number_packets * p.payload_size)){
			fprintf(stderr, "%s: %s\n", p.pgrm_name, strerror(errno));
		}
	}

	struct ip iphd;
	struct udphdr udphd;
	fill_out_iphdr(&data, IPPROTO_UDP, data.p_args.ttl, p.payload_size, &iphd);
}

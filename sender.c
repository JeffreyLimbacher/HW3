
#include "sender.h"

#include "network.h"

void send_thread(struct pgrm_data data) {
	build_raw_sock(&data);
}
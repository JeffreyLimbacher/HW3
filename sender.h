
#ifndef SENDER_H
#define SENDER_H

#include "includes.h"

#include <sys/stat.h>
#include <fcntl.h>

void send_thread(struct pgrm_data data);

#endif

#ifndef SENDER_H
#define SENDER_H

#include "includes.h"
#include "network.h"

#include <sys/stat.h>
#include <fcntl.h>

/**
 * @param data
 **/
void sender(struct pgrm_data data);

#endif

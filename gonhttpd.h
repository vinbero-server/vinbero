#ifndef _GONHTTPD_H
#define _GONHTTPD_H

#include "gonhttpd_parent.h"

void gonhttpd_start(size_t child_count, struct gonhttpd_socket_list* child_socket_list);

#endif

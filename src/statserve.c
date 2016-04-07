#include <stdio.h>
#include <ctype.h>
#include "dbg.h"

int echo_server(const char *host, const char *port)
{
	check(host != NULL, "Invalid host."); 
	check(port != NULL, "Invalid port."); 
	
	// Create a sigaction that handles SIGCHLD
	// listen on the given port and host

	while(1) {
		// accept a client connection 
		// fork process
		// if 0 then child process
		// else server
	}

error: // fallthrough
    return -1;
}

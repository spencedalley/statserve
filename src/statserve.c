#include <stdio.h>
#include <ctype.h>
#include <lcthw/dbg.h> 
#include <signal.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <sys/wait.h> 
 
	
void handle_sigchild(int sig) {
	while(waitpid(-1, NULL, WNOHANG) > 0) { }
} 

int echo_server(const char *host, const char *port)
{
	int rc = 0; 
		
	struct sigaction sa = {
		.sa_handler = handle_sigchild, 
		.sa_flags = SA_RESTART | SA_NOCLDSTOP 
	}; 

	check(host != NULL, "Invalid host."); 
	check(port != NULL, "Invalid port."); 

	
	// Create a sigaction that handles SIGCHLD
	sigemptyset(&sa.sa_mask); 
	rc = sigaction(SIGCHLD, &sa, 0);
	check(rc != -1, "Failed to setup signal handler for child process.");  
	
	// Create a sigaction that handles SIGCHLD
	// listen on the given port and host

	//while(1) {
		// accept a client connection 
		// fork process
		// if 0 then child process
		// else server
	//}

error: // fallthrough
    return -1;
}

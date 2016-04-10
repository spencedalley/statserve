#include <stdio.h>
#include <ctype.h>
#include <lcthw/dbg.h> 
#include <signal.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <sys/wait.h>
#include <netdb.h> 
#include <fcntl.h>  
 
	
void handle_sigchild(int sig) {
	// prevent child processes from leaking
	while(waitpid(-1, NULL, WNOHANG) > 0) { }
} 

int echo_server(const char *host, const char *port)
{
	int rc = 0;

	int server_socket = 0; 
	int client_fd = 0; 
	struct sockaddr_in client_addr; 
	socklen_t sin_size = sizeof(client_addr); 
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

	// listen on the given port and host
	server_socket = server_listen(host, port); 
	check(server_socket >= 0, "bind to %s:%s failed.", host, port); 

	while(1) {
		client_fd = accept(server_socket, (struct sockaddr *)&client_addr, &sin_size);  
		
		check(client_fd >= 0, "Failed to accept connection."); 
		debug("Client connected.");

		 
	
		// fork 
		// if 0 then child
		// else server
		}

error: // fallthrough
    return -1;
}

#include <stdio.h>
#include <ctype.h>
#include <lcthw/dbg.h> 
#include <lcthw/ringbuffer.h> 
#include <signal.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <sys/wait.h>
#include <netdb.h> 
#include <fcntl.h>
#include "net.h"  

const int RB_SIZE = 1024 * 10;   
	
void handle_sigchild(int sig) {
	// prevent child processes from leaking
	while(waitpid(-1, NULL, WNOHANG) > 0) { }
} 

void client_handler(int client_fd) 
{
	int rc = 0; 
	
	// need a ringbuffer for the input
	RingBuffer *sock_rb = RingBuffer_create(RB_SIZE);
	
	while(read_some(sock_rb, client_fd, 1) != -1) {
		if(write_some(sock_rb, client_fd, 1) == -1) {
			debug("Client closed."); 
			break; 
		}
	}

	// close the socket
	rc = close(client_fd); 
	check(rc != -1, "Failed to close the socket.");  

error:
//	if(sock_rb) Ringbuffer_destroy(sock_rb);  
	exit(0); // just exit child
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

		rc = fork(); 
		if(rc == 0) {
			client_handler(client_fd);
			// handle the client 
		} else {
			// server process
			close(client_fd); // don't need this 
		}
	
	}

error: // fallthrough
    return -1;
}

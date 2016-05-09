#include <stdio.h>
#include <ctype.h>
#include <lcthw/bstrlib.h>
#include <lcthw/dbg.h>
#include <lcthw/hashmap.h>
#include <lcthw/ringbuffer.h>
#include <lcthw/stats.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <netdb.h>
#include <fcntl.h>
#include "net.h"

struct tagbstring LINE_SPLIT = bsStatic(" ");
struct tagbstring CREATE = bsStatic("create");
struct tagbstring MEAN = bsStatic("mean");
struct tagbstring SAMPLE = bsStatic("sample");
struct tagbstring DUMP = bsStatic("dump");
struct tagbstring OK = bsStatic("OK\n");
struct tagbstring ERR = bsStatic("ERR\n");
struct tagbstring DNE = bsStatic("DNE\n");
struct tagbstring EXISTS = bsStatic("EXISTS\n");
const char LINE_ENDING = '\n';

const int RB_SIZE = 1024 * 10;

Hashmap *DATA = NULL;

// Pre-declare the Command exists
struct Command;

typedef int (*handler_cb)(struct Command *cmd, RingBuffer *send_rb);

typedef struct Command {
    bstring command;
    bstring name;
    bstring number;
    handler_cb handler;
}  Command;

typedef struct Record {
    bstring name;
    Stats *stat;
} Record;

void handle_sigchild(int sig) {
	// prevent child processes from leaking
	while(waitpid(-1, NULL, WNOHANG) > 0) { }
}

void send_reply(RingBuffer *send_rb, bstring reply)
{
    RingBuffer_puts(send_rb, reply);
}

int handle_create(Command *cmd, RingBuffer *send_rb)
{
    int rc = 0;
    // if the name is in the DATA map then return exists
    // else create the stats record and store it
    if(Hashmap_get(DATA, cmd->name)) {
        send_reply(send_rb, &EXISTS);
    } else {
        // allocate record
        debug("create: %s %s", bdata(cmd->name), bdata(cmd->number));

        Record *info = calloc(sizeof(Record), 1);
        check_mem(info);

        info->stat = Stats_create();
        check_mem(info->stat);

        info->name = bstrcpy(cmd->name);
        check_mem(info->name);

        Stats_sample(info->stat, atof(bdata(cmd->number)));

        rc = Hashmap_set(DATA, info->name, info);
        check(rc == 0, "Failed to add data to map.");

        send_reply(send_rb, &OK);
        // set its stat element
        // set its name element
        // do a first sample
        // add it to teh hashmap
    }

    return 0;
error:
    return -1;
}

int handle_sample(Command *cmd, RingBuffer *send_rb)
{
    Record *info = Hashmap_get(DATA, cmd->name); 

    if(info == NULL) {
        send_reply(send_rb, &DNE); 
    } else{
        Stats_sample(info->stat, atof(bdata(cmd->number))); 
        bstring reply = bformat("%f\n", Stats_mean(info->stat)); 
        send_reply(send_rb, reply); 
        bdestroy(reply); 
    }

    return 0; 
}

int parse_command(struct bstrList *splits, Command *cmd)
{
    cmd->command = splits->entry[0];

    if(biseq(cmd->command, &CREATE)) {
        cmd->name = splits->entry[1];
        cmd->number = splits->entry[2];
        cmd->handler = handle_create;
    } else if(biseq(cmd->command, &SAMPLE)) {
        cmd->name = splits->entry[1]; 
        cmd->number = splits->entry[2]; 
        cmd->handler = handle_sample;
    } else {
        sentinel("Failed to parse the command.");
    }

    return 0;
error:
    return -1;
}

int parse_line(bstring data, RingBuffer *send_rb)
{
    int rc = -1;
    Command cmd = {.command = NULL};

    // split data on line boundaries
    struct bstrList *splits = bsplits(data, &LINE_SPLIT);
    check(splits != NULL, "Bad data");
    // parse it into a command
    rc = parse_command(splits, &cmd);
    check(rc == 0, "Failed to parse command");

    // call the command handler for that command
    rc = cmd.handler(&cmd, send_rb);
    // destroy the splits

error:
    debug("RC: %d", rc); 
    if(splits) bstrListDestroy(splits);
    return rc;
}

void client_handler(int client_fd)
{
	int rc = 0;
    RingBuffer *recv_rb = RingBuffer_create(RB_SIZE);
    RingBuffer *send_rb = RingBuffer_create(RB_SIZE); 

    check_mem(recv_rb);
    check_mem(send_rb); 

    while(read_some(recv_rb, client_fd, 1) != -1) {
        // read a line from recv_rb
        bstring data = read_line(recv_rb, LINE_ENDING); 
        check(data != NULL, "Client closed."); 
        // parse it, close on protocol error
        rc = parse_line(data, send_rb); 
        bdestroy(data); 
        check(rc == 0, "Failed to parse user. Closing.");
        // and as long as there's something to send, send it
        if(RingBuffer_available_data(send_rb)) {
            write_some(send_rb, client_fd, 1); 
        }
    } 

	// close the socket
	rc = close(client_fd);
	check(rc != -1, "Failed to close the socket.");

error:
	if(recv_rb) RingBuffer_destroy(recv_rb);
    if(send_rb) RingBuffer_destroy(send_rb);
	exit(0); // Exit child process
}

int setup_data_store()
{
    DATA = Hashmap_create(NULL, NULL);
    check_mem(DATA);

    return 0;

error:
    return -1;
}

int echo_server(const char *host, const char *port)
{
	int rc = 0;

	int server_socket = 0;
	int client_fd = 0;
	struct sockaddr_in client_addr;
	socklen_t sin_size = sizeof(client_addr);

    rc = setup_data_store();
    check(rc == 0, "Failed to setup the data store."); 

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

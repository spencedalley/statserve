#ifndef _statserve_h
#define _statserve_h

#include <lcthw/bstrlib.h>
#include <lcthw/ringbuffer.h>
#include <lcthw/stats.h> 

typedef struct LineTest {
    char *line;
    bstring result;
    char *description;
} LineTest;

struct Command; 

struct tagbstring OK;

int setup_data_store();

int attempt_line(LineTest test); 

int parse_line(bstring data, RingBuffer *send_rb);

int echo_server(const char *host, const char *port);

#endif

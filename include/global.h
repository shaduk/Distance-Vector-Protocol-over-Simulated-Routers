#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

/* a comment */

uint32_t MY_IP_ADDRESS;
uint16_t CONTROL_PORT;
fd_set master_list, watch_list;
int head_fd;
uint16_t nodes;
uint16_t MY_ROUTER_PORT;
uint16_t interval;
struct timeval tv;
int initDone;

typedef enum {FALSE, TRUE} bool;

#define ERROR(err_msg) {perror(err_msg); exit(EXIT_FAILURE);}

/* https://scaryreasoner.wordpress.com/2009/02/28/checking-sizeof-at-compile-time/ */
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)])) // Interesting stuff to read if you are interested to know how this works
#endif

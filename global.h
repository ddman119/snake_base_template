
#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <iostream>
#include <cstdio>
#include <string>   //strlen
#include <string.h>   //strlen
#include <cstdlib>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#define MAX_BUFFER 	1024
#define MAX_LEN		64
#define MAX_PENDING	3
#define MAX_PLAYER	4
#define TRUE   		1
#define FALSE  		0
#define PORT 		8080

#define DISP_WIDTH	800
#define DISP_Height	600

#define	LOGIN			0x01
#define USER_MSG 		0x02

#endif


#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <iostream>
#include <cstdio>
#include <string>   //strlen
#include <string.h>   //strlen
#include <cstdlib>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#define MAX_BUFFER 	1024
#define MAX_PATH	256
#define MAX_LEN		64
#define MAX_PENDING	3
#define MAX_PLAYER	4
#define TRUE   		1
#define FALSE  		0
#define PORT 		8080

#define DISP_WIDTH	800
#define DISP_HEIGHT	600

#define	LOGIN			0x01
#define USER_MSG 		0x02
#define START			0x03
#define END 			0x04
#define USER_KEY		0x05
#define TIME_SYNC		0x06
#define FOOD			0x07

struct POSITION
{
	int xpos;
	int ypos;
};
#endif

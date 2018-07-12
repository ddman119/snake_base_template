#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_BUFFER_SIZE 1024

static int serversocketfd;

void exit_signal(int sig_unused)
{
    if(write(serversocketfd, "/exit\n", MAX_BUFFER_SIZE - 1) == -1)
        perror("Error in exit_signal: ");

    close(serversocketfd);
    exit(1);
}

void setNonBlock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    if(flags < 0)
        perror("Error in setNonBlock:");

    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}


void connectToServer(struct sockaddr_in *serverAddress, struct hostent *host, int serversocketfd, long port)
{
    memset(serverAddress, 0, sizeof(serverAddress));
    serverAddress->sin_family = AF_INET;
    serverAddress->sin_addr = *((struct in_addr *)host->h_addr_list[0]);
    serverAddress->sin_port = htons(port);
    if(connect(serversocketfd, (struct sockaddr *) serverAddress, sizeof(struct sockaddr)) < 0)
    {
        perror("Connection Failed. Try again.");
        exit(1);
    }
}

void mainLoop(char *name, int serversocketfd)
{
    fd_set clientsocketFd;
    char clientData[MAX_BUFFER_SIZE];
    char dataBuffer[MAX_BUFFER_SIZE], message[MAX_BUFFER_SIZE];

    while(1)
    {
        FD_ZERO(&clientsocketFd);
        FD_SET(serversocketfd, &clientsocketFd);
        FD_SET(0, &clientsocketFd);
        if(select(FD_SETSIZE, &clientsocketFd, NULL, NULL, NULL) != -1) 
        {
            for(int fd = 0; fd < FD_SETSIZE; fd++)
            {
                if(FD_ISSET(fd, &clientsocketFd))
                {
                    if(fd == serversocketfd) 
                    {
                        int numBytesRead = read(serversocketfd, message, MAX_BUFFER_SIZE - 1);
                        message[numBytesRead] = '\0';
                        printf("%s", message);
                        memset(&message, 0, sizeof(message));
                    }
                    else if(fd == 0) 
                    {
                        fgets(dataBuffer, MAX_BUFFER_SIZE - 1, stdin);
                        if(strcmp(dataBuffer, "/exit\n") == 0)
                            exit_signal(-1); 
                        else
                        {
                            memset(clientData, 0, MAX_BUFFER_SIZE);
							strcpy(clientData, name);
							strcat(clientData, ": ");
							strcat(clientData, dataBuffer);
                            if(write(serversocketfd, clientData, MAX_BUFFER_SIZE - 1) == -1)
                            	 perror("Failed to write in mainLoop: ");
                            memset(&dataBuffer, 0, sizeof(dataBuffer));
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    char *name;
    struct sockaddr_in serverAddress;
    struct hostent *host;
    long port;

    if(argc != 4)
    {
        fprintf(stderr, "./client [username] [host] [port]\n");
        exit(1);
    }
    name = argv[1];
    if((host = gethostbyname(argv[2])) == NULL)
    {
        fprintf(stderr, "Couldn't get host name\n");
        exit(1);
    }
    port = strtol(argv[3], NULL, 0);
    if((serversocketfd = socket(AF_INET, SOCK_STREAM, 0))== -1)
    {
        fprintf(stderr, "Couldn't create socket\n");
        exit(1);
    }

    connectToServer(&serverAddress, host, serversocketfd, port);
    setNonBlock(serversocketfd);
    setNonBlock(0);

    signal(SIGINT, exit_signal);

    mainLoop(name, serversocketfd);
}


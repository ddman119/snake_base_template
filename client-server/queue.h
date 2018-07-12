#ifndef QUEUE
#define QUEUE

#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include <stdlib.h>


#define MAX_BUFFER_SIZE 1024


struct queue {
    char *buffer[MAX_BUFFER_SIZE];
    int head;
    int tail;
    int isFull;
    int isEmpty;
    pthread_mutex_t *mutex;
    pthread_cond_t *notFull, *notEmpty;
};

// Used in server.c
struct clientDataSet{
    fd_set serverReadFds;
    int socketFd;
    int clientSockets[MAX_BUFFER_SIZE];
    int numClients;
    pthread_mutex_t *clientListMutex;
    struct queue *queue;
};

struct clientSocketData{
    struct clientDataSet *data;
    int clientSocketFd;
};

#endif // End of Queue.h guard.
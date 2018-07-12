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
#include <pthread.h>
#include <queue.c>


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

void removeClient(struct clientDataSet *data, int clientSocketFd)
{
    pthread_mutex_lock(data->clientListMutex);
    for(int i = 0; i < MAX_BUFFER_SIZE; i++)
    {
        if(data->clientSockets[i] == clientSocketFd)
        {
            data->clientSockets[i] = 0;
            close(clientSocketFd);
            data->numClients--;
            i = MAX_BUFFER_SIZE;
        }
    }
    pthread_mutex_unlock(data->clientListMutex);
}

void *handleClient(void *chv)
{
    struct clientSocketData *vars = (struct clientSocketData *)chv;
    struct clientDataSet *data = (struct clientDataSet *)vars->data;

    struct queue *q = data->queue;
    int clientSocketFd = vars->clientSocketFd;

    char msgBuffer[MAX_BUFFER_SIZE];
    while(1)
    {
        int numBytesRead = read(clientSocketFd, msgBuffer, MAX_BUFFER_SIZE - 1);
        msgBuffer[numBytesRead] = '\0';

        
        if(strcmp(msgBuffer, "/exit\n") == 0)
        {
            fprintf(stderr, "Client has disconnected\n", clientSocketFd);
            removeClient(data, clientSocketFd);
            return NULL;
        }
        else
        {
            
            while(q->isFull)
            {
                pthread_cond_wait(q->notFull, q->mutex);
            }

            
            pthread_mutex_lock(q->mutex);
            fprintf(stderr, "Pushing message to queue: %s\n", msgBuffer);
            queuePush(q, msgBuffer);
            pthread_mutex_unlock(q->mutex);
            pthread_cond_signal(q->notEmpty);
        }
    }
}


void *newConnection(void *data)
{
    struct clientDataSet *clientData = (struct clientDataSet *) data;
    while(1)
    {
        int clientSocketFd = accept(clientData->socketFd, NULL, NULL);
        if(clientSocketFd > 0)
        {
            fprintf(stderr, "Server accepted new client. Socket: %d\n", clientSocketFd);


            pthread_mutex_lock(clientData->clientListMutex);
            if(clientData->numClients < MAX_BUFFER_SIZE)
            {

                for(int i = 0; i < MAX_BUFFER_SIZE; i++)
                {
                    if(!FD_ISSET(clientData->clientSockets[i], &(clientData->serverReadFds)))
                    {
                        clientData->clientSockets[i] = clientSocketFd;
                        i = MAX_BUFFER_SIZE;
                    }
                }

                FD_SET(clientSocketFd, &(clientData->serverReadFds));

                struct clientSocketData chv;
                chv.clientSocketFd = clientSocketFd;
                chv.data = clientData;

                pthread_t clientThread;
                if((pthread_create(&clientThread, NULL, (void *)&handleClient, (void *)&chv)) == 0)
                {
                    clientData->numClients++;
                    fprintf(stderr, "Client has joined. %d\n", clientSocketFd);
                }
                else
                    close(clientSocketFd);
            }
            pthread_mutex_unlock(clientData->clientListMutex);
        }
    }
}


void *handleData(void *data)
{
    struct clientDataSet *clientData = (struct clientDataSet *)data;
    struct queue *q = clientData->queue;
    int *clientSockets = clientData->clientSockets;

    while(1)
    {
        
        pthread_mutex_lock(q->mutex);
        while(q->isEmpty)
        {
            pthread_cond_wait(q->notEmpty, q->mutex);
        }
        char* msg = queuePop(q);
        pthread_mutex_unlock(q->mutex);
        pthread_cond_signal(q->notFull);

        
        fprintf(stderr, "%s\n", msg);
        for(int i = 0; i < clientData->numClients; i++)
        {
            int socket = clientSockets[i];
            if(socket != 0 && write(socket, msg, MAX_BUFFER_SIZE - 1) == -1)
                perror("Socket write failed: ");
        }
    }
}


void initiateConnection(int socketFd)
{
    struct clientDataSet data;
    data.numClients = 0;
    data.socketFd = socketFd;
    data.queue = queueInit();
    data.clientListMutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(data.clientListMutex, NULL);

    
    pthread_t connectionThread;
    if((pthread_create(&connectionThread, NULL, (void *)&newConnection, (void *)&data)) == 0)
    {
        fprintf(stderr, "Connection handler started\n");
    }

    FD_ZERO(&(data.serverReadFds));
    FD_SET(socketFd, &(data.serverReadFds));

   
    pthread_t messagesThread;
    if((pthread_create(&messagesThread, NULL, (void *)&handleData, (void *)&data)) == 0)
    {
        fprintf(stderr, "Message handler started\n");
    }

    pthread_join(connectionThread, NULL);
    pthread_join(messagesThread, NULL);

    queueDestroy(data.queue);
    pthread_mutex_destroy(data.clientListMutex);
    free(data.clientListMutex);
}

void bindSocket(struct sockaddr_in *serverAddress, int socketFd, long port)
{
    memset(serverAddress, 0, sizeof(*serverAddress));
    serverAddress->sin_family = AF_INET;
    serverAddress->sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress->sin_port = htons(port);

    if(bind(socketFd, (struct sockaddr *)serverAddress, sizeof(struct sockaddr_in)) == -1)
    {
        perror("Socket bind failed: ");
        exit(1);
    }
}


int main(int argc, char *argv[])
{
    struct sockaddr_in serverAddress;
    long port = 9999;
    int socketFd;

    if(argc == 2) port = strtol(argv[1], NULL, 0);

    if((socketFd = socket(AF_INET, SOCK_STREAM, 0))== -1)
    {
        perror("Socket creation failed");
        exit(1);
    }

    bindSocket(&serverAddress, socketFd, port);
    if(listen(socketFd, 1) == -1)
    {
        perror("listen failed: ");
        exit(1);
    }

    initiateConnection(socketFd);

    close(socketFd);
}
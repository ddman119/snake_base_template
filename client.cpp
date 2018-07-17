#include "client.h"

void *client_main_thread_function(void *arg);
void *client_msg_thread_function(void *arg);
void *client_key_thread_function(void *arg);
void *client_time_thread_function(void *arg);

void clientMessageRequest(int sock, char *msg);

client *_pClient;

client::client(int sock, int id)
	: m_nClientSock(sock), m_nId(id)
{
	_pClient = this;
}

bool client::startClient()
{
	int res;
    m_bIsRunning = true;
	res = pthread_create(&m_pMainThread,NULL, client_main_thread_function,NULL);
    if(res != 0)
    {
        perror("Main thread creation failed");
        return false;        
    }
    
    res = pthread_create(&m_pKeyThread,NULL, client_key_thread_function,NULL);
    if(res != 0)
    {
        perror("Key thread creation failed");
        return false;
    }
    
    res = pthread_create(&m_pTimeThread,NULL, client_time_thread_function,NULL);
    if(res != 0)
    {
        perror("Time thread creation failed");
        return false;
    }

    res = pthread_create(&m_pMsgThread,NULL, client_msg_thread_function,NULL);
    if(res != 0)
    {
        perror("Message thread creation failed");
        return false;
    }

    return true;    
}

bool client::waitThread()
{
	int res;
    void *thread_result;
	res = pthread_join(m_pMainThread, &thread_result);
    if(res != 0)
    {
        perror("Main thread join failed");
        return false;
    }
    res = pthread_join(m_pKeyThread, &thread_result);
    if(res != 0)
    {
        perror("Key thread join failed");
        return false;
    }
    res = pthread_join(m_pTimeThread, &thread_result);
    if(res != 0)
    {
        perror("Time thread join failed");
        return false;
    }
    res = pthread_join(m_pMsgThread, &thread_result);
    if(res != 0)
    {
        perror("Message thread join failed");
        return false;
    }
    return true;
}

void *client_main_thread_function(void *arg)
{
    int new_socket , activity, i , valread , sd;
    int max_sd;
    
    char buffer[1025];  //data buffer of 1K

    //set of socket descriptors
    fd_set readfds;
        
    while(_pClient->m_bIsRunning)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(_pClient->m_nClientSock, &readfds);        

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select( _pClient->m_nClientSock + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(_pClient->m_nClientSock, &readfds))
        {            
            if ((valread = read( _pClient->m_nClientSock , buffer, MAX_BUFFER)) == 0)
            {
                close(_pClient->m_nClientSock);
                _pClient->m_bIsRunning = false;
            }         
            else
            {
                int packet_len = *(int *)buffer;
                int msg_code = *(int *)(buffer + sizeof(int));
                if (msg_code == USER_MSG)
                {
                    char *msg = (char *)calloc(packet_len - 2 * sizeof(int), 1);
                    memcpy(msg, buffer + sizeof(int) * 2, packet_len - 2 * sizeof(int));
                    printf("Message from server : %s\n", msg);
                    free(msg);
                }
            }   
        }        
    }
}

void *client_key_thread_function(void *arg)
{
    
}

void *client_time_thread_function(void *arg)
{
    
}

void *client_msg_thread_function(void *arg)
{
    while (_pClient->m_bIsRunning)
    {
        char msg[MAX_BUFFER];
        if (fgets(msg, MAX_BUFFER, stdin) != NULL)
        {
            clientMessageRequest(_pClient->m_nClientSock, msg);
        }
    }
}

void clientMessageRequest(int sock, char *msg)
{
    int packet_len = (1 + 1) * sizeof(int) + strlen(msg);
    char *packet = (char *)calloc(packet_len, sizeof(char));
    *(int *)packet = packet_len;
    *(int *)(packet + 4) = USER_MSG;
    strcpy(packet + 8, msg);    
    send(sock, packet, packet_len, 0);   
    free(packet);
}

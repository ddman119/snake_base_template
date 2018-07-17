#include "server.h"

void *server_main_thread_function(void *arg);
void *server_msg_thread_function(void *arg);
void *server_key_thread_function(void *arg);
void *server_time_thread_function(void *arg);

void serverMessageRequest(int sock, char *msg);
void loginReply(int sock, int id);

server *_pServer;

server::server(int serverSock, struct sockaddr_in serverAddr)
	: m_nServerSock(serverSock), m_ServerAddr(serverAddr)
{
	_pServer = this;	
}

bool server::startServer()
{
	int res;
	m_bIsRunning = true;
	res = pthread_create(&m_pMainThread,NULL, server_main_thread_function,NULL);
    if(res != 0)
    {
        perror("Main thread creation failed");
        return false;        
    }
    
    res = pthread_create(&m_pKeyThread,NULL, server_key_thread_function,NULL);
    if(res != 0)
    {
        perror("Key thread creation failed");
        return false;
    }
    
    res = pthread_create(&m_pTimeThread,NULL, server_time_thread_function,NULL);
    if(res != 0)
    {
        perror("Time thread creation failed");
        return false;
    }

    res = pthread_create(&m_pMsgThread,NULL, server_msg_thread_function,NULL);
    if(res != 0)
    {
        perror("Message thread creation failed");
        return false;
    }

    return true;    
}

bool server::waitThread()
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

void *server_main_thread_function(void *arg)
{
    int new_socket , activity, i , valread , sd;
    int max_sd;

    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < MAX_PLAYER; i++)
    {
        _pServer->m_ClientSockArr[i] = 0;
    }

    char buffer[1025];  //data buffer of 1K

    //set of socket descriptors
    fd_set readfds;
    
    //accept the incoming connection
    int addrlen = sizeof(_pServer->m_ServerAddr);
    puts("Waiting for connections ...");

    while(_pServer->m_bIsRunning)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(_pServer->m_nServerSock, &readfds);
        max_sd = _pServer->m_nServerSock;

        //add child sockets to set
        for ( i = 1 ; i < MAX_PLAYER ; i++)
        {
            //socket descriptor
            sd = _pServer->m_ClientSockArr[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno!=EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(_pServer->m_nServerSock, &readfds))
        {
            if ((new_socket = accept(_pServer->m_nServerSock, (struct sockaddr *)&_pServer->m_ServerAddr, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , 
            						inet_ntoa(_pServer->m_ServerAddr.sin_addr) , ntohs(_pServer->m_ServerAddr.sin_port));
            
            //add new socket to array of sockets
            for (i = 1; i < MAX_PLAYER; i++)
            {
                //if position is empty
                if( _pServer->m_ClientSockArr[i] == 0 )
                {
                    _pServer->m_ClientSockArr[i] = new_socket;
                    printf("Player #%d has joined.\n" , i+1);

                    break;
                }
            }

            if (i == MAX_PLAYER)
            {
                printf("New player from %s can not play because of maximum player count\n", inet_ntoa(_pServer->m_ServerAddr.sin_addr));
                loginReply(new_socket, -1);
            } else
            {
                printf("Get new index %d from server\n", i + 1);
                loginReply(new_socket, i + 1);
            }            
        }

        //else its some IO operation on some other socket
        for (i = 1; i < MAX_PLAYER; i++)
        {
            sd = _pServer->m_ClientSockArr[i];

            if (FD_ISSET( sd , &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( sd , buffer, MAX_BUFFER)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&_pServer->m_ServerAddr , \
                        (socklen_t*)&addrlen);
                    printf("Player #%d, Host disconnected , ip %s , port %d \n" , i+1,
                          inet_ntoa(_pServer->m_ServerAddr.sin_addr) , ntohs(_pServer->m_ServerAddr.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    _pServer->m_ClientSockArr[i] = 0;
                }

                //Echo back the message that came in
                else
                {
                    int packet_len = *(int *)buffer;
                    int msg_code = *(int *)(buffer + sizeof(int));
                    if (msg_code == USER_MSG)
                    {
                        char *msg = (char *)calloc(packet_len - 2 * sizeof(int), 1);
                        memcpy(msg, buffer + sizeof(int) * 2, packet_len - 2 * sizeof(int));
                        printf("Message from Player%d : %s\n", i + 1, msg);
                        free(msg);
                    }
                }
            }
        }
    }
}

void *server_key_thread_function(void *arg)
{
    
}

void *server_time_thread_function(void *arg)
{
    
}

void *server_msg_thread_function(void *arg)
{    
    while (_pServer->m_bIsRunning)
    {
        char msg[MAX_BUFFER];
        if (fgets(msg, MAX_BUFFER, stdin) != NULL)
        {
            for (int i = 1; i < MAX_PLAYER; ++i)
            {
                if (_pServer->m_ClientSockArr[i] > 0)
                {
                    serverMessageRequest(_pServer->m_ClientSockArr[i], msg);
                }
            }
        }
    }
}

void loginReply(int sock, int id)
{
    int packet_len = (1 + 1 + 1) * sizeof(int);
    char *packet = (char *)calloc(packet_len, sizeof(char));
    *(int *)packet = packet_len;
    *(int *)(packet + 4) = LOGIN;
    *(int *)(packet + 8) = id;
    send(sock, packet, packet_len, 0);
    free(packet);
}

void serverMessageRequest(int sock, char *msg)
{
    int packet_len = (1 + 1) * sizeof(int) + strlen(msg);
    char *packet = (char *)calloc(packet_len, sizeof(char));
    *(int *)packet = packet_len;
    *(int *)(packet + 4) = USER_MSG;
    strcpy(packet + 8, msg);    
    send(sock, packet, packet_len, 0);   
    free(packet);
}


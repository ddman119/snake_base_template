#include "server.h"

void *server_main_thread_function(void *arg);
void *server_msg_thread_function(void *arg);
void *server_key_thread_function(void *arg);
void *server_time_thread_function(void *arg);

void serverMessageRequest(int exceptId, char *msg);
void serverLoginRequest(int sock, int flag);
void serverStartGameRequest();
void serverEndGameRequest();
void serverTimeSyncRequest();
void serverBackendTimeRequest();

void serverKeyRequest(int exceptId, const char *keystr);
void serverBackendKeyRequest(const char *keystr);
void serverFoodRequest(const char* foodstr);

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
	m_bGameStart = false;
	m_nPlayerCount = 1;
	m_PythonPid = 0;

	res = pthread_create(&m_pMainThread,NULL, server_main_thread_function,NULL);
    if(res != 0)
    {
        perror("Main thread creation failed");
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

void server::createSnakeGame()
{
	char cmd[MAX_LEN];
	
	m_strFIFO_W_Path = (char *)calloc(MAX_PATH, sizeof(char));
	m_strFIFO_R_Path = (char *)calloc(MAX_PATH, sizeof(char));
	sprintf(m_strFIFO_R_Path, "/tmp/snakegame_fifo_w_%d", m_nId);
	sprintf(m_strFIFO_W_Path, "/tmp/snakegame_fifo_r_%d", m_nId);

	mkfifo(m_strFIFO_W_Path, 0666);
	mkfifo(m_strFIFO_R_Path, 0666);
	
	int res = pthread_create(&m_pKeyThread,NULL, server_key_thread_function,NULL);
    if(res != 0)
    {
        perror("Key thread creation failed");
        m_bIsRunning = false; m_bGameStart = false;
        return;
    }
    
    res = pthread_create(&m_pTimeThread,NULL, server_time_thread_function,NULL);
    if(res != 0)
    {
        perror("Time thread creation failed");
        m_bIsRunning = false; m_bGameStart = false;
        return;
    }

    sprintf(cmd, "python snake.py %d %d Server", m_nPlayerCount, m_nId);
    m_PythonPid = fork();
    if (m_PythonPid == 0)
    {
        system(cmd);
    }
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
                serverLoginRequest(new_socket, -1);
            } else
            {
            	_pServer->m_nPlayerCount ++;
                printf("New player created. Current player count is %d\n", _pServer->m_nPlayerCount);                
                serverLoginRequest(new_socket, 0);
            }
        }

        //else its some IO operation on some other socket
        for (i = 1; i < MAX_PLAYER; i++)
        {
            sd = _pServer->m_ClientSockArr[i];

            if (FD_ISSET(sd , &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read(sd , buffer, MAX_BUFFER)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd, (struct sockaddr*)&_pServer->m_ServerAddr, (socklen_t*)&addrlen);
					_pServer->m_nPlayerCount--;

					printf("Player #%d, Host disconnected , ip %s , port %d. Current player count is %d \n" , i + 1,
                          inet_ntoa(_pServer->m_ServerAddr.sin_addr) , ntohs(_pServer->m_ServerAddr.sin_port), _pServer->m_nPlayerCount);

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
                        char *msg = (char *)calloc(packet_len - 2 * sizeof(int) + 64, 1);                        
                        sprintf(msg, "Message from Player%d : %s\n", i + 1, buffer + sizeof(int) * 2);
                        printf("[backend] %s", msg);
                    	serverMessageRequest(i + 1, msg);
                        free(msg);
                    } else if (msg_code == USER_KEY)
                    {
                    	char *keystr = (char *)calloc(packet_len - 2 * sizeof(int), 1);
	                    memcpy(keystr, buffer + sizeof(int) * 2, packet_len - 2 * sizeof(int));
	                    serverBackendKeyRequest(keystr);
	                    serverKeyRequest(i + 1, keystr);
	                    free(keystr);
                    }
                }
            }
        }
    }
}

void *server_key_thread_function(void *arg)
{
	int fd = open(_pServer->m_strFIFO_R_Path, O_RDONLY);
	if (fd == -1)
	{
		printf("[backend] Open named pipe failed\n");
		return NULL;
	}
	char ch;
	std::string buf;
    while (_pServer->m_bIsRunning)
    {
    	if (read(fd, &ch, 1) == 1)
    	{    		
    		if (ch != '\n' && ch != '\0')
    		{
    			buf += ch;
    		} else {
    			const char *fifostr = buf.c_str();

    			if (strncmp(fifostr, "FOOD", 4) == 0)
    			{	// Food hit to snake. Relocate it
                    printf("[backend]%s\n", fifostr);
        			serverFoodRequest(fifostr);
    			} else
    			{	// User press key. tell others
    				char *tmp = (char *)calloc(strlen(fifostr) + 8, sizeof(char));
	    			sprintf(tmp, "KEY:1:%s", fifostr);
	                serverKeyRequest(1, tmp);
    			}
    			buf = "";
    		}
    	}
    }
}

void *server_time_thread_function(void *arg)
{
    while (_pServer->m_bIsRunning)
    {
    	serverTimeSyncRequest();
    	serverBackendTimeRequest();
    	usleep(1500);	// 1.5ms interval
    }
}

void *server_msg_thread_function(void *arg)
{    
    while (_pServer->m_bIsRunning)
    {
        char msg[MAX_BUFFER];
        if (fgets(msg, MAX_BUFFER, stdin) != NULL)
        {        	
        	if (strncmp(msg, "-c", 2) == 0)	// game control command
        	{
        		if (!_pServer->m_bGameStart && strncmp(msg + 3, "start", 5) == 0)		// start game
        		{        			
        			_pServer->m_nId = 1;
        			_pServer->createSnakeGame();
        			serverStartGameRequest();
        		} else if (_pServer->m_bGameStart && strncmp(msg + 3, "exit", 4) == 0)		// end game
        		{
        			serverEndGameRequest();
        			_pServer->m_bIsRunning = false;
        		}
        	} else if (strncmp(msg, "-m", 2) == 0)	// message command
        	{
        		char* buf = (char *)calloc(strlen(msg) + 32, sizeof(char));
        		sprintf(buf, "Message from Player1 : %s\n", msg + 2);                
        		serverMessageRequest(1, buf);
        		free(buf);
        	}
        }
    }
}

// reply to login request
void serverLoginRequest(int sock, int flag)
{
    int packet_len = (1 + 1 + 1) * sizeof(int);
    char *packet = (char *)calloc(packet_len, sizeof(char));
    *(int *)packet = packet_len;
    *(int *)(packet + 4) = LOGIN;
    *(int *)(packet + 8) = flag;
    pthread_mutex_lock(&_pServer->m_Mutex);
    send(sock, packet, packet_len, 0);
    pthread_mutex_unlock(&_pServer->m_Mutex);
    free(packet);
}

// send message to sock
void serverMessageRequest(int exceptId, char *msg)
{
	int packet_len = (1 + 1) * sizeof(int) + strlen(msg);
    char *packet = (char *)calloc(packet_len, sizeof(char));
    *(int *)packet = packet_len;
    *(int *)(packet + 4) = USER_MSG;
    strcpy(packet + 8, msg);

	for (int i = 1; i < MAX_PLAYER; ++i)
    {
        if (_pServer->m_ClientSockArr[i] > 0 && i + 1 != exceptId)
        {
            pthread_mutex_lock(&_pServer->m_Mutex);
		    send(_pServer->m_ClientSockArr[i], packet, packet_len, 0);
		    pthread_mutex_unlock(&_pServer->m_Mutex);
        }
    }
    
    free(packet);
}

void serverStartGameRequest()
{
	int packet_len = (1 + 1 + 1 + 2) * sizeof(int);
    char *packet = (char *)calloc(packet_len, sizeof(char));
    *(int *)packet = packet_len;
    *(int *)(packet + 4) = START;
    *(int *)(packet + 8) = _pServer->m_nPlayerCount;
    
	for (int i = 1; i < MAX_PLAYER; ++i)
    {
        if (_pServer->m_ClientSockArr[i] > 0)
        {
        	*(int *)(packet + 12) = i + 1;    
		    pthread_mutex_lock(&_pServer->m_Mutex);    
		    send(_pServer->m_ClientSockArr[i], packet, packet_len, 0);
		    pthread_mutex_unlock(&_pServer->m_Mutex);
        }
    }
    
    free(packet);	
}

void serverEndGameRequest()
{
	int packet_len = (1 + 1) * sizeof(int);
    char *packet = (char *)calloc(packet_len, sizeof(char));
    *(int *)packet = packet_len;
    *(int *)(packet + 4) = END;

	for (int i = 1; i < MAX_PLAYER; ++i)
	{
	    if (_pServer->m_ClientSockArr[i] > 0)
	    {
	    	pthread_mutex_lock(&_pServer->m_Mutex);
		    send(_pServer->m_ClientSockArr[i], packet, packet_len, 0);   
		    pthread_mutex_unlock(&_pServer->m_Mutex);	        
	    }
    }

    free(packet);		
}

// Send pressed key to client
void serverKeyRequest(int exceptId, const char *keystr)
{
    int packet_len = (1 + 1) * sizeof(int) + strlen(keystr);
    char *packet = (char *)calloc(packet_len, sizeof(char));
    *(int *)packet = packet_len;
    *(int *)(packet + 4) = USER_KEY;
    strcpy(packet + 8, keystr);

    for (int i = 1; i < MAX_PLAYER; ++i)
	{
	    if (_pServer->m_ClientSockArr[i] > 0 && i + 1 != exceptId)
	    {
	    	pthread_mutex_lock(&_pServer->m_Mutex);
		    send(_pServer->m_ClientSockArr[i], packet, packet_len, 0);   
		    pthread_mutex_unlock(&_pServer->m_Mutex);	        
	    }
    }

    free(packet);
}

// Send others key info to python
void serverBackendKeyRequest(const char *keystr)
{	
    int fd = open(_pServer->m_strFIFO_W_Path, O_WRONLY);
    if (fd == -1)
    {
        printf("[backend] Open fifo failed\n");
        return;
    }
    write(fd, keystr, strlen(keystr));
    close(fd);
}

void serverTimeSyncRequest()
{
	int packet_len = (1 + 1) * sizeof(int);
    char *packet = (char *)calloc(packet_len, sizeof(char));
    *(int *)packet = packet_len;
    *(int *)(packet + 4) = TIME_SYNC;

	for (int i = 1; i < MAX_PLAYER; ++i)
	{
	    if (_pServer->m_ClientSockArr[i] > 0)
	    {
	    	pthread_mutex_lock(&_pServer->m_Mutex);
		    send(_pServer->m_ClientSockArr[i], packet, packet_len, 0);   
		    pthread_mutex_unlock(&_pServer->m_Mutex);	        
	    }
    }

    free(packet);
}

void serverFoodRequest(const char * foodstr)
{
	int packet_len = (1 + 1) * sizeof(int) + strlen(foodstr);
    char *packet = (char *)calloc(packet_len, sizeof(char));
    *(int *)packet = packet_len;
    *(int *)(packet + 4) = FOOD;
    memcpy(packet + 8, foodstr, strlen(foodstr));
    
	for (int i = 1; i < MAX_PLAYER; ++i)
	{
	    if (_pServer->m_ClientSockArr[i] > 0)
	    {
	    	pthread_mutex_lock(&_pServer->m_Mutex);
		    send(_pServer->m_ClientSockArr[i], packet, packet_len, 0);   
		    pthread_mutex_unlock(&_pServer->m_Mutex);	        
	    }
    }

    free(packet);
}

void serverBackendTimeRequest()
{
    int fd = open(_pServer->m_strFIFO_W_Path, O_WRONLY);
    if (fd == -1)
    {
        printf("[backend] Open fifo failed\n");
        return;
    }    
    write(fd, "TIME\n", 5);
    close(fd);
}

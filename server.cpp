#include "server.h"

void *server_main_thread_function(void *arg);
void *server_msg_thread_function(void *arg);
void *server_key_thread_function(void *arg);
void *server_time_thread_function(void *arg);

void serverMessageRequest(int exceptId, char *msg);
void serverLoginRequest(int sock, int flag);
void serverStartGameRequest(POSITION pos);
void serverEndGameRequest();
void serverTimeSyncRequest();
void serverStateRequest(int exceptId, const char *keystr);
void serverPlayerDisconRequest(int id);
void serverFoodRequest(const char* foodstr);

void serverBackendPlayerDisconRequest(int id);
void serverBackendTimeRequest();
void serverBackendStateRequest(const char *keystr);
void serverBackendFoodRequest(char *foodstr);

void relocateFood(POSITION &pos);

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

    res = pthread_mutex_init(&m_Mutex, NULL);

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
    }

    res = pthread_join(m_pKeyThread, &thread_result);
    if(res != 0)
    {
        perror("Key thread join failed");        
    }

    pthread_cancel(m_pTimeThread);
    res = pthread_join(m_pTimeThread, &thread_result);
    if(res != 0)
    {
        perror("Time thread join failed");        
    }    

    pthread_cancel(m_pMsgThread);
    res = pthread_join(m_pMsgThread, &thread_result);
    if(res != 0)
    {
        perror("Message thread join failed");
    }

    pthread_mutex_destroy(&m_Mutex);

    return true;
}

void server::createSnakeGame(POSITION pos)
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

    m_bGameStart = true;
    sprintf(cmd, "python snake.py %d %d %d %d", m_nPlayerCount, m_nId, pos.xpos, pos.ypos);
    
    m_PythonPid = fork();
    if (m_PythonPid == 0)
    {
        system(cmd);
        exit(0);
    }
    // } else 
    // {
    //     int status = 0;
    //     int options = 0;        
    //     waitpid(m_PythonPid, &status, options);
    // }

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
    puts("Waiting for incoming connections ...");

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
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        activity = select( max_sd + 1 , &readfds , NULL , NULL , &timeout);

        if ((activity < 0) && (errno != EINTR))
        {
            continue;            
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
                    printf("Player %d has joined.\n" , i + 1);

                    break;
                }
            }

            if (i == MAX_PLAYER)
            {
                printf("New player from %s can not play because of maximum player count\n", inet_ntoa(_pServer->m_ServerAddr.sin_addr));
                serverLoginRequest(new_socket, -1);
            } else if (_pServer->m_bGameStart)
            {
                printf("New player from %s can not play because game already start\n", inet_ntoa(_pServer->m_ServerAddr.sin_addr));
                serverLoginRequest(new_socket, -2);
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

					printf("Player %d, Host disconnected , ip %s , port %d. Current player count is %d \n" , i + 1,
                          inet_ntoa(_pServer->m_ServerAddr.sin_addr) , ntohs(_pServer->m_ServerAddr.sin_port), _pServer->m_nPlayerCount);

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    _pServer->m_ClientSockArr[i] = 0;
                    
                    if (_pServer->m_bGameStart)
                    {
                        serverPlayerDisconRequest(i + 1);
                        serverBackendPlayerDisconRequest(i + 1);
                    }
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
                    } else if (msg_code == STATE)
                    {
                    	char *statestr = (char *)calloc(packet_len - 2 * sizeof(int), 1);
	                    memcpy(statestr, buffer + sizeof(int) * 2, packet_len - 2 * sizeof(int));
	                    serverBackendStateRequest(statestr);
	                    serverStateRequest(i + 1, statestr);
	                    free(statestr);
                    } else if (msg_code == FOOD)
                    {
                        POSITION pos;
                        relocateFood(pos);
                        char tmp[32];
                        sprintf(tmp, "FOOD:%d:%d\n", pos.xpos, pos.ypos);
                        serverFoodRequest(tmp);
                        serverBackendFoodRequest(tmp);
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
    			if (strncmp(fifostr, "FOODHIT", 7) == 0)
    			{	// Food hit to snake. Relocate it                    
                    POSITION pos;
                    relocateFood(pos);
                    printf("[backend] Get food hit request from python. relocate it x=%d y=%d\n", pos.xpos, pos.ypos);
                    char tmp[32];
                    sprintf(tmp, "FOOD:%d:%d\n", pos.xpos, pos.ypos);
                    serverBackendFoodRequest(tmp);
        			serverFoodRequest(tmp);
    			} else if (strncmp(fifostr, "EXIT", 4) == 0)
                {   // Python game end. Also all clients must end
                    serverEndGameRequest();
                    _pServer->m_bIsRunning = false;
                } else if (strncmp(fifostr, "STATE", 5) == 0)
                {
                    serverStateRequest(1, fifostr);
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
    	usleep(40000);	// 1.5ms interval
        serverTimeSyncRequest();
        serverBackendTimeRequest();
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
                    POSITION pos;
                    relocateFood(pos);        			
        			serverStartGameRequest(pos);
                    _pServer->createSnakeGame(pos);
        		} else if (strncmp(msg + 3, "exit", 4) == 0)		// end game
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
    strncpy(packet + 8, msg, strlen(msg));

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

void serverStartGameRequest(POSITION pos)
{
	int packet_len = (1 + 1 + 1 + 1 + 2) * sizeof(int);
    char *packet = (char *)calloc(packet_len, sizeof(char));
    *(int *)packet = packet_len;
    *(int *)(packet + 4) = START;
    *(int *)(packet + 8) = _pServer->m_nPlayerCount;
    *(int *)(packet + 16) = pos.xpos;
    *(int *)(packet + 20) = pos.ypos;

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

// Send player state to client
void serverStateRequest(int exceptId, const char *statestr)
{
    int packet_len = (1 + 1) * sizeof(int) + strlen(statestr);
    char *packet = (char *)calloc(packet_len, sizeof(char));
    *(int *)packet = packet_len;
    *(int *)(packet + 4) = STATE;
    strncpy(packet + 8, statestr, strlen(statestr));

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
    strncpy(packet + 8, foodstr, strlen(foodstr));
        
	for (int i = 1; i < MAX_PLAYER; ++i)
	{    
	    if (_pServer->m_ClientSockArr[i] > 0)
	    {            
 	    	pthread_mutex_lock(&_pServer->m_Mutex);
            printf("[backend] food packet len %d msg_code=%d\n", packet_len, *(int*)(packet + 4));
		    send(_pServer->m_ClientSockArr[i], packet, packet_len, 0);
            printf("[backend] food packet send\n");
		    pthread_mutex_unlock(&_pServer->m_Mutex);
	    }
    }

    free(packet);
}

void serverPlayerDisconRequest(int id)
{
    int packet_len = (1 + 1 + 1) * sizeof(int);
    char *packet = (char *)calloc(packet_len, sizeof(char));
    *(int *)packet = packet_len;
    *(int *)(packet + 4) = DISCON;
    *(int *)(packet + 8) = id;    
    
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
    
void serverBackendPlayerDisconRequest(int id)
{
    int fd = open(_pServer->m_strFIFO_W_Path, O_WRONLY);
    if (fd == -1)
    {
        printf("[backend] Open fifo failed in disconnect function\n");
        return;
    }
    char tmp[12];
    sprintf(tmp, "DISC:%d\n", id);
    
    write(fd, tmp, 12);
    close(fd);
}

void serverBackendTimeRequest()
{
    int fd = open(_pServer->m_strFIFO_W_Path, O_WRONLY);
    if (fd == -1)
    {
        printf("[backend] Open fifo failed in time function\n");
        return;
    }    
    write(fd, "TIME\n", 5);
    close(fd);
}

// Send others key info to python
void serverBackendStateRequest(const char *statestr)
{   
    std::string buf(statestr);
    buf = buf + '\n';
    int fd = open(_pServer->m_strFIFO_W_Path, O_WRONLY);
    if (fd == -1)
    {
        printf("[backend] Open fifo failed in key function\n");
        return;
    }
    write(fd, buf.c_str(), strlen(buf.c_str()));    
    close(fd);
}

void serverBackendFoodRequest(char *foodstr)
{
    std::string buf(foodstr);
    buf = buf + '\n';
    int fd = open(_pServer->m_strFIFO_W_Path, O_WRONLY);
    if (fd == -1)
    {
        printf("[backend] Open fifo failed in key function\n");
        return;
    }    
    write(fd, buf.c_str(), strlen(buf.c_str()));
    
    close(fd);
}

void relocateFood(POSITION &pos)
{    
    
    int x = 0, y = 0;
    while (x == 0 || y == 0)
    {
        time_t cur = time(NULL);
        srand(cur);
        x = rand() % DISP_WIDTH;
        y = rand() % DISP_HEIGHT;
        x = x - x % 10;
        y = y - y % 10;
    }
    
    pos.xpos = x;
    pos.ypos = y;
}

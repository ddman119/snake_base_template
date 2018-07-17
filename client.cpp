#include "client.h"

void *client_main_thread_function(void *arg);
void *client_msg_thread_function(void *arg);
void *client_key_thread_function(void *arg);
void *client_time_thread_function(void *arg);

void clientMessageRequest(int sock, const char *msg);
void clientKeyRequest(int sock, const char *keystr);
void clientBackendKeyRequest(const char *keystr);
void clientBackendTimeRequest();
void clientBackendFoodRequest(char *foodstr);

client *_pClient;

client::client(int sock)
	: m_nClientSock(sock)
{
	_pClient = this;
    pthread_mutex_init(&m_Mutex, NULL);
}

bool client::startClient()
{
	int res;
    m_bIsRunning = true;
    m_nPlayerCount = 0;
	res = pthread_create(&m_pMainThread,NULL, client_main_thread_function,NULL);
    if(res != 0)
    {
        perror("Main thread creation failed");
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

void client::createSnakeGame()
{
    char cmd[MAX_LEN];
    int res;
    m_strFIFO_W_Path = (char *)calloc(MAX_PATH, sizeof(char));
    m_strFIFO_R_Path = (char *)calloc(MAX_PATH, sizeof(char));
    sprintf(m_strFIFO_R_Path, "/tmp/snakegame_fifo_w_%d", m_nId);
    sprintf(m_strFIFO_W_Path, "/tmp/snakegame_fifo_r_%d", m_nId);

    mkfifo(m_strFIFO_W_Path, 0666);
    mkfifo(m_strFIFO_R_Path, 0666);

    res = pthread_create(&m_pKeyThread,NULL, client_key_thread_function,NULL);
    if(res != 0)
    {
        perror("Key thread creation failed");
        return;
    }
    
    res = pthread_create(&m_pTimeThread,NULL, client_time_thread_function,NULL);
    if(res != 0)
    {
        perror("Time thread creation failed");
        return;
    }

    sprintf(cmd, "python snake.py %d %d Client", m_nPlayerCount, m_nId);
    m_PythonPid = fork();
    if (m_PythonPid == 0)
    {
        system(cmd);
    }
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
                    printf("[backend] %s\n", msg);
                    free(msg);
                } else if (msg_code == USER_KEY) {
                    char *keystr = (char *)calloc(packet_len - 2 * sizeof(int), 1);
                    memcpy(keystr, buffer + sizeof(int) * 2, packet_len - 2 * sizeof(int));
                    clientBackendKeyRequest(keystr);
                    free(keystr);
                } else if (msg_code == TIME_SYNC) {
                    clientBackendTimeRequest();
                } else if (msg_code == FOOD) {                    
                    char *foodstr = (char *)calloc(packet_len - 2 * sizeof(int), 1);
                    memcpy(foodstr, buffer + sizeof(int) * 2, packet_len - 2 * sizeof(int));
                    printf("[client] Get Food pos %s\n", foodstr);
                    clientBackendFoodRequest(foodstr);
                    free(foodstr);                    
                } else if (msg_code = START)
                {
                    int player_count = *(int *)(buffer + sizeof(int) * 2);
                    int id = *(int *)(buffer + sizeof(int) * 3);                    
                    _pClient->m_nId = id;
                    _pClient->m_nPlayerCount = player_count;
                    printf("[backend] Get id %d from server. Start snake game with %d player\n", id, player_count);
                    _pClient->createSnakeGame();
                }
            }   
        }        
    }
}

void *client_key_thread_function(void *arg)
{
    int fd = open(_pClient->m_strFIFO_R_Path, O_RDONLY);
    if (fd == -1)
    {
        printf("[backend] Open named pipe failed\n");
        return NULL;
    }
    char ch;
    std::string buf;
    while (_pClient->m_bIsRunning)
    {
        if (read(fd, &ch, 1) == 1)
        {           
            if (ch != '\n' && ch != '\0')
            {
                buf += ch;
            } else {
                char *tmp = (char *)calloc(strlen(buf.c_str()) + 8, sizeof(char));
                sprintf(tmp, "KEY:%d:%s", _pClient->m_nId, buf.c_str());                
                clientKeyRequest(_pClient->m_nClientSock, tmp);
                buf = "";
            }
        }
    }

    close(fd);

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
            if (strncmp(msg, "-m", 2) == 0)     // message command
            {
                clientMessageRequest(_pClient->m_nClientSock, msg + 2);
            }            
        }
    }
}

// Send inputed message to server
void clientMessageRequest(int sock, const char *msg)
{
    int packet_len = (1 + 1) * sizeof(int) + strlen(msg);
    char *packet = (char *)calloc(packet_len, sizeof(char));
    *(int *)packet = packet_len;
    *(int *)(packet + 4) = USER_MSG;
    strcpy(packet + 8, msg);
    pthread_mutex_lock(&_pClient->m_Mutex);
    send(sock, packet, packet_len, 0);   
    pthread_mutex_unlock(&_pClient->m_Mutex);
    free(packet);
}

// Send pressed key to server
void clientKeyRequest(int sock, const char *keystr)
{
    int packet_len = (1 + 1) * sizeof(int) + strlen(keystr);
    char *packet = (char *)calloc(packet_len, sizeof(char));
    *(int *)packet = packet_len;
    *(int *)(packet + 4) = USER_KEY;
    strcpy(packet + 8, keystr);
    pthread_mutex_lock(&_pClient->m_Mutex);
    send(sock, packet, packet_len, 0);
    pthread_mutex_unlock(&_pClient->m_Mutex);
    free(packet);
}

// Send others key info to python
void clientBackendKeyRequest(const char *keystr)
{
    int fd = open(_pClient->m_strFIFO_W_Path, O_WRONLY);
    if (fd == -1)
    {
        printf("[backend] Open fifo failed\n");
        return;
    }
    write(fd, keystr, strlen(keystr));
    close(fd);
}

void clientBackendTimeRequest()
{
    int fd = open(_pClient->m_strFIFO_W_Path, O_WRONLY);
    if (fd == -1)
    {
        printf("[backend] Open fifo failed\n");
        return;
    }
    write(fd, "TIME\n", 5);
    close(fd);
}

void clientBackendFoodRequest(char *foodstr)
{
    int fd = open(_pClient->m_strFIFO_W_Path, O_WRONLY);
    if (fd == -1)
    {
        printf("[backend] Open fifo failed\n");
        return;
    }
    
    write(fd, foodstr, strlen(foodstr));
    close(fd);  
}

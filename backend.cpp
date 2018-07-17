//Example code: A simple server side code, which echos back the received message.
//Handle multiple socket connections with select and fd_set on Linux

#include "global.h"

void showHelp();
bool checkArgument(int argc, char* argv[], char *serverAddr, bool & isServer);

void *server_main_thread_function(void *arg);
void *server_msg_thread_function(void *arg);
void *server_key_thread_function(void *arg);
void *server_time_thread_function(void *arg);
int server_connect();

void *client_main_thread_function(void *arg);
void *client_msg_thread_function(void *arg);
void *client_key_thread_function(void *arg);
void *client_time_thread_function(void *arg);
int client_connect(char* serverAddr);
void messageRequest(int sock, char *msg);

void createSnakeGame(int id);

void loginReply(int sock, int id);

int main(int argc , char *argv[])
{
    char serverAddr[MAX_LEN] = {0};
    bool isServer;
    int res;
    pthread_t main_thread, key_thread, time_thread, msg_thread;
    void *thread_result;
    _bIsRunning = true;

    if (!(checkArgument(argc, argv, serverAddr, isServer)))
    {
        return -1;
    }

    if (isServer)
    {
        server_connect();

        res = pthread_create(&main_thread,NULL, server_main_thread_function,NULL);
        if(res != 0)
        {
            perror("Main thread creation failed");
            exit(0);
        }
        
        res = pthread_create(&key_thread,NULL, server_key_thread_function,NULL);
        if(res != 0)
        {
            perror("Key thread creation failed");
            exit(0);
        }
        
        res = pthread_create(&time_thread,NULL, server_time_thread_function,NULL);
        if(res != 0)
        {
            perror("Time thread creation failed");
            exit(0);
        }

        res = pthread_create(&msg_thread,NULL, server_msg_thread_function,NULL);
        if(res != 0)
        {
            perror("Message thread creation failed");
            exit(0);
        }

        createSnakeGame(1);

        res = pthread_join(main_thread, &thread_result);
        if(res != 0)
        {
            perror("Main thread join failed");
            exit(0);
        }
        res = pthread_join(key_thread, &thread_result);
        if(res != 0)
        {
            perror("Key thread join failed");
            exit(0);
        }
        res = pthread_join(time_thread, &thread_result);
        if(res != 0)
        {
            perror("Time thread join failed");
            exit(0);
        }
        res = pthread_join(msg_thread, &thread_result);
        if(res != 0)
        {
            perror("Message thread join failed");
            exit(0);
        }
    }
    else
    {
        client_connect(serverAddr);

        res = pthread_create(&main_thread,NULL, client_main_thread_function,NULL);
        if(res != 0)
        {
            perror("Main thread creation failed");
            exit(0);
        }
        
        res = pthread_create(&key_thread,NULL, client_key_thread_function,NULL);
        if(res != 0)
        {
            perror("Key thread creation failed");
            exit(0);
        }
        
        res = pthread_create(&time_thread,NULL, client_time_thread_function,NULL);
        if(res != 0)
        {
            perror("Time thread creation failed");
            exit(0);
        }

        res = pthread_create(&msg_thread,NULL, client_msg_thread_function,NULL);
        if(res != 0)
        {
            perror("Message thread creation failed");
            exit(0);
        }

        res = pthread_join(main_thread, &thread_result);
        if(res != 0)
        {
            perror("Main thread join failed");
            exit(0);
        }
        res = pthread_join(key_thread, &thread_result);
        if(res != 0)
        {
            perror("Key thread join failed");
            exit(0);
        }
        res = pthread_join(time_thread, &thread_result);
        if(res != 0)
        {
            perror("Time thread join failed");
            exit(0);
        }
        res = pthread_join(msg_thread, &thread_result);
        if(res != 0)
        {
            perror("Message thread join failed");
            exit(0);
        }
    }

    
    return 0;
}

bool checkArgument(int argc, char *argv[], char *serverAddr, bool &isServer)
{
    if (argc < 2 || argc > 4)
    {
        printf("Invalid arguments.\n");
        showHelp();
        return false;
    }
    if (strcmp(argv[1], "-h") == 0)
    {
        showHelp();
        return false;
    }
    if ((strcmp(argv[1], "-c") != 0 && strcmp(argv[1], "-s") != 0) || (strcmp(argv[1], "-s") == 0 && argc == 3) || 
                (strcmp(argv[1], "-c") == 0 && argc == 2))
    {
        printf("Invalid arguments.\n");
        showHelp();
        return false;   
    }
    if (strcmp(argv[1], "-s") == 0)
    {
        isServer = true;
    } else
    {
        isServer = false;
        strncpy(serverAddr, argv[2], MAX_LEN);
    }
    return true;
}

void showHelp()
{
    printf("Command type is : \n");
    printf("snake [-c/-s/-h] [server address(in case of argument 2 is -c)]\n");
}

void *client_main_thread_function(void *arg)
{
    int new_socket , activity, i , valread , sd;
    int max_sd;
    
    char buffer[1025];  //data buffer of 1K

    //set of socket descriptors
    fd_set readfds;
    
    //accept the incoming connection
    addrlen = sizeof(address);    

    while(_bIsRunning)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(my_socket, &readfds);        

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select( my_socket + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(my_socket, &readfds))
        {            
            if ((valread = read( my_socket , buffer, MAX_BUFFER)) == 0)
            {
                close(my_socket);
                _bIsRunning = false;
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
    while (_bIsRunning)
    {
        char msg[MAX_BUFFER];
        if (fgets(msg, MAX_BUFFER, stdin) != NULL)
        {
            messageRequest(my_socket, msg);
        }
    }
}


int client_connect(char* serverAddr){
    
    struct sockaddr_in address;
    int valread;
    struct sockaddr_in serv_addr;
    
    char buffer[MAX_BUFFER] = {0};

    if ((my_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, serverAddr, &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }

    if (connect(my_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        exit(EXIT_FAILURE);
    }
        
    if ((valread = read( my_socket , buffer, MAX_BUFFER)) == 0)
    {
        printf("Read Failed\n");
        close(my_socket);
        exit(EXIT_FAILURE);
    }

    int packet_len = *(int *)buffer;
    int msg_code = *(int *)(buffer + sizeof(int));
    int id = *(int *)(buffer + sizeof(int) * 2);
    if (id == -1)
    {
        printf("Player count is maximum. Can not play\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        createSnakeGame(id);
    }
}

void *server_main_thread_function(void *arg)
{
    int new_socket , activity, i , valread , sd;
    int max_sd;
    
    char buffer[1025];  //data buffer of 1K

    //set of socket descriptors
    fd_set readfds;
    
    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while(_bIsRunning)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for ( i = 1 ; i < MAX_PLAYER ; i++)
        {
            //socket descriptor
            sd = client_socket[i];

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
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
            
            //add new socket to array of sockets
            for (i = 1; i < MAX_PLAYER; i++)
            {
                //if position is empty
                if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    printf("Player #%d has joined.\n" , i+1);

                    break;
                }
            }

            if (i == MAX_PLAYER)
            {
                printf("New player from %s can not play because of maximum player count\n", inet_ntoa(address.sin_addr));
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
            sd = client_socket[i];

            if (FD_ISSET( sd , &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( sd , buffer, MAX_BUFFER)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);
                    printf("Player #%d, Host disconnected , ip %s , port %d \n" , i+1,
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
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
    while (_bIsRunning)
    {
        char msg[MAX_BUFFER];
        if (fgets(msg, MAX_BUFFER, stdin) != NULL)
        {
            for (int i = 1; i < MAX_PLAYER; ++i)
            {
                if (client_socket[i] > 0)
                {
                    messageRequest(client_socket[i], msg);
                }
            }
        }
    }
}

int server_connect(){
    int opt = TRUE, i;

    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < MAX_PLAYER; i++)
    {
        client_socket[i] = 0;
    }

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        // Then we must be a client.
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Listener on port %d \n", PORT);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, MAX_PLAYER - 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }   

    return 0;
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

void messageRequest(int sock, char *msg)
{
    int packet_len = (1 + 1) * sizeof(int) + strlen(msg);
    char *packet = (char *)calloc(packet_len, sizeof(char));
    *(int *)packet = packet_len;
    *(int *)(packet + 4) = USER_MSG;
    strcpy(packet + 8, msg);    
    send(sock, packet, packet_len, 0);   
    free(packet);
}

void createSnakeGame(int id)
{
    char cmd[MAX_LEN];
    sprintf(cmd, "python snake.py %d", id);
    pid_t pid = fork();
    if (pid == 0)
    {
        system(cmd);
    }
}

//Example code: A simple server side code, which echos back the received message.
//Handle multiple socket connections with select and fd_set on Linux

#include "global.h"
#include "server.h"
#include "client.h"

void showHelp();
bool checkArgument(int argc, char* argv[], char *serverAddr, bool & isServer);

int server_connect(int& serverSock, struct sockaddr_in& serverAddr);
int client_connect(char* serverAddr, int &sock);

int main(int argc , char *argv[])
{
    char serverAddr[MAX_LEN] = {0};
    bool isServer;    

    // Check inputed arguments
    if (!(checkArgument(argc, argv, serverAddr, isServer)))
    {
        return -1;
    }

    // Act for server
    if (isServer)
    {
        int serverSock;
        struct sockaddr_in serverAddr;
        if (server_connect(serverSock, serverAddr) == -1)
        {
            // Create server socket failed. Exit.
            return -1;
        }
        server *_pServer = new server(serverSock, serverAddr);
        if (!_pServer->startServer())
            exit(EXIT_FAILURE);        
        _pServer->waitThread();
        int status = 0;
        wait(&status);
        exit(0);
    }
    else    // Act for client
    {
        int id; int sock;
        if (client_connect(serverAddr, sock) == -1)        
            exit(EXIT_FAILURE);
        client *_pClient = new client(sock);
        if (!_pClient->startClient())
            exit(EXIT_FAILURE);
        _pClient->waitThread();        
        
        int status = 0;
        wait(&status);
        exit(0);
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

int client_connect(char* serverAddr, int &sock){
    
    struct sockaddr_in address;
    int valread;
    struct sockaddr_in serv_addr;
    
    char buffer[MAX_BUFFER] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, serverAddr, &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
        
    if ((valread = read( sock , buffer, MAX_BUFFER)) == 0)
    {
        printf("Read Failed\n");
        close(sock);
        return -1;
    }

    int packet_len = *(int *)buffer;
    int msg_code = *(int *)(buffer + sizeof(int));
    int flag = *(int *)(buffer + sizeof(int) * 2);
    if (flag == -1)
    {
        printf("Player count is maximum. Can not play\n");
        return -1;
    } else if (flag == -2)
    {
        printf("Game already start. Can not play\n");
        return -1;
    } else
    {
        printf("Login success. Please wait until start game\n");
    }

    return 0;
}

int server_connect(int& serverSock, struct sockaddr_in& serverAddr){
    int opt = TRUE;
    
    //create a master socket
    if( (serverSock = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        return -1;
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        return -1;        
    }

    //type of socket created
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons( PORT );

    //bind the socket to localhost port 8888
    if (bind(serverSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr))<0)
    {
        // Then we must be a client.
        perror("bind failed");
        return -1;
    }

    printf("Listener on port %d \n", PORT);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(serverSock, MAX_PLAYER - 1) < 0)
    {
        perror("listen");
        return -1;
    }

    return 0;
}


#include "global.h"

class server
{
public:
	server(int serverSock, struct sockaddr_in serverAddr);
	bool startServer();
	bool waitThread();

public:
	int m_nServerSock;
	struct sockaddr_in m_ServerAddr;
	int m_ClientSockArr[MAX_PLAYER];
	bool m_bIsRunning;
	
private:
	pthread_t m_pMainThread;
	pthread_t m_pMsgThread;
	pthread_t m_pTimeThread;
	pthread_t m_pKeyThread;


};
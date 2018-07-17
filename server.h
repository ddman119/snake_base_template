
#include "global.h"

class server
{
public:
	server(int serverSock, struct sockaddr_in serverAddr);
	bool startServer();
	bool waitThread();
	void createSnakeGame();

public:
	int m_nServerSock;
	struct sockaddr_in m_ServerAddr;
	int m_nId;
	int m_nPlayerCount;
	int m_ClientSockArr[MAX_PLAYER];
	bool m_bIsRunning;
	bool m_bGameStart;
	char* m_strFIFO_W_Path;
	char* m_strFIFO_R_Path;
	pthread_mutex_t m_Mutex;
	pid_t m_PythonPid;
	
private:
	pthread_t m_pMainThread;
	pthread_t m_pMsgThread;
	pthread_t m_pTimeThread;
	pthread_t m_pKeyThread;


};
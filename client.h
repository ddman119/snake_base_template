
#include "global.h"

class client
{
public:
	client(int sock);
	bool startClient();
	bool waitThread();
	void createSnakeGame(int food_x, int food_y);

public:
	int m_nClientSock;	
	int m_nId;
	int m_nPlayerCount;
	bool m_bIsRunning;
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
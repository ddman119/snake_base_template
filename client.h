
#include "global.h"

class client
{
public:
	client(int sock, int id);
	bool startClient();
	bool waitThread();
	
public:
	int m_nClientSock;	
	int m_nId;
	bool m_bIsRunning;
	
private:
	pthread_t m_pMainThread;
	pthread_t m_pMsgThread;
	pthread_t m_pTimeThread;
	pthread_t m_pKeyThread;
};
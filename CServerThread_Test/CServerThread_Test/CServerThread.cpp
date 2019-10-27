//*****************************************************************************
// ServerThreadクラス
//*****************************************************************************
#include "CServerThread.h"
#include <unistd.h>


#define _CSERVER_THREAD_THREAD_
#define CLIENT_CONNECT_NUM			( 5 )

//-----------------------------------------------------------------------------
// コンストラクタ
//-----------------------------------------------------------------------------
CServerThread::CServerThread()
{
	m_bInitFlag = false;
	m_ErrorNo = 0;
	m_pcConnectionMonitoringThread = NULL;
	memset(&m_tServerInfo, 0x00, sizeof(m_tServerInfo));
	m_tServerInfo.Socket = -1;

	// 接続監視スレッドクラス生成
	m_pcConnectionMonitoringThread = (CThread *) new CThread("ConnectionMonitoring");
	if (m_pcConnectionMonitoringThread == NULL)
	{
		return;
	}

	m_bInitFlag = true;
}


//-----------------------------------------------------------------------------
// デストラクタ
//-----------------------------------------------------------------------------
CServerThread::~CServerThread()
{
	// クライアント接続要求用スレッド停止忘れ考慮
	this->Stop();

	// 接続監視スレッドクラス解放
	if (m_pcConnectionMonitoringThread != NULL)
	{
		delete m_pcConnectionMonitoringThread;
		m_pcConnectionMonitoringThread = NULL;
	}
}


//-----------------------------------------------------------------------------
// サーバー開始
//-----------------------------------------------------------------------------
CSERVER_THREAD_RET_ENUM CServerThread::Start()
{
	CSERVER_THREAD_RET_ENUM		eRet = CSERVER_THREAD_RET_SECCESS;


	// 初期化チェック
	if (m_bInitFlag == false)
	{
		return CSERVER_THREAD_RET_ERROR_INIT;
	}

	// 接続監視スレッド開始
	m_tServerInfo.tAddr.sin_family = AF_INET;
	m_tServerInfo.tAddr.sin_port = 12345;
	m_tServerInfo.tAddr.sin_addr.s_addr = INADDR_ANY;
	eRet = ConnectionMonitoringThread_Start(m_tServerInfo);
	if (eRet != CSERVER_THREAD_RET_SECCESS)
	{
		return eRet;
	}

	return CSERVER_THREAD_RET_SECCESS;
}


//-----------------------------------------------------------------------------
// クライアント接続要求用スレッド停止
//-----------------------------------------------------------------------------
CSERVER_THREAD_RET_ENUM CServerThread::Stop()
{
	// 初期化チェック
	if (m_bInitFlag == false)
	{
		return CSERVER_THREAD_RET_ERROR_INIT;
	}

	// 接続監視スレッド停止
	ConnectionMonitoringThread_End(m_tServerInfo);

	return CSERVER_THREAD_RET_SECCESS;
}


//-----------------------------------------------------------------------------
// 接続監視スレッド開始
//-----------------------------------------------------------------------------
CSERVER_THREAD_RET_ENUM CServerThread::ConnectionMonitoringThread_Start(CSERVER_SERVER_INFO_TABLE& tServerInfo)
{
	int					iRet = 0;
	CTHREAD_RET_ENUM	eRet = CTHREAD_RET_ERROR_START;
	bool				bRet = false;


	// 馬鹿除け
	if (m_pcConnectionMonitoringThread == NULL)
	{
		return CSERVER_THREAD_RET_ERROR_SYSTEM;
	}

	// 既にスレッドが動作している場合
	bRet = m_pcConnectionMonitoringThread->IsActive();
	if (bRet == true)
	{
		return CSERVER_THREAD_RET_ERROR_STARTED;
	}

	// サーバー側のソケットを生成
	tServerInfo.Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tServerInfo.Socket == -1)
	{
		m_ErrorNo = errno;
#ifdef _CSERVER_THREAD_THREAD_
		perror("CServerThread - socket");
#endif	// #ifdef _CSERVER_THREAD_THREAD_
		close(tServerInfo.Socket);
		tServerInfo.Socket = -1;
		return CSERVER_THREAD_RET_ERROR_CREATE_SOCKET;
	}

	// サーバー側のIP Address・Portを設定
	iRet = bind(tServerInfo.Socket, (struct sockaddr*) &tServerInfo.tAddr, sizeof(tServerInfo.tAddr));
	if (iRet == -1)
	{
		m_ErrorNo = errno;
#ifdef _CSERVER_THREAD_THREAD_
		perror("CServerThread - bind");
#endif	// #ifdef _CSERVER_THREAD_THREAD_
		close(tServerInfo.Socket);
		tServerInfo.Socket = -1;
		return CSERVER_THREAD_RET_ERROR_BIND;
	}

	// クライアントから接続を待つ
	iRet = listen(tServerInfo.Socket, CLIENT_CONNECT_NUM);
	if (iRet == -1)
	{
		m_ErrorNo = errno;
#ifdef _CSERVER_THREAD_THREAD_
		perror("CServerThread - bind");
#endif	// #ifdef _CSERVER_THREAD_THREAD_
		close(tServerInfo.Socket);
		tServerInfo.Socket = -1;
		return CSERVER_THREAD_RET_ERROR_LISTEN;
	}

	// 接続監視スレッド開始
	eRet = m_pcConnectionMonitoringThread->Start(ConnectionMonitoringThreadLauncher);
	if (eRet != CTHREAD_RET_SUCCESS)
	{
		m_ErrorNo = m_pcConnectionMonitoringThread->GetErrorNo();
		close(tServerInfo.Socket);
		tServerInfo.Socket = -1;
		return (CSERVER_THREAD_RET_ENUM)eRet;
	}

	return CSERVER_THREAD_RET_SECCESS;
}


//-----------------------------------------------------------------------------
// 接続監視スレッド停止
//-----------------------------------------------------------------------------
CSERVER_THREAD_RET_ENUM CServerThread::ConnectionMonitoringThread_End(CSERVER_SERVER_INFO_TABLE& tServerInfo)
{
	bool				bRet = false;


	// 馬鹿除け
	if (m_pcConnectionMonitoringThread == NULL)
	{
		return CSERVER_THREAD_RET_ERROR_SYSTEM;
	}

	// 既にスレッドが停止している場合
	bRet = m_pcConnectionMonitoringThread->IsActive();
	if (bRet == false)
	{
		return CSERVER_THREAD_RET_SECCESS;
	}

	// 接続監視スレッド停止
	m_pcConnectionMonitoringThread->Stop();

	// ソケットの解放
	if (tServerInfo.Socket != -1)
	{
		close(tServerInfo.Socket);
		tServerInfo.Socket = -1;
	}

	return CSERVER_THREAD_RET_SECCESS;
}


//-----------------------------------------------------------------------------
// 接続監視スレッド呼び出し
//-----------------------------------------------------------------------------
void* CServerThread::ConnectionMonitoringThreadLauncher(void* pUserData)
{
	// スレッド処理呼び出し
	reinterpret_cast<CServerThread*>(pUserData)->ConnectionMonitoringThread(pUserData);

	return (void*)NULL;
}


//-----------------------------------------------------------------------------
// 接続監視スレッド
//-----------------------------------------------------------------------------
void CServerThread::ConnectionMonitoringThread(void* pUserData)
{
	CServerThread*			pcServerThread = (CServerThread*)pUserData;
	int						iRet = 0;
	fd_set					ReadFdSet;
	int						fdRead[] = { pcServerThread->m_pcConnectionMonitoringThread->GetEdfThreadEndEvent(),
										 pcServerThread->m_tServerInfo.Socket};
	ssize_t					FdNum = sizeof(fdRead) / sizeof(int);
	int						MaxFd = -1;
	bool					bLoop = true;
	bool					bRet = false;


#ifdef _CSERVER_THREAD_THREAD_
	printf("-- Connection Monitoring Thread Start --\n");
#endif	// #ifdef _CSERVER_THREAD_THREAD_

	// スレッド終了要求が来るまでループ
	while (bLoop)
	{
		// クライアント接続要求待ち
		MaxFd = FdSet(fdRead, FdNum, ReadFdSet);
		iRet = select(MaxFd + 1, &ReadFdSet, NULL, NULL, NULL);
		if (iRet < 0)
		{
			m_ErrorNo = errno;
#ifdef _CSERVER_THREAD_THREAD_
			perror("CServerThread - bind");
#endif	// #ifdef _CSERVER_THREAD_THREAD_
			continue;
		}
		else if (iRet == 0)
		{
			// タイムアウト
			continue;
		}
		else
		{
			// スレッド終了イベント
			bRet = FD_ISSET(fdRead[0], &ReadFdSet);
			if (bRet == true)
			{
				bLoop = false;
				continue;
			}
			
			// 接続要求
			bRet = FD_ISSET(fdRead[1], &ReadFdSet);
			if (bRet == true)
			{
				CSERVER_CLIENT_INFO_TABLE		tClientInfo;
				socklen_t						len = sizeof(struct sockaddr_in);
				iRet = accept(fdRead[1], (struct sockaddr*) &tClientInfo, &len);
				if (iRet == -1)
				{
					m_ErrorNo = errno;
#ifdef _CSERVER_THREAD_THREAD_
					perror("CServerThread - accept");
#endif	// #ifdef _CSERVER_THREAD_THREAD_
					continue;
				}

#ifdef _CSERVER_THREAD_THREAD_
				printf("Connect Client!\n");
#endif	// #ifdef _CSERVER_THREAD_THREAD_
				continue;
			}
		}
	}

#ifdef _CSERVER_THREAD_THREAD_
	printf("-- Connection Monitoring Thread End --\n");
#endif	// #ifdef _CSERVER_THREAD_THREAD_
}


//-----------------------------------------------------------------------------
// ファイルディスクリプタをディスクリプタ集合に設定する
//-----------------------------------------------------------------------------
int CServerThread::FdSet( int* pFd, ssize_t FdNum, fd_set& FdSet )
{
	int				TempFd = 0;
	int				MaxFd = -1;

	if (pFd == NULL)
	{
		return -1;
	}


	FD_ZERO(&FdSet);

	// ファイルディスクリプタをセットしながら最大値を探す
	for (ssize_t i = 0; i < FdNum; i++)
	{
		TempFd = pFd[i];

		if (MaxFd < TempFd)
		{
			MaxFd = TempFd;
		}

		FD_SET(TempFd, &FdSet);
	}

	return MaxFd;
}
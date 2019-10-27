#pragma once
//*****************************************************************************
// ServerThreadクラス
//*****************************************************************************
#include "CThread.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>



typedef enum
{
	CSERVER_THREAD_RET_SECCESS = 0x00000000,								// 正常終了
	CSERVER_THREAD_RET_ERROR_INIT = 0xE00000001,							// 初期処理に失敗している
	CSERVER_THREAD_RET_ERROR_STARTED = 0xE00000002,							// 既にスレッドを開始している
	CSERVER_THREAD_RET_ERROR_START = 0xE00000003,							// スレッド開始に失敗しました

	CSERVER_THREAD_RET_ERROR_PARAM = 0xE1000000,							// パラメータエラー
	CSERVER_THREAD_RET_ERROR_CREATE_SOCKET = 0xE1000001,					// ソケット生成に失敗
	CSERVER_THREAD_RET_ERROR_BIND = 0xE1000002,								// ソケットの名前つけに失敗
	CSERVER_THREAD_RET_ERROR_LISTEN = 0xE1000003,							// 接続待ちに失敗
	CSERVER_THREAD_RET_ERROR_SYSTEM = 0xE9999999,							// システム異常
} CSERVER_THREAD_RET_ENUM;


// サーバー情報構造体
typedef struct
{
	int								Socket;
	struct sockaddr_in				tAddr;
} CSERVER_SERVER_INFO_TABLE;


// クライアント情報構造体
typedef struct
{
	int								Socket;
	struct sockaddr_in				tAddr;
} CSERVER_CLIENT_INFO_TABLE;



class CServerThread : public CThread
{
private:
	bool							m_bInitFlag;						// 初期化完了フラグ
	int								m_ErrorNo;							// エラー番号

	CSERVER_SERVER_INFO_TABLE		m_tServerInfo;						// サーバー情報
	CThread* m_pcConnectionMonitoringThread;		// 接続監視スレッド

public:
	CServerThread();
	~CServerThread();
	CSERVER_THREAD_RET_ENUM Start();
	CSERVER_THREAD_RET_ENUM Stop();


private:
	static void* ConnectionMonitoringThreadLauncher(void* pUserData);
	CSERVER_THREAD_RET_ENUM ConnectionMonitoringThread_Start(CSERVER_SERVER_INFO_TABLE& tServerInfo);
	CSERVER_THREAD_RET_ENUM ConnectionMonitoringThread_End(CSERVER_SERVER_INFO_TABLE& tServerInfo);
	void ConnectionMonitoringThread(void* pUserData);
	int FdSet(int* pFd, ssize_t FdNum, fd_set& FdSet);
};


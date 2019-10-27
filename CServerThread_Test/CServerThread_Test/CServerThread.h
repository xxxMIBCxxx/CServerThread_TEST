#pragma once
//*****************************************************************************
// ServerThread�N���X
//*****************************************************************************
#include "CThread.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>



typedef enum
{
	CSERVER_THREAD_RET_SECCESS = 0x00000000,								// ����I��
	CSERVER_THREAD_RET_ERROR_INIT = 0xE00000001,							// ���������Ɏ��s���Ă���
	CSERVER_THREAD_RET_ERROR_STARTED = 0xE00000002,							// ���ɃX���b�h���J�n���Ă���
	CSERVER_THREAD_RET_ERROR_START = 0xE00000003,							// �X���b�h�J�n�Ɏ��s���܂���

	CSERVER_THREAD_RET_ERROR_PARAM = 0xE1000000,							// �p�����[�^�G���[
	CSERVER_THREAD_RET_ERROR_CREATE_SOCKET = 0xE1000001,					// �\�P�b�g�����Ɏ��s
	CSERVER_THREAD_RET_ERROR_BIND = 0xE1000002,								// �\�P�b�g�̖��O���Ɏ��s
	CSERVER_THREAD_RET_ERROR_LISTEN = 0xE1000003,							// �ڑ��҂��Ɏ��s
	CSERVER_THREAD_RET_ERROR_SYSTEM = 0xE9999999,							// �V�X�e���ُ�
} CSERVER_THREAD_RET_ENUM;


// �T�[�o�[���\����
typedef struct
{
	int								Socket;
	struct sockaddr_in				tAddr;
} CSERVER_SERVER_INFO_TABLE;


// �N���C�A���g���\����
typedef struct
{
	int								Socket;
	struct sockaddr_in				tAddr;
} CSERVER_CLIENT_INFO_TABLE;



class CServerThread : public CThread
{
private:
	bool							m_bInitFlag;						// �����������t���O
	int								m_ErrorNo;							// �G���[�ԍ�

	CSERVER_SERVER_INFO_TABLE		m_tServerInfo;						// �T�[�o�[���
	CThread* m_pcConnectionMonitoringThread;		// �ڑ��Ď��X���b�h

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


//*****************************************************************************
// ServerThread�N���X
//*****************************************************************************
#include "CServerThread.h"
#include <unistd.h>


#define _CSERVER_THREAD_THREAD_
#define CLIENT_CONNECT_NUM			( 5 )

//-----------------------------------------------------------------------------
// �R���X�g���N�^
//-----------------------------------------------------------------------------
CServerThread::CServerThread()
{
	m_bInitFlag = false;
	m_ErrorNo = 0;
	m_pcConnectionMonitoringThread = NULL;
	memset(&m_tServerInfo, 0x00, sizeof(m_tServerInfo));
	m_tServerInfo.Socket = -1;

	// �ڑ��Ď��X���b�h�N���X����
	m_pcConnectionMonitoringThread = (CThread *) new CThread("ConnectionMonitoring");
	if (m_pcConnectionMonitoringThread == NULL)
	{
		return;
	}

	m_bInitFlag = true;
}


//-----------------------------------------------------------------------------
// �f�X�g���N�^
//-----------------------------------------------------------------------------
CServerThread::~CServerThread()
{
	// �N���C�A���g�ڑ��v���p�X���b�h��~�Y��l��
	this->Stop();

	// �ڑ��Ď��X���b�h�N���X���
	if (m_pcConnectionMonitoringThread != NULL)
	{
		delete m_pcConnectionMonitoringThread;
		m_pcConnectionMonitoringThread = NULL;
	}
}


//-----------------------------------------------------------------------------
// �T�[�o�[�J�n
//-----------------------------------------------------------------------------
CSERVER_THREAD_RET_ENUM CServerThread::Start()
{
	CSERVER_THREAD_RET_ENUM		eRet = CSERVER_THREAD_RET_SECCESS;


	// �������`�F�b�N
	if (m_bInitFlag == false)
	{
		return CSERVER_THREAD_RET_ERROR_INIT;
	}

	// �ڑ��Ď��X���b�h�J�n
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
// �N���C�A���g�ڑ��v���p�X���b�h��~
//-----------------------------------------------------------------------------
CSERVER_THREAD_RET_ENUM CServerThread::Stop()
{
	// �������`�F�b�N
	if (m_bInitFlag == false)
	{
		return CSERVER_THREAD_RET_ERROR_INIT;
	}

	// �ڑ��Ď��X���b�h��~
	ConnectionMonitoringThread_End(m_tServerInfo);

	return CSERVER_THREAD_RET_SECCESS;
}


//-----------------------------------------------------------------------------
// �ڑ��Ď��X���b�h�J�n
//-----------------------------------------------------------------------------
CSERVER_THREAD_RET_ENUM CServerThread::ConnectionMonitoringThread_Start(CSERVER_SERVER_INFO_TABLE& tServerInfo)
{
	int					iRet = 0;
	CTHREAD_RET_ENUM	eRet = CTHREAD_RET_ERROR_START;
	bool				bRet = false;


	// �n������
	if (m_pcConnectionMonitoringThread == NULL)
	{
		return CSERVER_THREAD_RET_ERROR_SYSTEM;
	}

	// ���ɃX���b�h�����삵�Ă���ꍇ
	bRet = m_pcConnectionMonitoringThread->IsActive();
	if (bRet == true)
	{
		return CSERVER_THREAD_RET_ERROR_STARTED;
	}

	// �T�[�o�[���̃\�P�b�g�𐶐�
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

	// �T�[�o�[����IP Address�EPort��ݒ�
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

	// �N���C�A���g����ڑ���҂�
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

	// �ڑ��Ď��X���b�h�J�n
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
// �ڑ��Ď��X���b�h��~
//-----------------------------------------------------------------------------
CSERVER_THREAD_RET_ENUM CServerThread::ConnectionMonitoringThread_End(CSERVER_SERVER_INFO_TABLE& tServerInfo)
{
	bool				bRet = false;


	// �n������
	if (m_pcConnectionMonitoringThread == NULL)
	{
		return CSERVER_THREAD_RET_ERROR_SYSTEM;
	}

	// ���ɃX���b�h����~���Ă���ꍇ
	bRet = m_pcConnectionMonitoringThread->IsActive();
	if (bRet == false)
	{
		return CSERVER_THREAD_RET_SECCESS;
	}

	// �ڑ��Ď��X���b�h��~
	m_pcConnectionMonitoringThread->Stop();

	// �\�P�b�g�̉��
	if (tServerInfo.Socket != -1)
	{
		close(tServerInfo.Socket);
		tServerInfo.Socket = -1;
	}

	return CSERVER_THREAD_RET_SECCESS;
}


//-----------------------------------------------------------------------------
// �ڑ��Ď��X���b�h�Ăяo��
//-----------------------------------------------------------------------------
void* CServerThread::ConnectionMonitoringThreadLauncher(void* pUserData)
{
	// �X���b�h�����Ăяo��
	reinterpret_cast<CServerThread*>(pUserData)->ConnectionMonitoringThread(pUserData);

	return (void*)NULL;
}


//-----------------------------------------------------------------------------
// �ڑ��Ď��X���b�h
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

	// �X���b�h�I���v��������܂Ń��[�v
	while (bLoop)
	{
		// �N���C�A���g�ڑ��v���҂�
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
			// �^�C���A�E�g
			continue;
		}
		else
		{
			// �X���b�h�I���C�x���g
			bRet = FD_ISSET(fdRead[0], &ReadFdSet);
			if (bRet == true)
			{
				bLoop = false;
				continue;
			}
			
			// �ڑ��v��
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
// �t�@�C���f�B�X�N���v�^���f�B�X�N���v�^�W���ɐݒ肷��
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

	// �t�@�C���f�B�X�N���v�^���Z�b�g���Ȃ���ő�l��T��
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
//*****************************************************************************
// Threadクラス
// ※リンカオプションに「-pthread」を追加すること
//*****************************************************************************
#include "CThread.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <unistd.h>


//-----------------------------------------------------------------------------
// コンストラクタ
//-----------------------------------------------------------------------------
CThread::CThread(const char* pszId)
{
	int			iRet = 0;


	m_strId = "";
	m_bInitFlag = false;
	m_iError = 0;
	m_hThread = 0;
	
	// クラス名を保持
	if (pszId != NULL)
	{
		m_strId = pszId;
	}

	// ミューテック属性の初期化
	pthread_mutexattr_init(&m_tMutexAttr);
	iRet = pthread_mutexattr_settype(&m_tMutexAttr,PTHREAD_PROCESS_PRIVATE);		// スレッド間の排他
	if (iRet != 0)
	{
		return;
	}

	// スレッド終了用イベント初期化
	CEVENT_RET_ENUM eEventRet = m_cThreadEndEvent.Init();
	if (eEventRet != CEVENT_RET_SUCCESS)
	{
		return;
	}

	m_bInitFlag = true;
}


//-----------------------------------------------------------------------------
// デストラクタ
//-----------------------------------------------------------------------------
CThread::~CThread()
{
	// スレッドが停止していないことを考慮
	this->Stop();

	// ミューテックス属性オブジェクトを破棄
	pthread_mutexattr_destroy(&m_tMutexAttr);
}


//-----------------------------------------------------------------------------
// スレッド開始
//-----------------------------------------------------------------------------
CTHREAD_RET_ENUM CThread::Start(PTHREAD_LAUNCHER_FUNC pThreadLancherFunc)
{
	int			iRet = 0;


	// 初期化処理で失敗している場合
	if (m_bInitFlag == false)
	{
		return CTHREAD_RET_ERROR_INIT;
	}

	// 既に動作している場合
	if (m_hThread != 0)
	{
		return CTHREAD_RET_ERROR_AlREADY_STARTED;
	}

	// スレッド開始
	if (pThreadLancherFunc == NULL)
	{
		iRet = pthread_create(&m_hThread, NULL, ThreadLauncher, this);
	}
	else
	{
		iRet = pthread_create(&m_hThread, NULL, pThreadLancherFunc, this);
	}

	if (iRet != 0)
	{
		m_iError = errno;
		printf("pthread_create Error. [errno：%d]\n", m_iError);
		printf("-> %s\n", strerror(m_iError));
		return CTHREAD_RET_ERROR_START;
	}

	return CTHREAD_RET_SUCCESS;
}


//-----------------------------------------------------------------------------
// スレッド停止
//-----------------------------------------------------------------------------
CTHREAD_RET_ENUM CThread::Stop()
{
	CEVENT_RET_ENUM			eRet = CEVENT_RET_SUCCESS;


	// 初期化処理で失敗している場合
	if (m_bInitFlag == false)
	{
		return CTHREAD_RET_ERROR_INIT;
	}

	// 既に停止している場合
	if (m_hThread == 0)
	{
		return CTHREAD_RET_SUCCESS;
	}

	// スレッドを停止させる
	eRet = m_cThreadEndEvent.SetEvent();
	if (eRet != CEVENT_RET_SUCCESS)
	{
		// スレッド停止に失敗した場合は、強制的に終了させる
		pthread_cancel(m_hThread);
	}
	pthread_join(m_hThread, NULL);
	m_hThread = 0;

	return CTHREAD_RET_SUCCESS;
}


//-----------------------------------------------------------------------------
// エラー番号を取得
//-----------------------------------------------------------------------------
int CThread::GetErrorNo()
{
	return m_iError;
}


//-----------------------------------------------------------------------------
// スレッドが動作しているかの確認
//-----------------------------------------------------------------------------
bool CThread::IsActive()
{
	return ((m_hThread != 0) ? true : false);
}

//-----------------------------------------------------------------------------
// スレッド停止イベントファイルディスクリプタを取得
//-----------------------------------------------------------------------------
int CThread::GetEdfThreadEndEvent()
{
	return m_cThreadEndEvent.GetEdf();
}


//-----------------------------------------------------------------------------
// スレッド呼び出し
//-----------------------------------------------------------------------------
void* CThread::ThreadLauncher(void* pUserData)
{
	// スレッド処理呼び出し
	reinterpret_cast<CThread*>(pUserData)->ThreadProc();

	return (void *)NULL;
}



//-----------------------------------------------------------------------------
// スレッド処理（※サンプル※）
//-----------------------------------------------------------------------------
void CThread::ThreadProc()
{
	CEVENT_RET_ENUM eRet = CEVENT_RET_SUCCESS;
	DWORD			dwCount = 1;
	bool			bLoop = true;
	DWORD			dwTimeout = 0;


	printf("-- Thread %s Start --\n", m_strId.c_str());


	// pthread_testcancelが呼ばれるまで処理を続ける
	while (bLoop)
	{
		// Sleep時間を生成
		dwTimeout = ((rand() % 30) + 1) * 100;

		// 終了イベントが通知を待つ
		eRet = m_cThreadEndEvent.Wait(dwTimeout);
		switch (eRet) {
		case CEVENT_RET_WAIT_TIMEOUT:
//			pthread_mutex_lock(&(this->m_hMutex));
			printf("[%s] Count : %lu \n", this->m_strId.c_str(), dwCount++);
//			pthread_mutex_unlock(&(this->m_hMutex));
			break;

		case CEVENT_RET_RECIVE_EVENT:
			m_cThreadEndEvent.ResetEvent();
			bLoop = false;
			break;

		default:
			printf("CEvent::Wait error. \n");
			bLoop = false;
			break;
		}
	}

	printf("-- Thread %s End --\n", m_strId.c_str());
}











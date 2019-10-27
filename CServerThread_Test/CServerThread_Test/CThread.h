#pragma once
//*****************************************************************************
// Threadクラス
// ※リンカオプションに「-pthread」を追加すること
//*****************************************************************************
#include <cstdio>
#include <pthread.h>
#include <errno.h> 
#include <string.h>
#include "string"
#include "Type.h"
#include "CEvent.h"


// スレッドの呼出し関数定義
typedef void* (*PTHREAD_LAUNCHER_FUNC) (void*);


// CThreadクラスの結果種別
typedef enum
{
	CTHREAD_RET_SUCCESS = 0x00000000,					// 正常終了
	CTHREAD_RET_ERROR_INIT = 0xE00000001,				// 初期処理に失敗している
	CTHREAD_RET_ERROR_AlREADY_STARTED = 0xE00000002,	// 既にスレッドを開始している
	CTHREAD_RET_ERROR_START = 0xE00000003,				// スレッド開始に失敗しました

	CTHREAD_RET_ERROR_SYSTEM = 0xE9999999				// システム異常
} CTHREAD_RET_ENUM;


// CThreadクラス定義
class CThread
{
private:
	std::string					m_strId;				// 識別名
	bool						m_bInitFlag;			// 初期化フラグ
	int							m_iError;				// エラー番号
	pthread_t					m_hThread;				// スレッドハンドル
	pthread_mutexattr_t			m_tMutexAttr;			// ミューテックス属性
	pthread_mutex_t				m_hMutex;				// ミューテックスハンドル
	CEvent						m_cThreadEndEvent;		// スレッド終了イベント

public:
	CThread(const char* pszId = NULL);
	~CThread();
	CTHREAD_RET_ENUM Start(PTHREAD_LAUNCHER_FUNC pThreadLancherFunc = NULL);
	CTHREAD_RET_ENUM Stop();
	int GetErrorNo();
	bool IsActive();
	int GetEdfThreadEndEvent();

	virtual void ThreadProc();

private:
	static void* ThreadLauncher(void* pUserData);

};


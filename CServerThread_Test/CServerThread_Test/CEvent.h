#pragma once
//*****************************************************************************
// Eventクラス
//*****************************************************************************
#include "Type.h"


typedef enum
{
	CEVENT_RET_SUCCESS = 0x00000000,				// 正常
	CEVENT_RET_RECIVE_EVENT = 0x01000000,			// イベント待ちにてイベントを受信
	CEVENT_RET_WAIT_TIMEOUT = 0x09999999,			// イベント待ちにてタイムアウトが発生
	CEVENT_RET_ERROR_EVENT_FD = 0xE0000001,			// イベントファイルディスクリプタが取得できなかった
	CEVENT_RET_ERROR_EVENT_SET = 0xE0000002,		// イベント設定失敗
	CEVENT_RET_ERROR_EVENT_RESET = 0xE0000003,		// イベントリセット失敗
	CEVENT_RET_ERROR_EVENT_WAIT = 0xE0000004,		// イベント待ち失敗
	CEVENT_RET_ERROR_SYSTEM = 0xE9999999,			// システム異常
} CEVENT_RET_ENUM;


class CEvent
{
private:
	int					m_efd;				// イベントファイルディスクリプタ
	int					m_errno;			// エラー番号

public:
	CEvent();
	~CEvent();
	CEVENT_RET_ENUM Init();
	int GetEdf();
	int GetErrorNo();
	CEVENT_RET_ENUM SetEvent();
	CEVENT_RET_ENUM ResetEvent();
	CEVENT_RET_ENUM Wait(DWORD dwTimeout = 0);
};


#pragma once

#include <atlmisc.h>
#include "Constants.h"
#include "INet.h"
#include "Constants.h"

class Orchestrator;
class CMainDlg;

class LongPoll {
public:
	LongPoll(void);
	~LongPoll(void);
	int EndRequestNow();
	int ReceivedMessage(CString *text);
	int DoLongPoll();
	int GoOffline();
	DWORD StartThread();
	int connected;

	static DWORD WINAPI RunThread(LPVOID lpData);

	DWORD threadID;
	Orchestrator *orchestrator;
	int timeToQuit;
	int pollsWithoutQBRequest;
	bool firstTime;
	bool firstError;
	HANDLE signal;
	HANDLE goOfflineSignal;
	HANDLE tryAgainSignal;
	HANDLE threadHandle;
	HANDLE connectedSignal;
	HINTERNET currentHandle;
	CString state;

private:
	void ClearExpiredQBSession();
};
#pragma once

#include <atlmisc.h>
#include "Constants.h"
#include "ResponseEnvelope.h"

class CMainDlg;
class Orchestrator;

class ResponseDispatcher {
public:
	ResponseDispatcher(void);
	~ResponseDispatcher(void);
	DWORD StartThread();
	int SendBack(CString respKey, CString reply);
	static DWORD WINAPI RunThread(LPVOID lpData);
	
	DWORD threadID;
	Orchestrator *orchestrator;
	HANDLE signal;
	HANDLE threadHandle;
	CMainDlg* mainDlg;
	ResponseEnvelope lastEnvelope;
};
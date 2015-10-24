#pragma once

#include "SpawnCanary.h"
#include "EventHandler.h"
#include "Info.h"
#include "MainDlg.h"

class Orchestrator {
public:
	SpawnCanary spawnCanary;
	EventHandler eventHandler;
	Info info;
	int started;
	CString shellPID;
	HANDLE goOfflineSignal;
	DWORD mainThreadID;
	CMainDlg* cMainDlg;

	Orchestrator() : started(0) {
		// Give everybody access to the Orchestrator pointer
		spawnCanary.orchestrator = this;
		goOfflineSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	int StartConcert() {
		started = 1;
		return 1;
	}

	int StopConcert() {
		ATLTRACE2(atlTraceUI, 0, _T("::And audience applauds...\n"));
		PostMessage(this->cMainDlg->m_hWnd, WM_CLOSE, NULL, NULL);
		// PostThreadMessage(this->mainThreadID, WM_DESTROY, NULL, NULL);
		return 1;
	}
};
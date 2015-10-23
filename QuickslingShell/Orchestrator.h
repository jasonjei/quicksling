#pragma once

#include "SpawnCanary.h"

class CMainDlg;

class Orchestrator {
public:
	SpawnCanary spawnCanary;
	CMainDlg* cMainDlg;
	int started;
	CString shellPID;
	HANDLE goOfflineSignal;
	DWORD mainThreadID;

	Orchestrator() : started(0) {
		// Give everybody access to the Orchestrator pointer
		spawnCanary.orchestrator = this;
		goOfflineSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
	}


	int StopConcert() {
		ATLTRACE2(atlTraceUI, 0, _T("::And audience applauds...\n"));

		PostThreadMessage(mainThreadID, WM_QUIT, NULL, NULL);

		return 1;
	}
};
#pragma once

#include "SpawnCanary.h"
#include "EventHandler.h"
#include "Info.h"
#include "MainDlg.h"
#include "Downloader.h"
#include "spdlog\spdlog.h"

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
	HANDLE ghJob;
	Downloader downloader;
	HINSTANCE hInstance;

	Orchestrator() : started(0) {
		auto l = spdlog::get("quicksling_shell");

		// Give everybody access to the Orchestrator pointer
		spawnCanary.orchestrator = this;
		goOfflineSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
		ghJob = CreateJobObject(NULL, NULL);

		JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };

		// Configure all child processes associated with the job to terminate when the
		jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		if (0 == SetInformationJobObject(ghJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
		{
			l->error("Couldn't set job object");
			::MessageBox(0, _T("Could not SetInformationJobObject"), _T("TEST"), MB_OK);
		}
	}

	int StartConcert() {
		started = 1;
		return 1;
	}

	int StopConcert() {
		ATLTRACE2(atlTraceUI, 0, _T("::And audience applauds...\n"));
		PostMessage(this->cMainDlg->m_hWnd, WM_CLOSE, NULL, NULL);
		WaitForSingleObject(this->spawnCanary.threadHandle, INFINITE);
		// PostThreadMessage(this->mainThreadID, WM_DESTROY, NULL, NULL);
		return 1;
	}
};
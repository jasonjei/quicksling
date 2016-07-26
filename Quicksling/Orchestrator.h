#pragma once

#include "LongPoll.h"
#include "RequestProcessor.h"
#include "ResponseDispatcher.h"
#include "QBInfo.h"
#include "BrowserHandler.h"
#include "NetAware.h"

class CMainDlg;
class QBInfo;

class Orchestrator {
public:
	LongPoll longPoll;
	RequestProcessor request;
	ResponseDispatcher response;
	NetAware netAware;
	QBInfo qbInfo;
	BrowserHandler browser;
	CMainDlg* cMainDlg;
	CString shellVersion;
	bool lostDataEvents;
	std::vector<CString> dataEvents;
	std::vector<CString> newDataEvents;
	int started;
	bool brainRequestShutdown;
	CString shellPID;
	DWORD mainThreadID;

	Orchestrator() : started(0), brainRequestShutdown(false) {
		// Give everybody access to the Orchestrator pointer
		qbInfo.orchestrator = this;
		longPoll.orchestrator = this;
		response.orchestrator = this;
		request.orchestrator = this;
		lostDataEvents = false;
	}

	int SetDlgMain(CMainDlg *mainDlg) {
		cMainDlg = mainDlg;
		request.mainDlg = mainDlg;
		response.mainDlg = mainDlg;
		return 0;
	}

	int StartConcert() {
		if (!started) {
			request.StartThread();
			WaitForSingleObject(request.signal, INFINITE);
			response.StartThread();
			WaitForSingleObject(response.signal, INFINITE);
			// This starts the chain of events
			longPoll.StartThread();
			netAware.StartThread();
		}

		started = 1;

		return 1;
	}

	int StopConcert() {
		longPoll.GoOffline();
		::PostThreadMessage(this->longPoll.threadID, WM_QUIT, NULL, NULL);
		WaitForSingleObject(this->longPoll.threadHandle, INFINITE);
		::PostThreadMessage(this->request.threadID, WM_QUIT, NULL, NULL);
		WaitForSingleObject(this->request.threadHandle, INFINITE);

		if (this->qbInfo.persistentQBXMLWrapper != NULL) {
			delete this->qbInfo.persistentQBXMLWrapper;
			this->qbInfo.persistentQBXMLWrapper = NULL;
		}

		::PostThreadMessage(this->response.threadID, WM_QUIT, NULL, NULL);
		WaitForSingleObject(this->response.threadHandle, INFINITE);

		ATLTRACE2(atlTraceUI, 0, _T("::And audience applauds...\n"));

		return 1;
	}
};
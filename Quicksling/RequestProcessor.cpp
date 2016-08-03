#pragma once

#include "stdafx.h"
#include "RequestProcessor.h"
#include "Orchestrator.h"
#include "ResponseEnvelope.h"
#include "MainDlg.h"
#include <regex>
#include <time.h>

extern Orchestrator *defaultOrchestrator;
extern std::mutex mutexDataEvents;

class RequestProcessorMessageIdler : public CIdleHandler {
public:
	RequestProcessor* request;
	virtual BOOL OnIdle() {
		SetEvent(request->signal);
		return 0;
	}
};


class RequestProcessorMessageFilter : public CMessageFilter {
public:
	RequestProcessor* request;
	virtual BOOL PreTranslateMessage(MSG* pMsg) {
		ResetEvent(request->signal);
		return 0;
	}
};

class RequestProcessorMessageLoop : public CMessageLoop {
public:
	RequestProcessor* request;
	Actions actions;

	virtual BOOL PreTranslateMessage(MSG* pMsg) {
		// loop backwards
		for (int i = m_aMsgFilter.GetSize() - 1; i >= 0; i--) {
			CMessageFilter* pMessageFilter = m_aMsgFilter[i];
			if (pMessageFilter != NULL && pMessageFilter->PreTranslateMessage(pMsg))
				return TRUE;
		}

		/* Poll -> RequestProcessor Message Received
		* (CString) wParam
		*/
		if (pMsg->message == LEVION_REQUEST) {
			ResponseEnvelope *envelope = new ResponseEnvelope;
			request->ProcessText(pMsg, envelope);

			const std::string str(CT2A(envelope->message, CP_UTF8));

			actions.Call(str, request, envelope);
			
			WaitForSingleObject(request->orchestrator->response.signal, INFINITE);
			
			if (envelope->responseKey == _T("0")) {
				envelope->doNotReply = true;
			}

			if (envelope->doNotReply == false) {
				PostThreadMessage(request->orchestrator->response.threadID, LEVION_RESPONSE, (WPARAM)envelope, NULL);
			}
			else {
				delete envelope;
			}

			delete ((CString*)pMsg->wParam);
		}
		else if (pMsg->message == LEVION_END_SESSION) {
			request->orchestrator->qbInfo.EndSession();
		}

		return FALSE;   // not translated
	}
};

RequestProcessor::RequestProcessor(void) {
	threadID = 0;
	signal = CreateEvent(NULL, TRUE, FALSE, NULL);
}

RequestProcessor::~RequestProcessor(void) {
}

struct _RunData {
	RequestProcessor* requestInstance;
};

DWORD RequestProcessor::StartThread() {
	_RunData *pData = new _RunData;
	pData->requestInstance = this;

	threadHandle = ::CreateThread(NULL, 0, RequestProcessor::RunThread, pData, 0, &threadID);
	WaitForSingleObject(orchestrator->request.signal, INFINITE);
	return threadID;
}

DWORD WINAPI RequestProcessor::RunThread(LPVOID lpData) {
	RequestProcessorMessageLoop theLoop;
	RequestProcessorMessageIdler theIdler;
	RequestProcessorMessageFilter theFilter;

	_RunData* pData = (_RunData*)lpData;
	RequestProcessor* request = pData->requestInstance;

	HWND hRequestProcessor = reinterpret_cast<HWND>(pData->requestInstance->mainDlg);

	theLoop.request = pData->requestInstance;
	theIdler.request = pData->requestInstance;
	theFilter.request = pData->requestInstance;

	theLoop.AddMessageFilter(&theFilter);
	theLoop.AddIdleHandler(&theIdler);

	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	int nRet = theLoop.Run();
	delete pData;
	::CoUninitialize();
	return 1;
}

void RequestProcessor::ProcessText(MSG *pMsg, ResponseEnvelope *res) {
	/* Prepare RegEx */
	const wchar_t *reg = _T("^(\\d+):(\\S{0,})");
	std::wregex rgx(reg);

	/* Convert CString to C++ WString */
	CString result = *((CString*)pMsg->wParam);
	std::wstring cppResult(result);

	/* Attempt Match */
	std::match_results<std::wstring::const_iterator> matchResults;
	bool match = std::regex_search(cppResult, matchResults, rgx);

	if (match) {
		res->responseKey = WTL::CString(matchResults[1].str().c_str());
		res->message = WTL::CString(matchResults[2].str().c_str());
		res->body = WTL::CString(matchResults[2].second._Ptr);
		res->body.TrimLeft();
	}
}

int RequestProcessor::cmd_show_message(ResponseEnvelope* res) {
	// TrayMessage *trayMessage = BuildTrayMessage(_T("Levion"), res->body);
	//SendMessage(this->orchestrator->cMainDlg->m_hWnd, LEVION_MESSAGE_BOX, (WPARAM)trayMessage, NULL);
	CString *toDelete = new CString(res->body);
	PostMessage(this->orchestrator->cMainDlg->m_hWnd, LEVION_MESSAGE_BOX, (WPARAM)toDelete, NULL);
	res->reply = _T("OK");
	return 1;
}


int RequestProcessor::cmd_subscribexml(ResponseEnvelope* res) {
	// TrayMessage *trayMessage = BuildTrayMessage(_T("Levion"), res->body);
	//SendMessage(this->orchestrator->cMainDlg->m_hWnd, LEVION_MESSAGE_BOX, (WPARAM)trayMessage, NULL);
	qbXMLRPWrapper qbWrapper;
	res->reply = qbWrapper.ProcessSubscription((LPCTSTR) res->body).c_str();
	return 1;
}

/* This is a catch-all command if we can't find a proper routing */
int RequestProcessor::cmd_misunderestimate(ResponseEnvelope* res) {
	res->reply = _T("MISUNDERESTIMATE_MSG_OK");
	return 1;
}

/* Returns Coordinated Universal Time (UTC) in format Levion requires */
int RequestProcessor::cmd_utctime(ResponseEnvelope* res) {
	struct tm currentTime;
	__int64 ltime;

	_time64(&ltime);
	_gmtime64_s(&currentTime, &ltime);

	res->reply.Format(_T("%02d/%02d/%04d %02d:%02d:%02d"), currentTime.tm_mon + 1,
		currentTime.tm_mday, currentTime.tm_year + 1900, currentTime.tm_hour,
		currentTime.tm_min, currentTime.tm_sec);

	return 1;
}

int RequestProcessor::cmd_qb(ResponseEnvelope* res) {
	if (WaitForSingleObject(this->orchestrator->longPoll.goOfflineSignal, 0) == 0) {
		res->reply = _T("SHUTDOWN");
		return 1;
	}

	this->orchestrator->qbInfo.processedQBRequest = true;

	if (res->body.Compare(_T("versions")) == 0) {
		// if (this->orchestrator->qbInfo.qbxmlVersions.Compare(_T("")) == 0)
		this->orchestrator->qbInfo.GetInfoFromQB();
		res->reply = this->orchestrator->qbInfo.qbxmlVersions;
	}
	else if (res->body.Compare(_T("startsession")) == 0) {
		this->orchestrator->qbInfo.StartSession();
		res->reply = "OK";
	}
	else if (res->body.Compare(_T("endsession")) == 0) {
		this->orchestrator->qbInfo.EndSession();
		res->reply = "OK";
	}
	else if (res->body.Compare(_T("query_lost_data_events")) == 0) {

	}
	else if (res->body.Compare(_T("clear_lost_data_events")) == 0) {

		this->orchestrator->lostDataEvents = false;
		res->reply = this->orchestrator->qbInfo.ProcessQBXMLRequest(DATA_EVENT_RECOVERY_DELETE);
		this->orchestrator->dataEvents.clear();
		this->orchestrator->newDataEvents.clear();
	}
	else {
		res->reply = this->orchestrator->qbInfo.ProcessQBXMLRequest(res->body);
	}
	return 1;
}

int RequestProcessor::cmd_utcoffset(ResponseEnvelope *res) {
	TIME_ZONE_INFORMATION tz;
	GetTimeZoneInformation(&tz);
	res->reply.Format(_T("%d"), (0 - tz.Bias * 60));
	return 1;
}

int RequestProcessor::cmd_closemodal(ResponseEnvelope *res) {
	qbXMLRPWrapper qbWrapper;
	qbWrapper.CloseModal();
	res->reply = _T("OK");
	return 1;
}

int RequestProcessor::cmd_debug_timeout(ResponseEnvelope *res) {
	CString sURL;
	sURL.Format(URLS::GOLIATH_SERVER + _T("debug/clients"));
	CInternetSession Session(APP_NAME);
	CHttpFile cHttpFile(Session, sURL, NULL, 0, INTERNET_FLAG_DONT_CACHE);
	WTL::CString pageSource;

	UINT bytes = (UINT)cHttpFile.GetLength();

	char tChars[2048 + 1];
	int bytesRead;

	while ((bytesRead = cHttpFile.Read((LPVOID)tChars, 2048)) != 0) {
		tChars[bytesRead] = '\0';
		pageSource += tChars;
	}

	pageSource.Format(_T("%s"), pageSource);
	res->reply = pageSource;
	return 1;
}

int RequestProcessor::cmd_get_events(ResponseEnvelope *res) {
	// We clear new data events to differentiate from the events being 
	// sent to the server now versus events to be sent to the app server
	// for later.  In essence, this is the start of a new queue.
	{
		std::lock_guard<std::mutex> guard(mutexDataEvents);
		defaultOrchestrator->newDataEvents.clear();

		std::wstringstream ss;
		for (size_t i = 0; i < defaultOrchestrator->dataEvents.size(); ++i)
		{
			if (i != 0)
				ss << "|-|";
			ss << (LPCTSTR) defaultOrchestrator->dataEvents[i];
		}
		std::wstring s = ss.str();

		res->reply = s.c_str(); // data_events;
	}
	return 1;
}

int RequestProcessor::cmd_browser(ResponseEnvelope *res) {
	std::string url(CW2A(res->body, CP_UTF8));

	/* 
	
	if (defaultOrchestrator->browser.browser == NULL)
		defaultOrchestrator->browser.StartThread();
	WaitForSingleObject(defaultOrchestrator->browser.browserOpenSignal, INFINITE);

	defaultOrchestrator->browser.browser->GetMainFrame()->LoadURL(url);
	*/ 
	res->reply = "OK";
	return 1;
}

int RequestProcessor::cmd_quit(ResponseEnvelope *res) {
	// CString *newString = new CString("shutdown");
	// ::PostThreadMessage(defaultOrchestrator->pipeWrite.threadID, PIPE_REQUEST, (WPARAM)newString, NULL);

	res->reply = "OK";
	// Figure out a better way to quit so that a response is sent first
	//PostMessage(defaultOrchestrator->cMainDlg->m_hWnd, WM_CLOSE, NULL, NULL);
	//PostMessage(defaultOrchestrator->request.mainDlg->m_hWnd, WM_CLOSE, NULL, NULL);

	return 1;
}

int RequestProcessor::cmd_update(ResponseEnvelope *res) {
	res->reply = "OK";

	CString strWindowTitle = _T("QuickletShellEventsProcessor");
	CString strDataToSend = _T("update_requested");

	LRESULT copyDataResult;

	CWindow pOtherWnd = (HWND)FindWindow(NULL, strWindowTitle);

	if (pOtherWnd) {
		COPYDATASTRUCT cpd;
		cpd.dwData = NULL;
		cpd.cbData = strDataToSend.GetLength() * sizeof(wchar_t) + 1;
		cpd.lpData = strDataToSend.GetBuffer(cpd.cbData);
		copyDataResult = pOtherWnd.SendMessage(WM_COPYDATA,
			(WPARAM) defaultOrchestrator->cMainDlg->m_hWnd,
			(LPARAM)&cpd);
		strDataToSend.ReleaseBuffer();
		// copyDataResult has value returned by other app

	}

	PostMessage(defaultOrchestrator->cMainDlg->m_hWnd, WM_CLOSE, NULL, NULL);

	// Send update command to shell
	// CString *newString = new CString("update");
	// ::PostThreadMessage(defaultOrchestrator->pipeWrite.threadID, PIPE_REQUEST, (WPARAM)newString, NULL);

	// Shell checks for updates. If there are updates, shell will shutdown brain then download updates.

	// If there are not updates, shell will send "noupdate" to brain through pipe, triggering a tray message saying "Levion is up to date"
	return 1;
}

int RequestProcessor::cmd_crash_shell(ResponseEnvelope *res) {
	res->reply = "OK";

	CString strWindowTitle = _T("QuickletShellEventsProcessor");
	CString strDataToSend = _T("crash");

	LRESULT copyDataResult;

	CWindow pOtherWnd = (HWND)FindWindow(NULL, strWindowTitle);

	if (pOtherWnd) {
		COPYDATASTRUCT cpd;
		cpd.dwData = NULL;
		cpd.cbData = strDataToSend.GetLength() * sizeof(wchar_t) + 1;
		cpd.lpData = strDataToSend.GetBuffer(cpd.cbData);
		copyDataResult = pOtherWnd.SendMessage(WM_COPYDATA,
			(WPARAM)defaultOrchestrator->cMainDlg->m_hWnd,
			(LPARAM)&cpd);
		strDataToSend.ReleaseBuffer();
		// copyDataResult has value returned by other app

	}

	// Send update command to shell
	// CString *newString = new CString("update");
	// ::PostThreadMessage(defaultOrchestrator->pipeWrite.threadID, PIPE_REQUEST, (WPARAM)newString, NULL);

	// Shell checks for updates. If there are updates, shell will shutdown brain then download updates.

	// If there are not updates, shell will send "noupdate" to brain through pipe, triggering a tray message saying "Levion is up to date"
	return 1;
}

int RequestProcessor::cmd_start_test_sync(ResponseEnvelope *res) {
	if (defaultOrchestrator->qbInfo.RegisterConnector() == 0) {
		res->reply = "failed";
	}
	else {
		res->reply = defaultOrchestrator->qbInfo.companyTag;
	}
	return 1;
}

int RequestProcessor::cmd_clear_events(ResponseEnvelope *res) {
	// EventHandler eh;
	// eh.ClearDataEvents();
	{
		std::lock_guard<std::mutex> guard(mutexDataEvents);
		defaultOrchestrator->dataEvents.clear();
		defaultOrchestrator->dataEvents.insert(std::end(defaultOrchestrator->dataEvents), std::begin(defaultOrchestrator->newDataEvents), std::end(defaultOrchestrator->newDataEvents));
	}
	res->reply = "OK";
	return 1;
}

int RequestProcessor::cmd_ping(ResponseEnvelope *res) {
	res->reply = "pong";
	return 1;
}

int RequestProcessor::cmd_query_lost_data_events(ResponseEnvelope* res) {
	res->doNotReply = true;
	CString result = this->orchestrator->qbInfo.ProcessQBXMLRequest(DATA_EVENT_RECOVERY_QUERY);
	
	if (result.Find(_T("DataEventRecoveryTime")) >= 0)
		this->orchestrator->lostDataEvents = true;
	else {
		this->orchestrator->lostDataEvents = false;
	}

	return 1;
}
#pragma once

#include "stdafx.h"
#include "ResponseDispatcher.h"
#include "MainDlg.h"
#include "Orchestrator.h"
#include "LevionMisc.h"

class ResponseDispatcherIdler : public CIdleHandler {
public:
	ResponseDispatcher* response;
	virtual BOOL OnIdle() {
		SetEvent(response->signal);
		return 0;
	}
};

class ResponseDispatcherFilter : public CMessageFilter {
public:
	ResponseDispatcher* response;
	virtual BOOL PreTranslateMessage(MSG* pMsg) { 
		ResetEvent(response->signal);
		return 0;
	}
};

class ResponseDispatcherLoop : public CMessageLoop {
public:
	ResponseDispatcher* response;
	virtual BOOL PreTranslateMessage(MSG* pMsg) {
		// loop backwards
		for (int i = m_aMsgFilter.GetSize() - 1; i >= 0; i--) {
			CMessageFilter* pMessageFilter = m_aMsgFilter[i];
			if (pMessageFilter != NULL && pMessageFilter->PreTranslateMessage(pMsg))
				return TRUE;
		}

		/* Poll -> Request Message Received
		* (CString) wParam
		*/ 
		if ((pMsg->message == LEVION_RESPONSE))
		{
			ResponseEnvelope *envelope = NULL;

			if (pMsg->wParam)
				envelope = (ResponseEnvelope*) pMsg->wParam;

			response->SendBack(envelope->responseKey, envelope->reply);

			if (envelope) {
				response->lastEnvelope = *envelope;
				delete envelope;
			}
		}

		return FALSE;
	}
};

ResponseDispatcher::ResponseDispatcher(void) {
	signal = CreateEvent(NULL, TRUE, FALSE, NULL);
}

ResponseDispatcher::~ResponseDispatcher(void) {
}

struct _RunData {
	ResponseDispatcher* responseInstance;
};

DWORD ResponseDispatcher::StartThread() {
	_RunData *pData = new _RunData;
	pData->responseInstance = this;
	threadHandle = ::CreateThread(NULL, 0, ResponseDispatcher::RunThread, pData, 0, &threadID);
	return 1;
}

DWORD WINAPI ResponseDispatcher::RunThread(LPVOID lpData) {
	ResponseDispatcherLoop theLoop;
	ResponseDispatcherIdler theIdler;
	ResponseDispatcherFilter theFilter;

	_RunData* pData = (_RunData*)lpData;
	ResponseDispatcher* response = pData->responseInstance;

	HWND hResponseDispatcher = reinterpret_cast<HWND>(pData->responseInstance->mainDlg);

	theLoop.response = pData->responseInstance;
	theIdler.response = pData->responseInstance;
	theFilter.response = pData->responseInstance;

	theLoop.AddMessageFilter(&theFilter);
	theLoop.AddIdleHandler(&theIdler);

	theLoop.Run();
	
	delete pData;
	return 0;
}

int ResponseDispatcher::SendBack(CString respKey, CString reply) {
	CInternetSession session(APP_NAME.AllocSysString());
	
	std::string sessionUrl = std::string(CW2A(URLS::GOLIATH_SERVER, CP_UTF8));
	sessionUrl = sessionUrl.substr(8, sessionUrl.length() - 9);

	URL_COMPONENTS urlcomp;

	urlcomp.dwStructSize = sizeof(urlcomp);
	urlcomp.dwSchemeLength = 1;
	urlcomp.dwHostNameLength = 1;
	urlcomp.dwUserNameLength = 1;
	urlcomp.dwPasswordLength = 1;
	urlcomp.dwUrlPathLength = 1;
	urlcomp.dwExtraInfoLength = 1;

	urlcomp.lpszScheme = NULL;
	urlcomp.lpszHostName = NULL;
	urlcomp.lpszUserName = NULL;
	urlcomp.lpszPassword = NULL;
	urlcomp.lpszUrlPath = NULL;
	urlcomp.lpszExtraInfo = NULL;

	bool resultOfCracking = InternetCrackUrl(URLS::GOLIATH_SERVER, URLS::GOLIATH_SERVER.GetLength(), 0, &urlcomp);
	std::string host = CW2A(CString(urlcomp.lpszHostName).Left(urlcomp.dwHostNameLength), CP_UTF8);

	CHttpConnection connection(session, CString(host.c_str()), urlcomp.nPort);

	CString strHeaders =_T("Content-Type: application/x-www-form-urlencoded");

	CString data = _T("response_key=") + respKey +
				   _T("&message=") + UrlEncode(reply);

	std::string ansiData = CW2A(data, CP_UTF8);

	DWORD flags = INTERNET_FLAG_RELOAD;
	
	if (urlcomp.nScheme == INTERNET_SCHEME_HTTPS)
		flags = flags | INTERNET_FLAG_SECURE;

	CHttpFile *file = new CHttpFile(connection, _T("GET"), _T("client/reply"), NULL, NULL, NULL, flags);

	try {
		file->SendRequest(strHeaders, strHeaders.GetLength(), (LPVOID) ansiData.c_str(), ansiData.size());
	}
	catch (CInternetException exception) {
		#ifdef DEBUG
		MessageBox(NULL, exception.GetErrorMessage(), _T(""), MB_OK);
		#endif
	}

	delete file;
	return 1;
}
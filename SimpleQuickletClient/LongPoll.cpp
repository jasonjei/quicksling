#pragma once

#include "stdafx.h"
#include "Conductor.h"
#include "Orchestrator.h"
#include "INet.h"
#include "LongPoll.h"
#include "LevionMisc.h"
#include <strsafe.h>
#include <exception>

extern LongPoll* defaultPoll;
extern Conductor defaultConductor;

void __stdcall CallMaster(
	HINTERNET hInternet,
	DWORD_PTR dwContext,
	DWORD dwInternetStatus,
	LPVOID lpvStatusInformation,
	DWORD dwStatusInformationLength);

LongPoll::LongPoll(void) : firstTime(true), firstError(true) {
	timeToQuit = 0;
	this->signal = CreateEvent(NULL, TRUE, FALSE, NULL);
	this->connectedSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
	this->goOfflineSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
	this->tryAgainSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
}

LongPoll::~LongPoll(void) {
}

struct _LongPollRunData {
	LongPoll* longPollInstance;
};

DWORD LongPoll::StartThread() {
	// _LongPollRunData *pData = new _LongPollRunData;
	// pData->longPollInstance = this;

	threadHandle = ::CreateThread(NULL, 0, LongPoll::RunThread, NULL, NULL, &threadID);
	return threadID;
}

DWORD WINAPI LongPoll::RunThread(LPVOID lpData) {
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	// _LongPollRunData* pData = (_LongPollRunData*)lpData;
	LongPoll* poll = &defaultConductor.orchestrator.longPoll; // pData->longPollInstance;

	defaultPoll = &defaultConductor.orchestrator.longPoll; //poll;

	DWORD res = NULL;
	CString sURL;

	while (poll->timeToQuit != 1) {
		// make a call to /clients/wait
		poll->DoLongPoll();
	}

	// free(pData);
	::CoUninitialize();
	return 0;
}

int LongPoll::GoOffline() {
	SetEvent(this->orchestrator->qbInfo.readyForLongPollSignal);
	SetEvent(this->goOfflineSignal);

	timeToQuit = 1;

	if (this->orchestrator->qbInfo.companyTag.Compare(_T("")) == 0) {
		return 1;
	}

	CString sURL = URLS::GOLIATH_SERVER + "client/offline?auth_token=" + this->orchestrator->qbInfo.authToken +
		"&client_guid=" + this->orchestrator->qbInfo.clientGuid;

	CInternetSession session(APP_NAME);
	session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 1);
	session.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 1);
	CHttpFile *cHttpFile = NULL;

	int fail = 0;

	try {
		cHttpFile = new CHttpFile(session, sURL, NULL, 0, INTERNET_FLAG_DONT_CACHE);
	}

	catch (CInternetException&) {
		fail = 1;
	}

	if (!fail) {
		WTL::CString pageSource;

		UINT bytes = (UINT)cHttpFile->GetLength();

		char tChars[2048 + 1];
		int bytesRead;

		while ((bytesRead = cHttpFile->Read((LPVOID)tChars, 2048)) != 0) {
			tChars[bytesRead] = '\0';
			pageSource += tChars;
		}

		pageSource.Format(_T("%s"), pageSource);
	}

	delete cHttpFile;
	session.Close();
	EndRequestNow();
	return 1;
}

int LongPoll::DoLongPoll() {
	WaitForSingleObject(this->orchestrator->qbInfo.readyForLongPollSignal, INFINITE);

	// If we are supposed to go offline, don't longpoll
	if (WaitForSingleObject(this->goOfflineSignal, 0) == 0)
		return 0;


	CString sURL = URLS::GOLIATH_SERVER + "client/wait?auth_key=" + this->orchestrator->qbInfo.authToken +
		"&company_tag=" + this->orchestrator->qbInfo.companyTag + "&client_guid=" + this->orchestrator->qbInfo.clientGuid;

	int numDataEvents = 0; // defaultConductor.orchestrator.eventHandler.dataEvents.size();
	if (numDataEvents > 0) {
		CString numDataEventsStr;
		numDataEventsStr.Format(_T("%i"), numDataEvents);
		sURL += "&data_events=" + numDataEventsStr;
	}

	if (firstTime == true) {
		// TODO - temp, replace with state logic
		this->orchestrator->qbInfo.state = "UNREGISTERED";
		sURL += "&version=" + this->orchestrator->qbInfo.version + "&product_name=" + this->orchestrator->qbInfo.productName +
			"&country=" + this->orchestrator->qbInfo.country + "&first_time=1";
	}

	sURL += "&state=" + this->orchestrator->qbInfo.state;

	CInternetSession Session(APP_NAME);
	
	this->currentHandle = (HINTERNET) Session;

	INTERNET_STATUS_CALLBACK iscCallback;

	iscCallback = InternetSetStatusCallback((HINTERNET)Session, (INTERNET_STATUS_CALLBACK)CallMaster);

	WORD timeout = 40000;

	Session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, timeout);
	Session.SetOption(INTERNET_OPTION_SEND_TIMEOUT, timeout);

	CHttpFile* cHttpFile = NULL;
	int fail = 0;


	try {
		cHttpFile = new CHttpFile(Session, sURL, NULL, 0, INTERNET_FLAG_DONT_CACHE);
	}

	catch (CInternetException& e) {
		fail = 1;
	}

	if (!fail) {
		WTL::CString pageSource;

		UINT bytes = (UINT)cHttpFile->GetLength();

		char tChars[2048 + 1];
		int bytesRead;

		while ((bytesRead = cHttpFile->Read((LPVOID)tChars, 2048)) != 0) {
			tChars[bytesRead] = '\0';
			pageSource += CA2W((LPCSTR)tChars, CP_UTF8);
		}

		if (pageSource == "badtag" || pageSource == "unauthorized") {
			TrayMessage *trayMessage = BuildTrayMessage(_T("Registering Company"), _T("Please enter your login information in the browser window."));
			this->orchestrator->qbInfo.RegisterConnector();
			return 0;
		}
		else if (pageSource == "timeout") {
			this->orchestrator->qbInfo.processedQBRequest = false;

		}
		else if (pageSource == "offline") {
		}
		else if (pageSource == "ONLINE") {
			this->orchestrator->longPoll.connected = true;
			this->orchestrator->qbInfo.state = "ONLINE";
			firstTime = false;
			firstError = true;
			TrayMessage *trayMessage = BuildTrayMessage(_T("Connected!"), _T("Your company is connected to Levion!"));
			SendMessage(this->orchestrator->cMainDlg->m_hWnd, LEVION_TRAYICON_MSG, (WPARAM)trayMessage, NULL);
			SendMessage(this->orchestrator->cMainDlg->m_hWnd, QUICKLET_CONNECT_UPD, NULL, NULL);

		}
		else if (pageSource.Find(_T(":/")) != -1) {
			this->ReceivedMessage(&pageSource);
		}
		else {
			if (firstError == true) {
				this->orchestrator->longPoll.connected = false;

				TrayMessage *trayMessage = BuildTrayMessage(_T("Network Error"), _T("Could not connect to Levion. Retrying...\nPlease check your internet connection."));
				SendMessage(this->orchestrator->cMainDlg->m_hWnd, LEVION_TRAYICON_MSG, (WPARAM)trayMessage, NULL);
				SendMessage(this->orchestrator->cMainDlg->m_hWnd, QUICKLET_CONNECT_UPD, NULL, NULL);

				firstError = false;
			}
			firstTime = true;

			SendMessage(this->orchestrator->cMainDlg->m_hWnd, QUICKLET_CONNECT_UPD, NULL, NULL);

			// WaitForSingleObject(this->goOfflineSignal, 15000);
			HANDLE hEvent[2];
			hEvent[0] = this->goOfflineSignal;
			hEvent[1] = this->tryAgainSignal;

			DWORD result = WaitForMultipleObjects(2, hEvent, FALSE, 15000);

			if (result != WAIT_TIMEOUT) {
				if (result == WAIT_OBJECT_0 + 1) {
					ResetEvent(this->tryAgainSignal);
				}
			}
		}
	}
	else {
		if (firstError == true) {
			this->orchestrator->longPoll.connected = false;

			TrayMessage *trayMessage = BuildTrayMessage(_T("Network Error"), _T("Could not connect to Levion. Retrying...\nPlease check your internet connection."));
			SendMessage(this->orchestrator->cMainDlg->m_hWnd, LEVION_TRAYICON_MSG, (WPARAM)trayMessage, NULL);

			firstError = false;
		}

		firstTime = true;

		SendMessage(this->orchestrator->cMainDlg->m_hWnd, QUICKLET_CONNECT_UPD, NULL, NULL);

		// WaitForSingleObject(this->goOfflineSignal, 15000);
		HANDLE hEvent[2];
		hEvent[0] = this->goOfflineSignal;
		hEvent[1] = this->tryAgainSignal;

		DWORD result = WaitForMultipleObjects(2, hEvent, FALSE, 15000);

		if (result != WAIT_TIMEOUT) {
			if (result == WAIT_OBJECT_0 + 1) {
				ResetEvent(this->tryAgainSignal);
			}
		}

	}
	delete cHttpFile;
	ResetEvent(this->connectedSignal);

	ClearExpiredQBSession();

	return 1;
}

int LongPoll::EndRequestNow() {
	::InternetCloseHandle(currentHandle);
	return 1;
}

void LongPoll::ClearExpiredQBSession() {
	if (this->orchestrator->qbInfo.persistentQBXMLWrapper != NULL) {
		if (this->orchestrator->qbInfo.processedQBRequest == false) {
			this->pollsWithoutQBRequest += 1;

			if (this->pollsWithoutQBRequest >= 3) {
				WaitForSingleObject(orchestrator->request.signal, INFINITE);
				::PostThreadMessage(orchestrator->request.threadID, LEVION_END_SESSION, NULL, NULL);
				this->pollsWithoutQBRequest = 0;
			}
		}
		else {
			this->pollsWithoutQBRequest = 0;
		}
	}
}

int LongPoll::ReceivedMessage(CString *text) {
	CString *newString = new CString(*text);
	WaitForSingleObject(orchestrator->request.signal, INFINITE);
	::PostThreadMessage(orchestrator->request.threadID, LEVION_REQUEST, (WPARAM)newString, NULL);
	return 0;
}

typedef struct{
	LongPoll*	   poll;
	int        nStatusList;  // List box control to hold callbacks
	HINTERNET  hResource;    // HINTERNET handle created by InternetOpenUrl
	char       szMemo[512];  // String to store status memo
} REQUEST_CONTEXT;

void __stdcall CallMaster(
	HINTERNET hInternet,
	DWORD_PTR dwContext,
	DWORD dwInternetStatus,
	LPVOID lpvStatusInformation,
	DWORD dwStatusInformationLength
	) {
	UNREFERENCED_PARAMETER(hInternet);
	UNREFERENCED_PARAMETER(lpvStatusInformation);
	UNREFERENCED_PARAMETER(dwStatusInformationLength);

	REQUEST_CONTEXT *cpContext;
	cpContext = (REQUEST_CONTEXT*)dwContext;
	char szStatusText[80];
	defaultPoll->signal;

	switch (dwInternetStatus)
	{
	case INTERNET_STATUS_CLOSING_CONNECTION:
		StringCchPrintfA(szStatusText,
			80,
			"%s CLOSING_CONNECTION",
			cpContext->szMemo);
		break;
	case INTERNET_STATUS_CONNECTED_TO_SERVER:
		StringCchPrintfA(szStatusText,
			80,
			"%s CONNECTED_TO_SERVER",
			cpContext->szMemo);
		break;
	case INTERNET_STATUS_CONNECTING_TO_SERVER:
		StringCchPrintfA(szStatusText,
			80,
			"%s CONNECTING_TO_SERVER",
			cpContext->szMemo);
		break;
	case INTERNET_STATUS_CONNECTION_CLOSED:
		StringCchPrintfA(szStatusText,
			80,
			"%s CONNECTION_CLOSED",
			cpContext->szMemo);
		break;
	case INTERNET_STATUS_HANDLE_CLOSING:
		StringCchPrintfA(szStatusText,
			80,
			"%s HANDLE_CLOSING",
			cpContext->szMemo);
		break;
	case INTERNET_STATUS_HANDLE_CREATED:
		StringCchPrintfA(szStatusText,
			80,
			"%s HANDLE_CREATED",
			cpContext->szMemo);
		break;
	case INTERNET_STATUS_INTERMEDIATE_RESPONSE:
		StringCchPrintfA(szStatusText,
			80,
			"%s INTERMEDIATE_RESPONSE",
			cpContext->szMemo);
		break;
	case INTERNET_STATUS_NAME_RESOLVED:
		StringCchPrintfA(szStatusText,
			80,
			"%s NAME_RESOLVED",
			cpContext->szMemo);
		break;
	case INTERNET_STATUS_RECEIVING_RESPONSE:
		StringCchPrintfA(szStatusText,
			80,
			"%s RECEIVING_RESPONSE",
			cpContext->szMemo);
		SetEvent(defaultConductor.orchestrator.longPoll.connectedSignal);
		break;
	case INTERNET_STATUS_RESPONSE_RECEIVED:
		StringCchPrintfA(szStatusText,
			80,
			"%s RESPONSE_RECEIVED",
			cpContext->szMemo);
		break;
	case INTERNET_STATUS_REDIRECT:
		StringCchPrintfA(szStatusText,
			80,
			"%s REDIRECT",
			cpContext->szMemo);
		break;
	case INTERNET_STATUS_REQUEST_COMPLETE:
		StringCchPrintfA(szStatusText,
			80,
			"%s REQUEST_COMPLETE",
			cpContext->szMemo);
		break;
	case INTERNET_STATUS_REQUEST_SENT:
		StringCchPrintfA(szStatusText,
			80,
			"%s REQUEST_SENT",
			cpContext->szMemo);
		break;
	case INTERNET_STATUS_RESOLVING_NAME:
		StringCchPrintfA(szStatusText,
			80,
			"%s RESOLVING_NAME",
			cpContext->szMemo);
		break;
	case INTERNET_STATUS_SENDING_REQUEST:
		StringCchPrintfA(szStatusText,
			80,
			"%s SENDING_REQUEST",
			cpContext->szMemo);
		break;
	case INTERNET_STATUS_STATE_CHANGE:
		StringCchPrintfA(szStatusText,
			80,
			"%s STATE_CHANGE",
			cpContext->szMemo);
		break;
	default:
		StringCchPrintfA(szStatusText,
			80,
			"%s Unknown Status %d Given",
			cpContext->szMemo,
			dwInternetStatus);
		break;
	}
}
#pragma once

#include "stdafx.h"
#include <queue>
#include "EventHandler.h"
#include "Orchestrator.h"
#include "MainDlg.h"
#import "sdkevent.dll" no_namespace named_guids raw_interfaces_only
#include <string>
#include "QBInfo.h"
#include "Constants.h"

class EventHandlerMessageIdler : public CIdleHandler {
public:
	EventHandler* eventSub;
	virtual BOOL OnIdle() {
		SetEvent(eventSub->signal);
		return 0;
	}
};

class EventHandlerMessageFilter : public CMessageFilter {
public:
	EventHandler* eventSub;
	virtual BOOL PreTranslateMessage(MSG* pMsg) { 
		ResetEvent(eventSub->signal);
		return 0;
	}
};

class EventHandlerMessageLoop : public CMessageLoop {
public:
	EventHandler* eventSub;
	virtual BOOL PreTranslateMessage(MSG* pMsg) {
		// loop backwards
		for (int i = m_aMsgFilter.GetSize() - 1; i >= 0; i--) {
			CMessageFilter* pMessageFilter = m_aMsgFilter[i];
			if (pMessageFilter != NULL && pMessageFilter->PreTranslateMessage(pMsg))
				return TRUE;
		}

		if (pMsg->message == LEVION_EVENT) {
			CString receivedQBXML;

			if (pMsg->wParam) {
				receivedQBXML = *((CString*) pMsg->wParam);
				delete ((CString*) pMsg->wParam);
			}

			eventSub->Inform(receivedQBXML);
		}

		return FALSE;
	}
};

EventHandler::EventHandler(void) {
	signal = CreateEvent(NULL, TRUE, FALSE, NULL);
}

EventHandler::~EventHandler(void) {
}

struct _RunData {
	EventHandler* eventSub_instance;
};

DWORD EventHandler::StartThread() {
	_RunData *pData = new _RunData;
	pData->eventSub_instance = this;
	threadHandle = ::CreateThread(NULL, 0, EventHandler::RunThread, pData, 0, &threadID);
	return threadID;
}

DWORD WINAPI EventHandler::RunThread(LPVOID lpData) {
	EventHandlerMessageLoop theLoop;
	EventHandlerMessageIdler theIdler;
	EventHandlerMessageFilter theFilter;

	_RunData* pData = (_RunData*)lpData;
	EventHandler* eventSub = pData->eventSub_instance;

	HWND hQBConnection = reinterpret_cast<HWND>(pData->eventSub_instance->mainDlg);

	theLoop.eventSub = pData->eventSub_instance;
	theIdler.eventSub = pData->eventSub_instance;
	theFilter.eventSub = pData->eventSub_instance;

	theLoop.AddMessageFilter(&theFilter);
	theLoop.AddIdleHandler(&theIdler);

	HRESULT hRes = ::CoInitialize(NULL);
	theLoop.Run();
	::CoUninitialize();

	delete pData;
	return 0;
}

int EventHandler::CallbackRegistered() {
	LPOLESTR lpszProgID = NULL;
	HRESULT hr = ProgIDFromCLSID( __uuidof(IQBEventCallback), &lpszProgID );
	if( lpszProgID ) {
		CoTaskMemFree( lpszProgID );
		lpszProgID = NULL;
	}

	return SUCCEEDED(hr);
}

int EventHandler::RegisterCallback() {
	// HRESULT result = _Module.UpdateRegistryFromResourceD(IDR_EVENTHANDLER, TRUE);
	DWORD err = GetLastError();

	// Translate ErrorCode to String.
	LPTSTR Error = 0;
	if(::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		0,
		(LPTSTR)&Error,
		0,
		NULL) == 0)
	{
		// Failed in translating.
	}

	// Display message.
	::MessageBox( NULL, Error, _T("GetCurrentDirectory Error"), MB_OK|MB_ICONWARNING );

	// Free the buffer.
	if( Error )
	{
		::LocalFree( Error );
		Error = 0;
	}
	HRESULT ok = _Module.RegisterServer(FALSE);
	return TRUE;
}

int EventHandler::RegisterUICallback() {
	qbXMLRPWrapper qb;
	qb.OpenCompanyFile(_T(""));
	qb.ProcessSubscription(std::wstring(UI_EVENT_SUBSCRIPTION));

	return 1;
}

int EventHandler::UnregisterUICallback() {
	qbXMLRPWrapper qb;
	// qb.OpenCompanyFile(_T(""));
	qb.ProcessSubscription(std::wstring(UI_EVENT_UNSUBSCRIPTION));
	return 0;
}
int EventHandler::UnregisterCallback() {
	// HRESULT result = _Module.UpdateRegistryFromResourceD(IDR_EVENTHANDLER, FALSE);
	DWORD err = GetLastError();

	// Translate ErrorCode to String.
	LPTSTR Error = 0;
	if(::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		0,
		(LPTSTR)&Error,
		0,
		NULL) == 0)
	{
		// Failed in translating.
	}

	// Display message.
	::MessageBox( NULL, Error, _T("GetCurrentDirectory Error"), MB_OK|MB_ICONWARNING );

	// Free the buffer.
	if( Error )
	{
		::LocalFree( Error );
		Error = 0;
	}
	HRESULT ok = _Module.UnregisterServer(TRUE);
	return TRUE;
}

int EventHandler::SubscribeDataEvents() {
	qbXMLRPWrapper qb;
	qb.OpenCompanyFile(_T(""));
	qb.ProcessSubscription(std::wstring(DATA_EVENT_SUBSCRIPTION));
	return 1;
}

int EventHandler::ClearDataEvents() {
	this->dataEvents.clear();
	return 1;
}

/* This isn't really the same inform as what's required by the COM interface
they are just coincentally named */
int EventHandler::Inform(CString strMsg) {
	int isUIEvent = this->orchestrator->qbInfo.SetState(strMsg);

	if (!isUIEvent) {
		dataEvents.push_back(strMsg);
		newDataEvents.push_back(strMsg);
		strMsg;
		//MessageBox(NULL, strMsg, _T("Our Data Event"), MB_OK);
	}

	if (this->orchestrator->qbInfo.state.Compare(_T("Close")) == 0) {
		//defaultOrchestrator->StopConcert();
		//PostMessage(defaultOrchestrator->request.mainDlg->m_hWnd, WM_CLOSE, NULL, NULL);
	}

	return 1;
}
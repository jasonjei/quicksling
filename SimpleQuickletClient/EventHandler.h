#pragma once

#include "stdafx.h"
#include <atlmisc.h>
#include "Constants.h"
#include "INet.h"
#include <map>
#include "resource.h"
#include <vector>
#import <msxml3.dll> named_guids raw_interfaces_only
#include "QBInfo.h"

class Orchestrator;
class CMainDlg;

class EventHandler {
public:
	EventHandler(void);
	~EventHandler(void);

	DWORD StartThread();
	static DWORD WINAPI RunThread(LPVOID lpData);

	DWORD threadID;
	Orchestrator *orchestrator;
	HANDLE signal;
	HANDLE threadHandle;
	CMainDlg* mainDlg;

	int Inform(CString strMsg);
	int CallbackRegistered();
	int RegisterCallback();
	int UnregisterCallback();
	int RegisterUICallback();
	int UnregisterUICallback();
	int ReadCompanyInfoAndTag();
	int SubscribeDataEvents();
	int ClearDataEvents();

	std::vector<CString> dataEvents;
	std::vector<CString> newDataEvents;
};

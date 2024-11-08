// SimpleQuickletClient.cpp : main source file for SimpleQuickletClient.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include "resource.h"

#include "aboutdlg.h"
#include "MainDlg.h"
#include "Orchestrator.h"
#include "Constants.h"
#include "Conductor.h"
#include "Settings.h"

CAppModule _Module;

Conductor defaultConductor;
Orchestrator *defaultOrchestrator = &defaultConductor.orchestrator;
LongPoll* defaultPoll;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	Settings settings(HKEY_CURRENT_USER,
		_T("Software\\Quicklet\\DevClient\\1.0"));

	settings.Load(); // Load configuration

	defaultConductor.orchestrator.qbInfo.authToken = settings.ClientKey;
	URLS::GOLIATH_SERVER = settings.URL;

	if (!defaultConductor.orchestrator.qbInfo.TestQBWorks()) {
		return 0;
	}

	defaultConductor.orchestrator.qbInfo.GetInfoFromQB();

	CMainDlg dlgMain;

	if(dlgMain.Create(NULL) == NULL)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	dlgMain.ShowWindow(nCmdShow);



	SetEvent(defaultConductor.orchestrator.qbInfo.readyForLongPollSignal);
	
	defaultConductor.orchestrator.mainThreadID = GetCurrentThreadId();
	defaultConductor.orchestrator.cMainDlg = &dlgMain;
	defaultConductor.orchestrator.StartConcert();

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}

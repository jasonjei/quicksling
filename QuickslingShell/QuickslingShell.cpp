// QuickslingShell.cpp : main source file for QuickslingShell.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include "resource.h"
#include "CQBSDKCallback.h"

// Note: Proxy/Stub Information
//		To build a separate proxy/stub DLL, 
//		run nmake -f QuickslingShellps.mk in the project directory.
#include "initguid.h"
#include "QuickslingShell.h"
#include "QuickslingShell_i.c"

#include "aboutdlg.h"
#include "MainDlg.h"
#include "ShellUtilities.h"
#include "Conductor.h"
#include "Orchestrator.h"

#import "sdkevent.dll" no_namespace named_guids raw_interfaces_only

CServerAppModule _Module;

Conductor defaultConductor;
Orchestrator *defaultOrchestrator = &defaultConductor.orchestrator;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_QBSDKCallback, CQBSDKCallback)
END_OBJECT_MAP()

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	defaultConductor.orchestrator.mainThreadID = GetCurrentThreadId();

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainDlg dlgMain;
	defaultConductor.orchestrator.cMainDlg = &dlgMain;

	if(dlgMain.Create(NULL) == NULL)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	_Module.Lock();
	// dlgMain.ShowWindow(nCmdShow);

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

	hRes = _Module.Init(ObjectMap, hInstance);
	ATLASSERT(SUCCEEDED(hRes));
	
	// MessageBox(NULL, _T("DEBUG HOOK"), _T("DEBUG HOOK"), MB_OK);

	defaultConductor.orchestrator.hInstance = hInstance;

	// Parse command line, register or unregister or run the server
	int nRet = 0;
	TCHAR szTokens[] = _T("-/");
	bool bRun = false;
	bool bAutomation = false;

	LPCTSTR lpszToken = _Module.FindOneOf(::GetCommandLine(), szTokens);
	while(lpszToken != NULL)
	{
		if(lstrcmpi(lpszToken, _T("UnregServer")) == 0)
		{
			_Module.UpdateRegistryFromResource(IDR_QUICKSLINGSHELL, FALSE);
			nRet = _Module.UnregisterServer(TRUE);
			bRun = false;
			return nRet;
			break;
		}
		else if(lstrcmpi(lpszToken, _T("RegServer")) == 0)
		{
			_Module.UpdateRegistryFromResource(IDR_QUICKSLINGSHELL, TRUE);
			nRet = _Module.RegisterServer(TRUE);
			bRun = false;
			return nRet;
			break;
		}
		else if (lstrcmpi(lpszToken, _T("RegUIEvents")) == 0)
		{
			int success = ShellUtilities::RegisterUICallbacks();
			bRun = false;
			return (success == 1 ? 0 : 1);
			break;
		}
		else if (lstrcmpi(lpszToken, _T("UnregUIEvents")) == 0)
		{
			int success = ShellUtilities::UnregisterUICallbacks();
			bRun = false;
			return (success == 1 ? 0 : 1);
			break;
		}
		else if((lstrcmpi(lpszToken, _T("Automation")) == 0) ||
			(lstrcmpi(lpszToken, _T("Embedding")) == 0))
		{
			bRun = true;
			// bAutomation = true;
			break;
		}
		else if ((lstrcmpi(lpszToken, _T("Testing")) == 0))
		{
			bRun = true;
			// bAutomation = true;
			defaultOrchestrator->spawnCanary.StartThread();

			break;
		}
		lpszToken = _Module.FindOneOf(lpszToken, szTokens);
	}

	if (bRun)
	{
		_Module.StartMonitor();
		hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED);
		ATLASSERT(SUCCEEDED(hRes));
		hRes = ::CoResumeClassObjects();
		ATLASSERT(SUCCEEDED(hRes));

		defaultConductor.orchestrator.StartConcert();

		if (bAutomation)
		{
			CMessageLoop theLoop;
			nRet = theLoop.Run();
		}
		else
		{
			nRet = Run(lpstrCmdLine, nCmdShow);
		}

		_Module.RevokeClassObjects();
		::Sleep(_Module.m_dwPause);
	}
	else {
		MessageBox(NULL, _T("Please start QuickBooks, open a company, and authorize QuickSling to start QuickSling"), _T("QuickBooks Must Start QuickSling"), MB_OK);
	}

	_Module.Term();
	::CoUninitialize();

	return nRet;
}

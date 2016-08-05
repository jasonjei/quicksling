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
#include "spdlog\spdlog.h"
#include "BugSplat.h"
#include "exclusion.h"

CServerAppModule _Module;

Conductor defaultConductor;
Orchestrator *defaultOrchestrator = &defaultConductor.orchestrator;

bool ExceptionCallback(UINT nCode, LPVOID lpVal1, LPVOID lpVal2);
MiniDmpSender *mpSender;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_QBSDKCallback, CQBSDKCallback)
END_OBJECT_MAP()

void InitLogger() {
	CString fileName = defaultConductor.orchestrator.downloader.GetLevionUserAppDir(_T("quickslingshell_log")); 
	defaultConductor.orchestrator.downloader.CreateLevionAppDir();

	auto rotating_logger = spdlog::rotating_logger_mt("quicksling_shell", (LPCTSTR)fileName, 1048576, 3);
}

int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	auto l = spdlog::get("quicksling_shell");

	defaultConductor.orchestrator.mainThreadID = GetCurrentThreadId();

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainDlg dlgMain;
	defaultConductor.orchestrator.cMainDlg = &dlgMain;

	if(dlgMain.Create(NULL) == NULL)
	{
		l->error("Main dialog creation failed");

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
	HANDLE hMutexOneInstance = ::CreateMutex(NULL, TRUE,
		createExclusionName(_T("QuickslingShell_"), UNIQUE_TO_SESSION));

	bool AlreadyRunning;
	DWORD lastError = NULL;
	lastError = GetLastError();

	AlreadyRunning = (GetLastError() == ERROR_ALREADY_EXISTS);

	if (AlreadyRunning) {
		CString strWindowTitle = _T("QuickletCoreEventsProcessor");
		LRESULT copyDataResult;
		CWindow pOtherWnd = (HWND)FindWindow(NULL, strWindowTitle);

		CString strMsg = _T("focus");

		if (pOtherWnd) {
			COPYDATASTRUCT cpd;
			cpd.dwData = NULL;
			cpd.cbData = strMsg.GetLength() * sizeof(wchar_t) + 1;
			cpd.lpData = strMsg.GetBuffer(cpd.cbData);
			copyDataResult = pOtherWnd.SendMessage(WM_COPYDATA,
				(WPARAM) NULL,
				(LPARAM)&cpd);
			strMsg.ReleaseBuffer();
		}

		return 0;
	}

	InitLogger();

	if (!IsDebuggerPresent())
	{
		CString version = QUICKSLING_SHELL_VER;
#ifdef DEBUG
		version += "D";
#endif
		mpSender = new MiniDmpSender(L"QuickslingShell1_1", L"QuickslingShell", (LPCTSTR)version, NULL);
		mpSender->setCallback(ExceptionCallback);
	}

	CString productName;
	CString productVersion;

	defaultConductor.orchestrator.spawnCanary.GetProductAndVersion(productName, productVersion);
	{
		auto l = spdlog::get("quicksling_shell");
		if (l.get())
			l->info("Welcome to QuickslingShell (Version {}, Process ID {}, Main Thread {})", CW2A(productVersion, CP_UTF8), GetCurrentProcessId(), GetCurrentThreadId());
	}

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
	bool startedByQb = false;

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
			{
				auto l = spdlog::get("quicksling_shell");
				if (l.get())
					l->info("Registering UI Events");
			}
			int success = ShellUtilities::RegisterUICallbacks();

			if (success != 1)
			{
				auto l = spdlog::get("quicksling_shell");
				if (l.get())
					l->error("Couldn't register callback");
			}

			bRun = false;
			return (success == 1 ? 0 : 1);
			break;
		}
		else if (lstrcmpi(lpszToken, _T("UnregUIEvents")) == 0)
		{
			{
				auto l = spdlog::get("quicksling_shell");
				if (l.get())
					l->info("Unregistering UI Events");
			}
			int success = ShellUtilities::UnregisterUICallbacks();
			bRun = false;

			if (success != 1)
			{
				auto l = spdlog::get("quicksling_shell");
				if (l.get())
					l->error("Couldn't unregister callback");
			}

			return (success == 1 ? 0 : 1);
			break;
		}
		else if((lstrcmpi(lpszToken, _T("Automation")) == 0) ||
			(lstrcmpi(lpszToken, _T("Embedding")) == 0))
		{
			auto l = spdlog::get("quicksling_shell");
			if (l.get())
				l->info("Started by COM automation/embedding");
			bRun = true;
			startedByQb = true;
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
		if (hMutexOneInstance != NULL)
		{
			::ReleaseMutex(hMutexOneInstance);
		}
		
		CloseHandle(hMutexOneInstance);

		{
			auto l = spdlog::get("quicksling_shell");
			if (l.get())
				l->alert("Not started by QuickBooks");
		}
		spdlog::drop_all();
		MessageBox(NULL, _T("Please start QuickBooks, open a company, and authorize QuickSling to start QuickSling"), _T("QuickBooks Must Start QuickSling"), MB_OK);
		return 0;
	}

	{
		auto l = spdlog::get("quicksling_shell");
		if (l.get())
			l->info("Bye Bye!!! See you next time! (Process ID {})", GetCurrentProcessId());
	}

	_Module.Term();
	::CoUninitialize();

	if (hMutexOneInstance != NULL)
	{
		::ReleaseMutex(hMutexOneInstance);
	}

	return nRet;
}

bool ExceptionCallback(UINT nCode, LPVOID lpVal1, LPVOID lpVal2)
{
	auto l = spdlog::get("quicksling_shell");
	l->flush();

	CString fileName = defaultConductor.orchestrator.downloader.GetLevionUserAppDir(_T("quickslingshell_log.txt"));

	struct _stat buffer;
	int success = _wstat((LPCTSTR)fileName, &buffer);

	if (success != -1) {
		mpSender->sendAdditionalFile(fileName);
	}

	spdlog::drop_all();

	return false;
}
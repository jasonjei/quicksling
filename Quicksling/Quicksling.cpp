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

#include "simple_app.h"
#include <mutex>

#include "spdlog\spdlog.h"
#include "BugSplat.h"

CAppModule _Module;

Conductor defaultConductor;
Orchestrator *defaultOrchestrator = &defaultConductor.orchestrator;
LongPoll* defaultPoll;

bool ExceptionCallback(UINT nCode, LPVOID lpVal1, LPVOID lpVal2);
MiniDmpSender *mpSender;

std::mutex mutexDataEvents;
std::mutex mutexQBInfo;

void InitLogger() {
	CString fileName = defaultConductor.orchestrator.qbInfo.GetLevionUserAppDir(_T("quicksling_log"));
	defaultConductor.orchestrator.qbInfo.CreateLevionAppDir();

	auto rotating_logger = spdlog::rotating_logger_mt("quicksling", (LPCTSTR)fileName, 1048576, 3);
}

class QuickslingMessageFilter : public CMessageFilter {
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg) {
		CefDoMessageLoopWork();
		return 0;
	}
};


int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	InitLogger();

	{
		auto l = spdlog::get("quicksling");
		l->info("Welcome to QuickslingShell (Version {}, Process ID {}, Main Thread {})", std::string(CW2A(defaultConductor.orchestrator.qbInfo.version, CP_UTF8)), GetCurrentProcessId(), GetCurrentThreadId());
	}

	CMessageLoop theLoop;
	QuickslingMessageFilter quickslingMsgFilter;

	_Module.AddMessageLoop(&theLoop);
	/* TO DELETE
	Settings settings(HKEY_CURRENT_USER,
		_T("Software\\Quicklet\\DevClient\\1.0"));

	settings.Load(); // Load configuration
	*/

	// defaultConductor.orchestrator.qbInfo.authToken = settings.ClientKey;
	// URLS::GOLIATH_SERVER = settings.URL;

	defaultConductor.orchestrator.qbInfo.GetInfoFromQB();

	CMainDlg dlgMain;

	if(dlgMain.Create(NULL) == NULL)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	// dlgMain.ShowWindow(nCmdShow);
	
	defaultConductor.orchestrator.mainThreadID = GetCurrentThreadId();
	defaultConductor.orchestrator.cMainDlg = &dlgMain;
	defaultConductor.orchestrator.StartConcert();

	theLoop.AddMessageFilter(&quickslingMsgFilter);

	int nRet = theLoop.Run();

	CefDoMessageLoopWork();
	CefDoMessageLoopWork();

	CefShutdown();
	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	if (!IsDebuggerPresent())
	{
		CString version = QUICKSLING_VER;
#ifdef DEBUG
		version += "D";
#endif
		mpSender = new MiniDmpSender(BUGSPLAT_DB, BUGSPLAT_APP, version, NULL);
		mpSender->setCallback(ExceptionCallback);
	}

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

	// Enable High-DPI support on Windows 7 or newer.
	CefEnableHighDPISupport();

	// Provide CEF with command-line arguments.
	defaultOrchestrator->browser.hInstance = hInstance;
	defaultOrchestrator->browser.main_args = CefMainArgs(hInstance);
	defaultOrchestrator->browser.app = new SimpleApp;
	defaultOrchestrator->browser.simpleHandler = new SimpleHandler();

	int exit_code = CefExecuteProcess(defaultOrchestrator->browser.main_args, defaultOrchestrator->browser.app.get(), NULL);
	if (exit_code >= 0) {
		// MessageBox(NULL, _T("Got exit code"), _T("Got exit code"), MB_OK);
		// TODO - handle. The sub-process terminated
	}

	if (CString(lpstrCmdLine).Find(_T("--type=")) != -1) {
		defaultConductor.orchestrator.browser.settings.multi_threaded_message_loop = true;
		CefInitialize(defaultOrchestrator->browser.main_args, defaultOrchestrator->browser.settings, defaultOrchestrator->browser.app.get(), NULL);

		// Run the CEF message loop. This will block until CefQuitMessageLoop() is
		// called.
		CefRunMessageLoop();

		// Shut down CEF.
		CefShutdown();
	}
	else {
		if (! IsDebuggerPresent()) {
			HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

			PROCESSENTRY32 selfEntry;
			PROCESSENTRY32 parentEntry;

			{
				PROCESSENTRY32 entry;
				entry.dwSize = sizeof(PROCESSENTRY32);

				if (Process32First(snapshot, &entry) == TRUE) {
					while (Process32Next(snapshot, &entry) == TRUE) {
						CString procEntry(entry.szExeFile);

						if (entry.th32ProcessID == GetCurrentProcessId()) {
							selfEntry = entry;
						}
					}
				}
			}

			{
				PROCESSENTRY32 entry;
				entry.dwSize = sizeof(PROCESSENTRY32);

				if (Process32First(snapshot, &entry) == TRUE) {
					while (Process32Next(snapshot, &entry) == TRUE) {
						CString procEntry(entry.szExeFile);

						if (entry.th32ProcessID == selfEntry.th32ParentProcessID) {
							parentEntry = entry;
						}
					}
				}

			}

			CString parentExe = parentEntry.szExeFile;
			CloseHandle(snapshot);

			parentExe.MakeLower();
			parentExe.TrimLeft();
			parentExe.TrimRight();

			if (parentExe != "quickslingshell.exe") {
				MessageBox(NULL, _T("QuickSling must be started by QuickBooks"), _T("Start QuickSling through QuickBooks"), MB_OK | MB_SYSTEMMODAL);
				return 0;
			}
		}

		TCHAR szTokens[] = _T("-/");
		LPCTSTR lpszToken = _Module.FindOneOf(::GetCommandLine(), szTokens);
		CString* currentToken = new CString(lpszToken);

		while (!currentToken->IsEmpty())
		{
			CString nextToken;
			int nextTokenPos = currentToken->Find(_T(" -"));
			if (nextTokenPos != -1) {
				nextToken = currentToken->Right(currentToken->GetLength() - nextTokenPos - 2);
				*currentToken = currentToken->Left(nextTokenPos);
			}

			if (currentToken->Find(_T("ShellPID")) != -1)
			{
				defaultOrchestrator->shellPID = currentToken->Right(currentToken->GetLength() - 9);
			}
			else if (currentToken->Find(_T("ShellVersion")) != -1) {
				defaultOrchestrator->shellVersion = currentToken->Right(currentToken->GetLength() - 13);
			}
			else if (currentToken->Find(_T("DoNotAskToSend")) != -1)
			{
				defaultConductor.askToSendCrashRpt = false;
				// InitCrashReport();
			}

			*currentToken = nextToken;
		}
		delete currentToken;

		defaultConductor.orchestrator.browser.settings.multi_threaded_message_loop = false;
		CefInitialize(defaultOrchestrator->browser.main_args, defaultOrchestrator->browser.settings, defaultOrchestrator->browser.app.get(), NULL);
		// CefShutdown();
	}

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}

bool ExceptionCallback(UINT nCode, LPVOID lpVal1, LPVOID lpVal2)
{
	auto l = spdlog::get("quicksling");
	l->flush();

	CString fileName = defaultConductor.orchestrator.qbInfo.GetLevionUserAppDir(_T("quicksling_log.txt"));

	struct _stat buffer;
	int success = _wstat((LPCTSTR)fileName, &buffer);

	if (success != -1) {
		mpSender->sendAdditionalFile(fileName);
	}

	return false;
}


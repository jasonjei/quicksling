#pragma once

#include "stdafx.h"
#include "SpawnCanary.h"
#include "Conductor.h"
#include <string>
#include "BugSplat.h"

extern Conductor defaultConductor;

extern bool ExceptionCallback(UINT nCode, LPVOID lpVal1, LPVOID lpVal2);
extern MiniDmpSender *mpSender;

SpawnCanary::SpawnCanary(void) {
}

SpawnCanary::~SpawnCanary(void) {
	threadID = NULL;
}

DWORD SpawnCanary::StartThread() {
	if (threadID == NULL) {
		threadHandle = ::CreateThread(NULL, 0, SpawnCanary::RunThread, NULL, NULL, &threadID);
	}
	return threadID;
}

int SpawnCanary::SpawnIt() {
	auto l = spdlog::get("quicksling_shell");

	int result = defaultConductor.orchestrator.spawnCanary.GetClientSettings();
	if (result == 0) {
		l->error("Couldn't start Quicksling because version is disabled remotely.");
		return 0;
	}
	defaultConductor.orchestrator.downloader.DoDownload();

	if (defaultConductor.orchestrator.spawnCanary.StartBrainProcess() == false) {
		l->error("Couldn't start Quicksling Core");
		MessageBox(NULL, _T("Quicksling Core couldn't start! Please check for updates or reinstall Quicksling."), _T("Error starting QuickSling Core"), MB_OK | MB_SYSTEMMODAL);
		defaultConductor.orchestrator.StopConcert();
		return 0;
	}

	WaitForSingleObject(defaultConductor.orchestrator.spawnCanary.brainProcessInfo.hProcess, INFINITE);

	CloseHandle(defaultConductor.orchestrator.spawnCanary.brainProcessInfo.hProcess);
	CloseHandle(defaultConductor.orchestrator.spawnCanary.brainProcessInfo.hThread);
	return 1;
}

DWORD WINAPI SpawnCanary::RunThread(LPVOID lpData) {
	auto l = spdlog::get("quicksling_shell");

	if (defaultConductor.orchestrator.spawnCanary.SpawnIt() == 0) {
		return 0;
	}
	
	while (WaitForSingleObject(defaultConductor.orchestrator.goOfflineSignal, 0) != 0) {

		if (defaultConductor.updateRequested == true) {
			l->info("Quicksling Core requested client update");

			defaultConductor.orchestrator.downloader.DoDownload();
			defaultConductor.updateRequested = false; //
			
			//defaultConductor.orchestrator.spawnCanary.threadID = NULL;
			//return defaultConductor.orchestrator.spawnCanary.StartThread();
			
			if (defaultConductor.orchestrator.spawnCanary.SpawnIt() == 0) {
				return 0;
			}
		}
		else {
			l->warn("Quicksling Core shut down abruptly without warning");
			int res = MessageBox(NULL, _T("Quicksling seems to have shut down incorrectly. Would you like it to restart?"), _T("Quicksling Core abruptly exited"), MB_YESNO | MB_SYSTEMMODAL);
			if (res == IDYES) {
				//defaultConductor.orchestrator.spawnCanary.threadID = NULL;
				//CloseHandle(defaultConductor.orchestrator.spawnCanary.threadHandle);
				//return defaultConductor.orchestrator.spawnCanary.StartThread();
				if (defaultConductor.orchestrator.spawnCanary.SpawnIt() == 0) {
					return 0;
				}
			}
			else {
				defaultConductor.orchestrator.StopConcert();
			}
		}
	}

	defaultConductor.orchestrator.spawnCanary.threadID = NULL;
	return 0;
}

bool SpawnCanary::GetProductAndVersion(CString & strProductName, CString & strProductVersion) {
	// get the filename of the executable containing the version resource
	TCHAR szFilename[MAX_PATH + 1] = { 0 };
	if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0)
	{
		// TRACE("GetModuleFileName failed with error %d\n", GetLastError());
		return false;
	}

	// allocate a block of memory for the version info
	DWORD dummy;
	DWORD dwSize = GetFileVersionInfoSize(szFilename, &dummy);
	if (dwSize == 0)
	{
		// TRACE("GetFileVersionInfoSize failed with error %d\n", GetLastError());
		return false;
	}
	std::vector<BYTE> data(dwSize);

	// load the version info
	if (!GetFileVersionInfo(szFilename, NULL, dwSize, &data[0]))
	{
		// TRACE("GetFileVersionInfo failed with error %d\n", GetLastError());
		return false;
	}

	// get the name and version strings
	LPVOID pvProductName = NULL;
	unsigned int iProductNameLen = 0;
	LPVOID pvProductVersion = NULL;
	unsigned int iProductVersionLen = 0;

	// replace "040904e4" with the language ID of your resources
	UINT                uiVerLen = 0;
	VS_FIXEDFILEINFO*   pFixedInfo = 0;     // pointer to fixed file info structure
	// get the fixed file info (language-independend) 
	if (VerQueryValue(&data[0], TEXT("\\"), (void**)&pFixedInfo, (UINT *)&uiVerLen) == 0)
	{
		return false;
	}

	strProductVersion.Format(_T("%u.%u.%u.%u"),
		HIWORD(pFixedInfo->dwProductVersionMS),
		LOWORD(pFixedInfo->dwProductVersionMS),
		HIWORD(pFixedInfo->dwProductVersionLS),
		LOWORD(pFixedInfo->dwProductVersionLS));

	// strProductName.SetString((LPCSTR)pvProductName, iProductNameLen);
	// strProductVersion.SetString((LPCSTR)pvProductVersion, iProductVersionLen);

	return true;
}

BOOL SpawnCanary::StartBrainProcess() {
	CString app_path = _T("\"") + defaultConductor.orchestrator.info.GetQuickslingUserAppDir() + _T("\\Quicksling.exe\"");

	CString productName;
	CString productVersion;

	GetProductAndVersion(productName, productVersion);

#ifdef DEBUG
	productVersion += _T("D");
#endif

	auto l = spdlog::get("quicksling_shell");
	l->info("Starting core at {}", CW2A(app_path, CP_UTF8));

	STARTUPINFO si;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &brainProcessInfo, sizeof(brainProcessInfo) );

	DWORD pid = GetCurrentProcessId();
	std::ostringstream stream;
	stream << pid;
	app_path = app_path + _T(" -ShellPID ") + stream.str().c_str();
	app_path += _T(" -ShellVersion ") + productVersion;
	if (defaultConductor.askToSendCrashRpt == false) {
		app_path += _T(" -DoNotAskToSend");
	}

	BOOL successful = CreateProcess(NULL, app_path.GetBuffer(0), NULL, NULL, TRUE, NULL, NULL, NULL, &si, &brainProcessInfo);

	if (successful == 1) {

		CloseHandle(si.hStdError);
		CloseHandle(si.hStdInput);
		CloseHandle(si.hStdOutput);

		if (defaultConductor.orchestrator.ghJob)
		{
			if (0 == AssignProcessToJobObject(defaultConductor.orchestrator.ghJob, brainProcessInfo.hProcess))
			{
				l->info("Couldn't assign process to job object");
				::MessageBox(0, _T("Could not AssignProcessToObject"), _T("Crap"), MB_OK | MB_SYSTEMMODAL);
			}
		}
		// CString *openString = new CString(defaultConductor.orchestrator.eventHandler.qbOpenEvent);
		// ::PostThreadMessage(defaultConductor.orchestrator.pipeWrite.threadID, PIPE_REQUEST, (WPARAM)openString, NULL);
	}
	l->info("Core started");
	return successful;
}

void SpawnCanary::StopBrainProcess() {
	if (brainProcessInfo.dwThreadId != 0) {
		// Allow brain to shutdown cleanly
		SetEvent(defaultConductor.orchestrator.goOfflineSignal);

		// CString *newString = new CString("shutdown");
		// ::PostThreadMessage(defaultConductor.orchestrator.pipeWrite.threadID, PIPE_REQUEST, (WPARAM)newString, NULL);

		CString strWindowTitle = _T("QuickletCoreEventsProcessor");
		CString strDataToSend = _T("shutdown");

		LRESULT copyDataResult;

		CWindow pOtherWnd = (HWND)FindWindow(NULL, strWindowTitle);

		if (pOtherWnd) {
			COPYDATASTRUCT cpd;
			cpd.dwData = NULL;
			cpd.cbData = strDataToSend.GetLength() * sizeof(wchar_t) + 1;
			cpd.lpData = strDataToSend.GetBuffer(cpd.cbData);
			copyDataResult = pOtherWnd.SendMessage(WM_COPYDATA,
				(WPARAM) defaultConductor.orchestrator.cMainDlg->m_hWnd,
				(LPARAM)&cpd);
			strDataToSend.ReleaseBuffer();
			// copyDataResult has value returned by other app
		}
		else
		{
			// MessageBox(_T("Can't find other app!"), _T("UH OH"), MB_OK);
			// AfxMessageBox("Unable to find other app.");
		}

		WaitForSingleObject(defaultConductor.orchestrator.spawnCanary.threadHandle, INFINITE);
	}
}

int SpawnCanary::GetClientSettings() {
	CIniFile configIni;

	CString version = QUICKSLING_SHELL_VER;
#ifdef DEBUG
	version += "D";
#endif
	CString sURL = URLS::APP_SERVER + "client/settings?client=QuickslingShell&version=" + version;

	CInternetSession Session(_T("Quicksling Downloader"));
	WORD timeout = 10000;
	DeleteUrlCacheEntry(sURL);

	Session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, timeout);
	Session.SetOption(INTERNET_OPTION_SEND_TIMEOUT, timeout);

	CHttpFile* cHttpFile = NULL;
	int fail = 0;


	try {
		cHttpFile = new CHttpFile(Session, sURL, NULL, 0, INTERNET_FLAG_DONT_CACHE);
	}

	catch (CInternetException& e) {
		fail = 1;
		delete cHttpFile;
	}

	if (!fail) {
		WTL::CString pageSource;
		CIniFile configIni;

		CIniSectionW* settingsSec;

		UINT bytes = (UINT)cHttpFile->GetLength();

		char tChars[2048 + 1];
		int bytesRead;

		while ((bytesRead = cHttpFile->Read((LPVOID)tChars, 2048)) != 0) {
			tChars[bytesRead] = '\0';
			pageSource += CA2W((LPCSTR)tChars, CP_UTF8);
		}

		delete cHttpFile;

		std::wistringstream downloadManifestStream((LPCTSTR)pageSource);
		configIni.Load(downloadManifestStream, false);
		settingsSec = configIni.GetSection(_T("settings"));

		if (settingsSec != NULL) {
			CIniKeyW* databaseKey = settingsSec->GetKey(_T("database"));

			CString version = QUICKSLING_SHELL_VER;
#ifdef DEBUG
			version += "D";
#endif

			CString databaseVal = BUGSPLAT_DB, appVal = BUGSPLAT_APP, versionVal = version, disableMsgVal = QUICKSLING_DISABLE_MESSAGE;
			bool noninteractive = false, disable = false;

			if (databaseKey != NULL) {
				databaseVal = databaseKey->GetValue().c_str();
				databaseVal.TrimLeft();
				databaseVal.TrimRight();

				if (databaseVal.IsEmpty()) {
					databaseVal = BUGSPLAT_DB;
				}
			}

			CIniKeyW* appKey = settingsSec->GetKey(_T("app"));
			if (appKey != NULL) {
				appVal = appKey->GetValue().c_str();
				appVal.TrimLeft();
				appVal.TrimRight();

				if (appVal.IsEmpty()) {
					appVal = BUGSPLAT_APP;
				}
			}

			CIniKeyW* versionKey = settingsSec->GetKey(_T("version"));
			if (versionKey != NULL) {
				versionVal = versionKey->GetValue().c_str();
				versionVal.TrimLeft();
				versionVal.TrimRight();

				if (versionVal.IsEmpty()) {
					versionVal = version;
				}
			}

			CIniKeyW* noninteractiveKey = settingsSec->GetKey(_T("noninteractive"));
			if (noninteractiveKey != NULL) {
				CString noninteractiveVal = noninteractiveKey->GetValue().c_str();
				noninteractiveVal.TrimLeft();
				noninteractiveVal.TrimRight();
				noninteractiveVal.MakeLower();

				if (noninteractiveVal == "true" || noninteractiveVal == "1") {
					noninteractive = true;
				}
			}

			CIniKeyW* disableKey = settingsSec->GetKey(_T("disable"));
			if (disableKey != NULL) {
				CString disableVal = disableKey->GetValue().c_str();
				disableVal.TrimLeft();
				disableVal.TrimRight();
				disableVal.MakeLower();

				if (disableVal == "true" || disableVal == "1") {
					disable = true;
				}
			}

			CIniKeyW* disableMsgKey = settingsSec->GetKey(_T("disable_message"));
			if (disableMsgKey != NULL) {
				disableMsgVal = disableMsgKey->GetValue().c_str();
				disableMsgVal.TrimLeft();
				disableMsgVal.TrimRight();

				if (disableMsgVal.IsEmpty()) {
					disableMsgVal = QUICKSLING_DISABLE_MESSAGE;
				}
			}

			if (disable == true) {
				MessageBox(NULL, disableMsgVal, _T("This version of QuickSling is disabled"), MB_OK | MB_SYSTEMMODAL);

				SetEvent(defaultConductor.orchestrator.goOfflineSignal);
				PostMessage(defaultConductor.orchestrator.cMainDlg->m_hWnd, WM_CLOSE, NULL, NULL);
				return 0;
			}

			if (!IsDebuggerPresent())
			{
				delete mpSender;
				mpSender = new MiniDmpSender(databaseVal, appVal, versionVal, NULL);
				mpSender->setCallback(ExceptionCallback);

				if (noninteractive == true) {
					mpSender->setFlags(MDSF_NONINTERACTIVE | mpSender->getFlags());
				}
			}

			return 1;
		}
	}
}
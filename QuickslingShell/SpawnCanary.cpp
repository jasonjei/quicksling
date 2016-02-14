#pragma once

#include "stdafx.h"
#include "SpawnCanary.h"
#include "Conductor.h"
#include <string>
#include <memory>
#include "unzip.h"
#include "zip.h"
#include "DownloadCallback.h"
#include "LevionMisc.h"

extern Conductor defaultConductor;

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

DWORD WINAPI SpawnCanary::RunThread(LPVOID lpData) {
	defaultConductor.orchestrator.spawnCanary.CheckForUpdates();

	if (defaultConductor.orchestrator.spawnCanary.StartBrainProcess() == false) {
		MessageBox(NULL, _T("Quicksling Core couldn't start! Please check for updates or reinstall Quicksling."), _T("Error starting QuickSling Core"), MB_OK);
		defaultConductor.orchestrator.StopConcert();
		return 0;
	}

	WaitForSingleObject(defaultConductor.orchestrator.spawnCanary.brainProcessInfo.hProcess, INFINITE);

	CloseHandle(defaultConductor.orchestrator.spawnCanary.brainProcessInfo.hProcess);
	CloseHandle(defaultConductor.orchestrator.spawnCanary.brainProcessInfo.hThread);

	if (WaitForSingleObject(defaultConductor.orchestrator.goOfflineSignal, 0) != 0) {
		int res = MessageBox(NULL, _T("Quicksling seems to have shut down incorrectly. Would you like it to restart?"), _T("Quicksling Core abruptly exited"), MB_YESNO);
		if (res == IDYES) {
			defaultConductor.orchestrator.spawnCanary.threadID = NULL;
			return defaultConductor.orchestrator.spawnCanary.StartThread();
		} else {
			defaultConductor.orchestrator.StopConcert();
		}
	}

	defaultConductor.orchestrator.spawnCanary.threadID = NULL;
	return 0;
}

BOOL SpawnCanary::StartBrainProcess() {
	CString app_path = _T("\"") + defaultConductor.orchestrator.info.GetQuickslingUserAppDir() + _T("\\Quicksling.exe\"");

	STARTUPINFO si;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &brainProcessInfo, sizeof(brainProcessInfo) );

	DWORD pid = GetCurrentProcessId();
	std::ostringstream stream;
	stream << pid;
	app_path = app_path + _T(" -ShellPID ") + stream.str().c_str();
	if (defaultConductor.askToSendCrashRpt == false) {
		app_path += _T(" -DoNotAskToSend");
	}

	BOOL successful = CreateProcess(NULL, app_path.GetBuffer(0), NULL, NULL, TRUE, NULL, NULL, NULL, &si, &brainProcessInfo);

	if (successful == 1) {
		if (defaultConductor.orchestrator.ghJob)
		{
			if (0 == AssignProcessToJobObject(defaultConductor.orchestrator.ghJob, brainProcessInfo.hProcess))
			{
				::MessageBox(0, _T("Could not AssignProcessToObject"), _T("Crap"), MB_OK);
			}
		}
		// CString *openString = new CString(defaultConductor.orchestrator.eventHandler.qbOpenEvent);
		// ::PostThreadMessage(defaultConductor.orchestrator.pipeWrite.threadID, PIPE_REQUEST, (WPARAM)openString, NULL);
	}
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

		// WaitForSingleObject(defaultConductor.orchestrator.spawnCanary.threadHandle, INFINITE);
	}
}


void DownloadFilesWithProgress(HWND dialogWindow, UpdatesInfo* updatesInfo)
{
	try {
		// Tell the download callback how many bytes we need to download, pointing it to progress dialog
		DownloadCallback pCallback(dialogWindow, updatesInfo->totalBytes);

		for (std::wstring fileName : updatesInfo->filesToDownload) {
			std::wstring serverFilePath(updatesInfo->serverCRCIni.GetKeyValue(fileName, std::wstring(_T("path"))));
			CString localFilePath = defaultConductor.orchestrator.info.GetQuickslingUserAppDir() + _T("\\") + CString(fileName.c_str());

			if (updatesInfo->serverCRCIni.GetKeyValue(fileName, std::wstring(_T("decompress"))) == std::wstring(_T("true")))
				localFilePath += _T(".zip");

			// fix for MS retardedness
			DeleteUrlCacheEntry(URLS::DOWNLOAD_SERVER + serverFilePath.c_str());

			// Delete the old version of the file only if the download of the new version succeeds
			MoveFile(localFilePath, localFilePath + _T("OLD"));
			HRESULT result = URLDownloadToFile(NULL, URLS::DOWNLOAD_SERVER + serverFilePath.c_str(), localFilePath, 0, &pCallback);
			if (result != S_OK) {
				MoveFile(localFilePath + _T("OLD"), localFilePath);
				continue; // TODO - pop up notification?
			}
			else {
				DeleteFile(localFilePath + _T("OLD"));
			}

			// Decompress the file if necessary and delete zip
			if (updatesInfo->serverCRCIni.GetKeyValue(fileName, std::wstring(_T("decompress"))) == std::wstring(_T("true"))) {
				HZIP hz = OpenZip(localFilePath, 0);
				SetUnzipBaseDir(hz, defaultConductor.orchestrator.info.GetQuickslingUserAppDir());
				ZIPENTRY ze;
				GetZipItem(hz, -1, &ze);
				int numitems = ze.index;

				for (int i = 0; i < numitems; i++)
				{
					GetZipItem(hz, i, &ze);
					UnzipItem(hz, i, ze.name);
				}
				CloseZip(hz);

				DeleteFile(localFilePath);
			}
		}
		delete updatesInfo; //TODO - figure out how to get shared_ptr working with lParam
	}
	catch (...) {
		delete updatesInfo; //TODO - confirm ptr is destroyed
	}
}

BOOL CALLBACK DownloadDialogProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	if (msg==WM_INITDIALOG) {
		SetWindowText(hwnd,_T("Downloading updates..."));
		PostMessage(hwnd,WM_USER,0,lParam);
		return TRUE;
	}
	if (msg==WM_COMMAND) {return TRUE;}
	if (msg==WM_USER) {
		DownloadFilesWithProgress(GetDlgItem(hwnd,1), (UpdatesInfo*) lParam);
		EndDialog(hwnd,IDOK);
		return TRUE;
	}
	return FALSE;
}

int SpawnCanary::CheckForUpdates() {
	UpdatesInfo* updatesInfo = new UpdatesInfo;
	updatesInfo->totalBytes = 0;

	// Create an ini map of the current CRC's for critical files retrieved from the download server.
	CString serverCRCIniPath = defaultConductor.orchestrator.info.GetQuickslingUserAppDir() + _T("\\") + _T("file_versions.ini");

	HRESULT result = URLDownloadToFile(NULL, URLS::APP_SERVER + _T("file_versions.ini"), serverCRCIniPath, 0, NULL);

	updatesInfo->serverCRCIni.Load(std::wstring(serverCRCIniPath));
	for (SecIndex::const_iterator itr = updatesInfo->serverCRCIni.GetSections().begin(); itr != updatesInfo->serverCRCIni.GetSections().end(); ++itr) {
		CString localFilePath = defaultConductor.orchestrator.info.GetQuickslingUserAppDir() + _T("\\") + (*itr)->GetSectionName().c_str();
		DWORD dwCrc32;
		DWORD dwErrorCode = CCrc32Static::FileCrc32Assembly(localFilePath, dwCrc32);

		std::wstring currentCRC = std::to_wstring(dwCrc32);
		std::wstring updatedCRC = (*itr)->GetKeyValue(std::wstring(_T("CRC")));
		
		if (currentCRC != updatedCRC) {
			updatesInfo->filesToDownload.push_back((*itr)->GetSectionName());
			updatesInfo->totalBytes += std::stoi((*itr)->GetKeyValue(std::wstring(_T("size"))));
		}
	}

	if (updatesInfo->filesToDownload.empty() == false) {
		StopBrainProcess();

		// Create download progress dialog
		HINSTANCE hInstance=GetModuleHandle(NULL);
		InitCommonControls();

#pragma pack(push,1)
		struct TDlgItemTemplate {DWORD s,ex; short x,y,cx,cy; WORD id;};
		struct TDlgTemplate {DWORD s,ex; WORD cdit; short x,y,cx,cy;};
		struct TDlgItem1 {TDlgItemTemplate dli; WCHAR cls[7],tit[7]; WORD cdat;};
		struct TDlgItem2 {TDlgItemTemplate dli; WCHAR cls[18],tit[1]; WORD cdat;};
		struct TDlgData  {TDlgTemplate dlt; WORD m,c; WCHAR t[8]; WORD pt; WCHAR f[14]; TDlgItem1 i1; TDlgItem2 i2;};
		TDlgData dtp={{DS_MODALFRAME|DS_3DLOOK|DS_SETFONT|DS_CENTER|WS_POPUP|WS_CAPTION|WS_SYSMENU|WS_VISIBLE,0,2, 0,0,278,54},
			0,0,L"Zipping",8,L"MS Sans Serif",
		{{BS_PUSHBUTTON|WS_CHILD|WS_VISIBLE,0,113,32,50,14,IDCANCEL},L"BUTTON",L"Cancel",0},
		{{WS_CHILD|WS_VISIBLE,0,7,7,264,18,1},L"msctls_progress32",L"",0} };
#pragma pack(pop)

		DialogBoxIndirectParam(hInstance, (DLGTEMPLATE*) &dtp, 0, DownloadDialogProc, (LPARAM) updatesInfo); //TODO

		return 1;
	} else {
		return 0;
	}
}

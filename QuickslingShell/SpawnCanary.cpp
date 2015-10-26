#pragma once

#include "stdafx.h"
#include "SpawnCanary.h"
#include "Conductor.h"
#include <string>

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
	if (defaultConductor.orchestrator.spawnCanary.StartBrainProcess() == false) {
		MessageBox(NULL, _T("Levion Connector failed to start! Please check for updates or reinstall Levion."), _T("Error!"), MB_OK);
		defaultConductor.orchestrator.StopConcert();
		return 0;
	}

	WaitForSingleObject(defaultConductor.orchestrator.spawnCanary.brainProcessInfo.hProcess, INFINITE);

	if (WaitForSingleObject(defaultConductor.orchestrator.goOfflineSignal, 0) != 0) {
		int res = MessageBox(NULL, _T("Levion Connector seems to have shut down incorrectly. Would you like it to restart?"), _T("Error."), MB_YESNO);
		if (res == IDYES) {
			defaultConductor.orchestrator.spawnCanary.RunThread(NULL);
		} else {
			defaultConductor.orchestrator.StopConcert();
		}
	}

	defaultConductor.orchestrator.spawnCanary.threadID = NULL;
	return 0;
}

BOOL SpawnCanary::StartBrainProcess() {
	CString app_path = _T("\"") + defaultConductor.orchestrator.info.GetQuickslingUserAppDir() + _T("\\SimpleQuickletClient.exe\"");

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

		// CloseHandle(brainProcessInfo.hProcess);
		// CloseHandle(brainProcessInfo.hThread);
	}
}

/*
void DownloadFilesWithProgress(HWND dialogWindow, UpdatesInfo* updatesInfo)
{
	// Tell the download callback how many bytes we need to download, pointing it to progress dialog
	DownloadCallback pCallback(dialogWindow, updatesInfo->totalBytes);

	for (auto &fileName : updatesInfo->filesToDownload) {
		CString serverFilePath(updatesInfo->serverCRCMap.mapPtr->map[fileName].mapPtr->map["path"].value.c_str());
		CString localFilePath = defaultConductor.orchestrator.qbInfo.GetLevionUserAppDir() + _T("\\") + CString(fileName.c_str());

		DeleteFile(localFilePath);

		if (updatesInfo->serverCRCMap.mapPtr->map[fileName].mapPtr->map["decompress"].value == "true")
			localFilePath += _T(".zip");

		// fix for MS retardedness
		DeleteUrlCacheEntry(URLS::DOWNLOAD_SERVER + serverFilePath);

		HRESULT result = URLDownloadToFile(NULL, URLS::DOWNLOAD_SERVER + serverFilePath, localFilePath, 0, &pCallback);
		// TODO - If result != S_OK, then panic

		// Decompress the file if necessary and delete zip
		if (updatesInfo->serverCRCMap.mapPtr->map[fileName].mapPtr->map["decompress"].value == "true") {
			HZIP hz = OpenZip(localFilePath,0);
			SetUnzipBaseDir(hz, defaultConductor.orchestrator.qbInfo.GetLevionUserAppDir());
			ZIPENTRY ze;
			GetZipItem(hz,-1,&ze);
			int numitems=ze.index;

			for (int i=0; i<numitems; i++)
			{
				GetZipItem(hz,i,&ze);
				UnzipItem(hz,i,ze.name);
			}
			CloseZip(hz);

			DeleteFile(localFilePath);
		}
	}

	delete updatesInfo;
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

	// Create a yaml map of the current CRC's for critical files retrieved from the download server.
	updatesInfo->serverCRCMap = MapObject::processYaml(std::string(CW2A(VisitURL(URLS::DOWNLOAD_SERVER + _T("file_versions.yml")), CP_UTF8)));
	for (auto &fileName : updatesInfo->serverCRCMap.mapPtr->keys) {
		CString localFilePath = defaultConductor.orchestrator.qbInfo.GetLevionUserAppDir() + _T("\\") + CString(fileName.c_str());
		DWORD dwCrc32;
		DWORD dwErrorCode = CCrc32Static::FileCrc32Assembly(localFilePath, dwCrc32);

		std::ostringstream stream;
		stream << dwCrc32;

		std::string currentCRC= stream.str();
		std::string updatedCRC = updatesInfo->serverCRCMap.mapPtr->map[fileName].mapPtr->map["CRC"].value;

		if (currentCRC != updatedCRC) {
			updatesInfo->filesToDownload.push_back(fileName);
			updatesInfo->totalBytes += atoi(updatesInfo->serverCRCMap.mapPtr->map[fileName].mapPtr->map["size"].value.c_str());
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

		DialogBoxIndirectParam(hInstance, (DLGTEMPLATE*) &dtp, 0, DownloadDialogProc, (LPARAM) updatesInfo);

		return 1;
	} else
		return 0;
}

int SpawnCanary::DownloadFile(MapObject serverCRCMap, std::string fileName) {
	CString localFilePath = orchestrator->qbInfo.GetLevionUserAppDir() + _T("\\") + CString(fileName.c_str());
	CString serverFilePath(serverCRCMap.mapPtr->map[fileName].mapPtr->map["path"].value.c_str());

	HRESULT result = URLDownloadToFile(NULL, URLS::DOWNLOAD_SERVER + serverFilePath, localFilePath, 0, NULL);
	if (result != S_OK)
		return 0;

	// Decompress the file if necessary and delete zip
	if (serverCRCMap.mapPtr->map[fileName].mapPtr->map["decompress"].value == "true") {
		HZIP hz = OpenZip(localFilePath,0);
		SetUnzipBaseDir(hz, orchestrator->qbInfo.GetLevionUserAppDir());
		ZIPENTRY ze;
		GetZipItem(hz,-1,&ze);
		int numitems=ze.index;

		for (int i=0; i<numitems; i++)
		{
			GetZipItem(hz,i,&ze);
			UnzipItem(hz,i,ze.name);
		}
		CloseZip(hz);

		DeleteFile(localFilePath);
	}

	return 1;
}

*/
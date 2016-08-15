#pragma once
#include "stdafx.h"
#include "ShellEventsProcessor.h"
#include "Conductor.h"

extern Conductor defaultConductor;
extern Orchestrator *defaultOrchestrator;

LRESULT ShellEventsProcessor::OnCopyData(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	COPYDATASTRUCT* pCopyDataStruct = (COPYDATASTRUCT*)lParam;
	CString strRecievedText = (LPCTSTR)(pCopyDataStruct->lpData);
	// MessageBox(strRecievedText, _T("Received stuff from kkk!"), MB_OK);
	if (strRecievedText.CompareNoCase(_T("shutdown")) == 0) {
		if (defaultConductor.updateRequested == false) {
			{
				auto l = spdlog::get("quicksling_shell");
				if (l.get())
					l->info("Core requesting shutdown");
			}
			SetEvent(defaultConductor.orchestrator.goOfflineSignal);
			SendMessage(defaultConductor.orchestrator.cMainDlg->m_hWnd, WM_CLOSE, NULL, NULL);
		}
		// defaultConductor.orchestrator.StopConcert();
	}
	else if (strRecievedText.CompareNoCase(_T("update_requested")) == 0) {
		{
			auto l = spdlog::get("quicksling_shell");
			if (l.get())
				l->info("Core requesting core update");
		}
		defaultConductor.updateRequested = true;
	}
	else if (strRecievedText.CompareNoCase(_T("version")) == 0) {
		CString strWindowTitle = _T("QuickletCoreEventsProcessor");
		LRESULT copyDataResult;
		CWindow pOtherWnd = (HWND)FindWindow(NULL, strWindowTitle);

		CString productName;
		CString productVersion;

		GetProductAndVersion(productName, productVersion);

		CString strMsg = _T("version:") + productVersion;
#ifdef DEBUG
		strMsg += _T("D");
#endif
		if (pOtherWnd) {
			COPYDATASTRUCT cpd;
			cpd.dwData = NULL;
			cpd.cbData = strMsg.GetLength() * sizeof(wchar_t) + 1;
			cpd.lpData = strMsg.GetBuffer(cpd.cbData);
			copyDataResult = pOtherWnd.SendMessage(WM_COPYDATA,
				(WPARAM)defaultOrchestrator->cMainDlg->m_hWnd,
				(LPARAM)&cpd);
			strMsg.ReleaseBuffer();
			// copyDataResult has value returned by other app

		}
		else {
			// MessageBox(_T("Can't find other app!"), _T("UH OH"), MB_OK);
			// AfxMessageBox("Unable to find other app.");
		}
	}
// #ifdef DEBUG
	else if (strRecievedText.CompareNoCase(_T("crash")) == 0) {
		volatile int x, y, z;
		x = 1;
		y = 0;
		z = x / y;
	}
// #endif
	ATLTRACE2(atlTraceUI, 0, TEXT("Received stuff from QBSEP, %s\n"), strRecievedText);
	return true;
}

bool ShellEventsProcessor::GetProductAndVersion(CString & strProductName, CString & strProductVersion) {
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
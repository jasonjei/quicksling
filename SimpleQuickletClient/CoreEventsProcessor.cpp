#pragma once
#include "stdafx.h"
#include "CoreEventsProcessor.h"
#include "Conductor.h"
#include "resource.h"

extern Conductor defaultConductor;

LRESULT CoreEventsProcessor::OnCopyData(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	COPYDATASTRUCT* pCopyDataStruct = (COPYDATASTRUCT*)lParam;
	CString strRecievedText = (LPCTSTR)(pCopyDataStruct->lpData);
	// MessageBox(strRecievedText, _T("Received stuff from kkk!"), MB_OK);
	ATLTRACE2(atlTraceUI, 0, TEXT("Received stuff from QBCEP, %s\n"), strRecievedText);
	if (strRecievedText.CompareNoCase(_T("shutdown")) == 0) {
		// PostMessage(defaultConductor.orchestrator.cMainDlg->m_hWnd, WM_CLOSE, NULL, NULL);
		defaultConductor.orchestrator.brainRequestShutdown = true;
		SendMessage(defaultConductor.orchestrator.cMainDlg->m_hWnd, WM_CLOSE, NULL, NULL);
		// defaultConductor.orchestrator.StopConcert();

	}
	else if (strRecievedText.CompareNoCase(_T("version")) == 0) {

	}

	return true;
}

bool GetProductAndVersion(CStringA & strProductName, CStringA & strProductVersion)
{
	// get the filename of the executable containing the version resource
	TCHAR szFilename[MAX_PATH + 1] = { 0 };
	if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0)
	{
		TRACE("GetModuleFileName failed with error %d\n", GetLastError());
		return false;
	}

	// allocate a block of memory for the version info
	DWORD dummy;
	DWORD dwSize = GetFileVersionInfoSize(szFilename, &dummy);
	if (dwSize == 0)
	{
		TRACE("GetFileVersionInfoSize failed with error %d\n", GetLastError());
		return false;
	}
	std::vector<BYTE> data(dwSize);

	// load the version info
	if (!GetFileVersionInfo(szFilename, NULL, dwSize, &data[0]))
	{
		TRACE("GetFileVersionInfo failed with error %d\n", GetLastError());
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

	strProductVersion.Format("%u.%u.%u.%u",
		HIWORD(pFixedInfo->dwProductVersionMS),
		LOWORD(pFixedInfo->dwProductVersionMS),
		HIWORD(pFixedInfo->dwProductVersionLS),
		LOWORD(pFixedInfo->dwProductVersionLS));

	strProductName.SetString((LPCSTR)pvProductName, iProductNameLen);
	strProductVersion.SetString((LPCSTR)pvProductVersion, iProductVersionLen);

	return true;
}
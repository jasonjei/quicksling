#pragma once
#include "stdafx.h"
#include "CoreEventsProcessor.h"
#include "Conductor.h"

extern Conductor defaultConductor;
extern std::mutex mutexDataEvents;

LRESULT CoreEventsProcessor::OnCopyData(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	COPYDATASTRUCT* pCopyDataStruct = (COPYDATASTRUCT*)lParam;
	CString strRecievedText = (LPCTSTR)(pCopyDataStruct->lpData);
	// MessageBox(strRecievedText, _T("Received stuff from kkk!"), MB_OK);
	ATLTRACE2(atlTraceUI, 0, TEXT("Received stuff from QBCEP, %s\n"), strRecievedText);
	if (strRecievedText.CompareNoCase(_T("shutdown")) == 0) {
		// PostMessage(defaultConductor.orchestrator.cMainDlg->m_hWnd, WM_CLOSE, NULL, NULL);
		defaultConductor.orchestrator.brainRequestShutdown = true;
		// WaitForSingleObject(defaultConductor.orchestrator.request.signal, INFINITE);
		SendMessage(defaultConductor.orchestrator.cMainDlg->m_hWnd, WM_CLOSE, NULL, NULL);
		// defaultConductor.orchestrator.StopConcert();
	}
	else if (strRecievedText.CompareNoCase(_T("refresh")) == 0) {
		CString *newString = new CString(_T("0:/qb versions"));
		WaitForSingleObject(defaultConductor.orchestrator.request.signal, INFINITE);
		::PostThreadMessage(defaultConductor.orchestrator.request.threadID, LEVION_REQUEST, (WPARAM)newString, NULL);
	}
	else if (strRecievedText.Find(_T("version:")) == 0) {
		CString shellVersion = strRecievedText;
		shellVersion.Delete(0, 8);
		defaultConductor.orchestrator.shellVersion = shellVersion;
	}
	else if (strRecievedText.Find(_T("qbeventsxml:")) == 0) {
		CString qbxmleventxml = strRecievedText;
		qbxmleventxml.Delete(0, 12);
		{
			std::lock_guard<std::mutex> guard(mutexDataEvents);
			defaultConductor.orchestrator.dataEvents.push_back(qbxmleventxml);
			defaultConductor.orchestrator.newDataEvents.push_back(qbxmleventxml);
		}
	}
	return true;
}
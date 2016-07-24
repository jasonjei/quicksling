#pragma once
#include "stdafx.h"
#include "CoreEventsProcessor.h"
#include "Conductor.h"

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
	else if (strRecievedText.Find(_T("version:")) == 0) {
		CString shellVersion = strRecievedText;
		shellVersion.Delete(0, 8);
		defaultConductor.orchestrator.shellVersion = shellVersion;
	}
	return true;
}
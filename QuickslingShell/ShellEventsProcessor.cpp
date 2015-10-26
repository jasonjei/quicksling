#pragma once
#include "stdafx.h"
#include "ShellEventsProcessor.h"
#include "Conductor.h"

extern Conductor defaultConductor;

LRESULT ShellEventsProcessor::OnCopyData(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	COPYDATASTRUCT* pCopyDataStruct = (COPYDATASTRUCT*)lParam;
	CString strRecievedText = (LPCTSTR)(pCopyDataStruct->lpData);
	// MessageBox(strRecievedText, _T("Received stuff from kkk!"), MB_OK);
	if (strRecievedText.CompareNoCase(_T("shutdown")) == 0) {
		SetEvent(defaultConductor.orchestrator.goOfflineSignal);
		defaultConductor.orchestrator.StopConcert();
	}
	ATLTRACE2(atlTraceUI, 0, TEXT("Received stuff from QBSEP, %s\n"), strRecievedText);
	return true;
}
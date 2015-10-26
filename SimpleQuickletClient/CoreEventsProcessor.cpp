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
		defaultConductor.orchestrator.StopConcert();
	}
	return true;
}
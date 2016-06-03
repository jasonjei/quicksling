#include "stdafx.h"
#include "Conductor.h"
#include "DownloadProgressDlg.h"

extern Conductor defaultConductor;

LRESULT CDownloadProgressDlg::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = false;

	if (wParam == SC_CLOSE) {
		defaultConductor.orchestrator.downloader.currentDownloadAbort = 1;
	}
	return TRUE;
}

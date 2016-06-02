#include "stdafx.h"
#include "MainDlg.h"
#include "Conductor.h"

#pragma once

extern Conductor defaultConductor;

LRESULT CMainDlg::OnShowProgressBar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (this->progressDlg.m_hWnd == NULL) {
		HWND ret = this->progressDlg.Create(this->m_hWnd);
	}
	this->progressDlg.ShowWindow(SW_SHOWDEFAULT);

	CProgressBarCtrl progress(this->progressDlg.GetDlgItem(IDC_PROGRESS1));
	progress.SetPos(defaultConductor.orchestrator.downloader.currentProgress);

	CStatic label(this->progressDlg.GetDlgItem(IDC_STATIC));
	CString labelText;
	labelText.Format(_T("Downloading file: %s"), defaultConductor.orchestrator.downloader.currentFile);

	if (defaultConductor.orchestrator.downloader.bytesPerSecond) {
		std::ostringstream ss;
		ss << defaultConductor.orchestrator.downloader.bytesPerSecond / 1000.0;
		std::string s(ss.str());
		CString bytesPerSecString = CA2W(s.c_str(), CP_UTF8);

		if (defaultConductor.orchestrator.downloader.bytesTotal) {
			std::ostringstream ssBytesTotal;
			ssBytesTotal << (defaultConductor.orchestrator.downloader.bytesTotal / 1000.0) / 1000.0;
			std::string sBytesTotal(ssBytesTotal.str());
			CString megabytesTotal = CA2W(sBytesTotal.c_str(), CP_UTF8);

			labelText.Format(_T("Downloading file: %s - %s MB (%s KB/s)"), defaultConductor.orchestrator.downloader.currentFile, megabytesTotal, bytesPerSecString);
		}
		else {
			labelText.Format(_T("Downloading file: %s (%s KB/s)"), defaultConductor.orchestrator.downloader.currentFile, bytesPerSecString);

		}
	}
	label.SetWindowTextW(labelText);

	return TRUE;
}

LRESULT CMainDlg::OnHideProgressBar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (this->progressDlg.m_hWnd != NULL) {
		this->progressDlg.SendMessageW(WM_CLOSE, NULL, NULL);
	}
	return TRUE;
}
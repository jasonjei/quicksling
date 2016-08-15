// aboutdlg.h : interface of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"
#include "atlapp.h"
#include "atlframe.h"
#include "atlmisc.h"
#include "resource.h"
#include "spdlog\spdlog.h"

class Conductor;

class ShellEventsProcessor : public CDialogImpl<ShellEventsProcessor>
{
public:
	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(ShellEventsProcessor)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_COPYDATA, OnCopyData)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		SetWindowText(_T("QuickletShellEventsProcessor"));
		auto l = spdlog::get("quicksling_shell");
		l->info("Initialized QuickletShellEventsProcessor");
		return TRUE;
	}

	LRESULT OnCopyData(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

	//DWORD StartThread() {
	//	threadHandle = ::CreateThread(NULL, 0, CoreEventsProcessor::RunThread, NULL, 0, &threadID);
	//	return threadID;
	//}
	bool GetProductAndVersion(CString &strProductName, CString &strProductVersion);
};

// aboutdlg.h : interface of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "atlapp.h"
#include "atlframe.h"
#include "atlmisc.h"
#include "resource.h"
#pragma once

class CoreEventsProcessor : public CDialogImpl<CoreEventsProcessor>
{
public:
	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(CoreEventsProcessor)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_COPYDATA, OnCopyData)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		SetWindowText(_T("QuickletCoreEventsProcessor"));
		return TRUE;
	}

	LRESULT CoreEventsProcessor::OnCopyData(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

	//DWORD StartThread() {
	//	threadHandle = ::CreateThread(NULL, 0, CoreEventsProcessor::RunThread, NULL, 0, &threadID);
	//	return threadID;
	//}

};

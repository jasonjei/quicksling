// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "Downloader.h"
#include "DownloadProgressDlg.h"
#include "atlctrls.h"
#include "Constants.h"

extern Downloader downloader;

#pragma once

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
		public CMessageFilter, public CIdleHandler
{
public:
	CDownloadProgressDlg progressDlg;

	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(SHOW_DOWNLOAD_PROGRESS, OnShowProgressBar)
		MESSAGE_HANDLER(HIDE_DOWNLOAD_PROGRESS, OnHideProgressBar)

		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();

		// set icons
		HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
		SetIcon(hIconSmall, FALSE);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		UIAddChildWindowContainer(m_hWnd);
		int v = downloader.StartDownload();
		return TRUE;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if (this->progressDlg)
			this->progressDlg.CloseDialog(0);

		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		return 0;
	}

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// CAboutDlg dlg;
		// dlg.DoModal();

		int *ptr = NULL;
		*ptr = 0;

		if (this->progressDlg.m_hWnd == NULL) {
			HWND ret = this->progressDlg.Create(this->m_hWnd);
		}
		this->progressDlg.ShowWindow(SW_SHOWDEFAULT);
		CProgressBarCtrl progress(this->progressDlg.GetDlgItem(IDC_PROGRESS1));
		progress.SetPos(progress.GetPos() + 10);
		return 0;
	}

	LRESULT OnShowProgressBar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if (this->progressDlg.m_hWnd == NULL) {
			HWND ret = this->progressDlg.Create(this->m_hWnd);
		}
		this->progressDlg.ShowWindow(SW_SHOWDEFAULT);

		CProgressBarCtrl progress(this->progressDlg.GetDlgItem(IDC_PROGRESS1));
		progress.SetPos(downloader.currentProgress);

		CStatic label(this->progressDlg.GetDlgItem(IDC_STATIC));
		CString labelText;
		labelText.Format(_T("Downloading file: %s"), downloader.currentFile);

		if (downloader.bytesPerSecond) {
			std::ostringstream ss;
			ss << downloader.bytesPerSecond / 1000.0;
			std::string s(ss.str());
			CString bytesPerSecString = CA2W(s.c_str(), CP_UTF8);

			if (downloader.bytesTotal) {
				std::ostringstream ssBytesTotal;
				ssBytesTotal << (downloader.bytesTotal / 1000.0) / 1000.0;
				std::string sBytesTotal(ssBytesTotal.str());
				CString megabytesTotal = CA2W(sBytesTotal.c_str(), CP_UTF8);

				labelText.Format(_T("Downloading file: %s - %s MB (%s KB/s)"), downloader.currentFile, megabytesTotal, bytesPerSecString);
			}
			else {
				labelText.Format(_T("Downloading file: %s (%s KB/s)"), downloader.currentFile, bytesPerSecString);

			}
		}
		label.SetWindowTextW(labelText);

		return TRUE;
	}

	LRESULT OnHideProgressBar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if (this->progressDlg.m_hWnd != NULL) {
			this->progressDlg.SendMessageW(WM_CLOSE, NULL, NULL);
		}
		return TRUE;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: Add validation code 
		CloseDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(wID);
		return 0;
	}

	void CloseDialog(int nVal)
	{
		DestroyWindow();
		{
			std::unique_lock<std::mutex> lk(downloader.download_mutex);
			downloader.cvThreadFinished.wait(lk, [] { return 1; });
		}
		::PostQuitMessage(nVal);
	}
};

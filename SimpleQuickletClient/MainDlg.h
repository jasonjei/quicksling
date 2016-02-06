// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "stdafx.h"
#include <atlframe.h>
#include "AboutDlg.h"
#include "resource.h"
#include "Constants.h"
#include "Orchestrator.h"
#include "Settings.h"
#include "CoreEventsProcessor.h"
#include "LevionMisc.h"
#include "trayiconimpl.h"

extern Orchestrator *defaultOrchestrator;

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
	public CMessageFilter, public CIdleHandler, public CTrayIconImpl<CMainDlg>
{
public:
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
		MESSAGE_HANDLER(QUICKLET_CONNECT_UPD, OnConnUpdate)
		MESSAGE_HANDLER(LEVION_MESSAGE_BOX, DisplayMessage)
		MESSAGE_HANDLER(LEVION_TRAYICON_MSG, DisplayNotification)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		CHAIN_MSG_MAP(CTrayIconImpl<CMainDlg>)
	END_MSG_MAP()

	CoreEventsProcessor cep;

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnConnUpdate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		if (defaultOrchestrator->longPoll.connected == true) {
			
			SetDlgItemText(IDC_CONN_STATIC, _T("CONNECTED!"));
		} 
		else {
			// SetDlgItemText(IDC_)
			SetDlgItemText(IDC_CONN_STATIC, _T("NOT CONNECTED"));

		}
		return 1;
	}
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();

		// set icons
		HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
		SetIcon(hIconSmall, FALSE);

		InstallIcon(APP_NAME, hIconSmall, IDR_POPUP);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		UIAddChildWindowContainer(m_hWnd);
		SetDlgItemText(IDC_EDIT1, URLS::GOLIATH_SERVER);
		SetDlgItemText(IDC_EDIT2, defaultOrchestrator->qbInfo.authToken);

		HWND ret = this->cep.Create(this->m_hWnd);

		return TRUE;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if (!defaultOrchestrator->brainRequestShutdown) {
			CString strWindowTitle = _T("QuickletShellEventsProcessor");
			CString strDataToSend = _T("shutdown");

			LRESULT copyDataResult;

			CWindow pOtherWnd = (HWND)FindWindow(NULL, strWindowTitle);

			if (pOtherWnd) {
				COPYDATASTRUCT cpd;
				cpd.dwData = NULL;
				cpd.cbData = strDataToSend.GetLength() * sizeof(wchar_t) + 1;
				cpd.lpData = strDataToSend.GetBuffer(cpd.cbData);
				copyDataResult = pOtherWnd.SendMessage(WM_COPYDATA,
					(WPARAM) this->m_hWnd,
					(LPARAM)&cpd);
				strDataToSend.ReleaseBuffer();
				// copyDataResult has value returned by other app

			}
			else {
				// MessageBox(_T("Can't find other app!"), _T("UH OH"), MB_OK);
				// AfxMessageBox("Unable to find other app.");
			}
		}
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		return 0;
	}

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}

	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		// TODO: Add validation code 
		// CloseDialog(wID);

		BSTR urlCtrlTextBStr = SysAllocString(NULL);

		GetDlgItemText(IDC_EDIT1, urlCtrlTextBStr);

		URLS::GOLIATH_SERVER = urlCtrlTextBStr;

		SysFreeString(urlCtrlTextBStr);
		urlCtrlTextBStr = SysAllocString(NULL);

		GetDlgItemText(IDC_EDIT2, urlCtrlTextBStr);

		defaultOrchestrator->qbInfo.authToken = urlCtrlTextBStr;


		SysFreeString(urlCtrlTextBStr);

		Settings settings(HKEY_CURRENT_USER,
			_T("Software\\Quicklet\\DevClient\\1.0"));

		settings.Load(); // Load configuration

		settings.ClientKey = defaultOrchestrator->qbInfo.authToken;
		settings.URL = URLS::GOLIATH_SERVER;
		settings.Save();

		InternetCloseHandle(defaultOrchestrator->longPoll.currentHandle);
		SetEvent(defaultOrchestrator->longPoll.tryAgainSignal);


		return 0;
	}

	LRESULT DisplayNotification(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		TrayMessage lNIS = *((TrayMessage*)wParam);
		delete (TrayMessage*)wParam;

		SetTooltipText(lNIS.Title, lNIS.Body);

		return TRUE;
	}

	LRESULT DisplayMessage(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CString textToDisplay = *((CString*)wParam);
		delete (CString*)wParam;

		MessageBox(textToDisplay, _T("An important message from the Quicklet server"), MB_OK);

		return TRUE;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(wID);
		return 0;
	}

	void CloseDialog(int nVal)
	{
		RemoveIcon();
		cep.DestroyWindow();

		DestroyWindow();
		defaultOrchestrator->StopConcert();
		::PostQuitMessage(nVal);
	}
};

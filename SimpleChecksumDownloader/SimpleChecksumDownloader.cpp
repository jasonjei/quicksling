// SimpleChecksumDownloader.cpp : main source file for SimpleChecksumDownloader.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include "resource.h"

#include "aboutdlg.h"
#include "MainDlg.h"

#include <iostream>
#include <iomanip>
#include "downloader.h"
#include <wininet.h>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "urlmon.lib")

CAppModule _Module;
Downloader downloader;
CMainDlg* cMainDlg;

class CallbackHandler : public IBindStatusCallback
{
private:
	int m_percentLast;

public:
	CallbackHandler() : m_percentLast(0)
	{
	}

	// IUnknown

	HRESULT STDMETHODCALLTYPE
		QueryInterface(REFIID riid, void** ppvObject)
	{

		if (IsEqualIID(IID_IBindStatusCallback, riid)
			|| IsEqualIID(IID_IUnknown, riid))
		{
			*ppvObject = reinterpret_cast<void*>(this);
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE
		AddRef()
	{
		return 2UL;
	}

	ULONG STDMETHODCALLTYPE
		Release()
	{
		return 1UL;
	}

	// IBindStatusCallback

	HRESULT STDMETHODCALLTYPE
		OnStartBinding(DWORD     /*dwReserved*/,
		IBinding* /*pib*/)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE
		GetPriority(LONG* /*pnPriority*/)
	{
		return E_NOTIMPL; 
	}

	HRESULT STDMETHODCALLTYPE
		OnLowResource(DWORD /*reserved*/)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE
		OnProgress(ULONG   ulProgress,
		ULONG   ulProgressMax,
		ULONG   ulStatusCode,
		LPCWSTR /*szStatusText*/)
	{
		switch (ulStatusCode)
		{
		case BINDSTATUS_FINDINGRESOURCE:
			// tcout << _T("Finding resource...") << endl;
			break;
		case BINDSTATUS_CONNECTING:
			// tcout << _T("Connecting...") << endl;
			break;
		case BINDSTATUS_SENDINGREQUEST:
			// tcout << _T("Sending request...") << endl;
			break;
		case BINDSTATUS_MIMETYPEAVAILABLE:
			// tcout << _T("Mime type available") << endl;
			break;
		case BINDSTATUS_CACHEFILENAMEAVAILABLE:
			// tcout << _T("Cache filename available") << endl;
			break;
		case BINDSTATUS_BEGINDOWNLOADDATA:
			// tcout << _T("Begin download") << endl;
			break;
		case BINDSTATUS_DOWNLOADINGDATA:
		case BINDSTATUS_ENDDOWNLOADDATA:
		{
			int percent = (int)(100.0 * static_cast<double>(ulProgress)
				/ static_cast<double>(ulProgressMax));
			if (m_percentLast < percent)
			{
				downloader.currentProgress = percent;
				// LoadBar(percent, 100);
				m_percentLast = percent;
			}
			if (ulStatusCode == BINDSTATUS_ENDDOWNLOADDATA)
			{
				// tcout << endl << _T("End download") << endl;
			}
		}
		break;

		default:
		{
			// tcout << _T("Status code : ") << ulStatusCode << endl;
		}
		}
		// The download can be cancelled by returning E_ABORT here
		// of from any other of the methods.
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE
		OnStopBinding(HRESULT /*hresult*/,
		LPCWSTR /*szError*/)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE
		GetBindInfo(DWORD*    /*grfBINDF*/,
		BINDINFO* /*pbindinfo*/)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE
		OnDataAvailable(DWORD      /*grfBSCF*/,
		DWORD      /*dwSize*/,
		FORMATETC* /*pformatetc*/,
		STGMEDIUM* /*pstgmed*/)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE
		OnObjectAvailable(REFIID    /*riid*/,
		IUnknown* /*punk*/)
	{
		return E_NOTIMPL;
	}
};


int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainDlg dlgMain;
	cMainDlg = &dlgMain;

	if(dlgMain.Create(NULL) == NULL)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	dlgMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}

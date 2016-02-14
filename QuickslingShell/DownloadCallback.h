#pragma once

#include <urlmon.h>    
#include <iostream>
#include "unzip.h"
#include <atlmisc.h>
#include "Constants.h"
#include <sstream>
#include <strsafe.h>
#include <exception>
#include "Conductor.h"

extern Conductor defaultConductor;

class DownloadCallback : public IBindStatusCallback {
public:
	HWND hprog;
	int downloadedBytes;

	DownloadCallback::DownloadCallback(HWND hprog, int totalBytes) : downloadedBytes(0) {
		this->hprog = hprog;
		SendMessage(hprog, PBM_SETRANGE32, 0, (LPARAM)totalBytes);
	}

    DownloadCallback::~DownloadCallback() { }
	
    STDMETHOD(OnProgress)(/* [in] */ ULONG ulProgress, /* [in] */ ULONG ulProgressMax, /* [in] */ ULONG ulStatusCode, /* [in] */ LPCWSTR wszStatusText)
    {
		if (ulStatusCode == BINDSTATUS_BEGINDOWNLOADDATA || ulStatusCode == BINDSTATUS_DOWNLOADINGDATA) {
			this->downloadedBytes += ulProgress;
			//SendDlgItemMessage(this->hprog, IDC_PROGRESS1, PBM_SETPOS, this->downloadedBytes, 0);
			int test = SendMessage(this->hprog, PBM_GETPOS, 0, 0);
			SendMessage(this->hprog, PBM_SETPOS, (LPARAM)this->downloadedBytes, 0);
			
		}
		PumpMessages();
        return S_OK;
    }

    // The rest  don't do anything...
    STDMETHOD(OnStartBinding)(/* [in] */ DWORD dwReserved, /* [in] */ IBinding __RPC_FAR *pib)
    { PumpMessages();return E_NOTIMPL; }

    STDMETHOD(GetPriority)(/* [out] */ LONG __RPC_FAR *pnPriority)
    { PumpMessages();return E_NOTIMPL; }

    STDMETHOD(OnLowResource)(/* [in] */ DWORD reserved)
    { PumpMessages();return E_NOTIMPL; }

    STDMETHOD(OnStopBinding)(/* [in] */ HRESULT hresult, /* [unique][in] */ LPCWSTR szError)
    { PumpMessages();return E_NOTIMPL; }

    STDMETHOD(GetBindInfo)(/* [out] */ DWORD __RPC_FAR *grfBINDF, /* [unique][out][in] */ BINDINFO __RPC_FAR *pbindinfo)
    { PumpMessages();return E_NOTIMPL; }

    STDMETHOD(OnDataAvailable)(/* [in] */ DWORD grfBSCF, /* [in] */ DWORD dwSize, /* [in] */ FORMATETC __RPC_FAR *pformatetc, /* [in] */ STGMEDIUM __RPC_FAR *pstgmed)
    { PumpMessages();return E_NOTIMPL; }

    STDMETHOD(OnObjectAvailable)(/* [in] */ REFIID riid, /* [iid_is][in] */ IUnknown __RPC_FAR *punk)
    { PumpMessages();return E_NOTIMPL; }

	// IUnknown stuff
    STDMETHOD_(ULONG,AddRef)()
    { return 0; }

    STDMETHOD_(ULONG,Release)()
    { return 0; }

    STDMETHOD(QueryInterface)(/* [in] */ REFIID riid, /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
    { return E_NOTIMPL; }

	void PumpMessages()
	{
		for (MSG msg;;) {
				BOOL res=PeekMessage(&msg,0,0,0,PM_REMOVE);
				if (!res||msg.message==WM_QUIT) return;
				TranslateMessage(&msg); DispatchMessage(&msg);
		}
	}
};
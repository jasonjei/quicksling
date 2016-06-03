#ifndef DOWNLOADER_CALLBACK_HANDLER_H
#define DOWNLOADER_CALLBACK_HANDLER_H

#include <iostream>
#include <iomanip>

#include <wininet.h>
#include <chrono>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "urlmon.lib")
#pragma once

class DownloaderCallbackHandler : public IBindStatusCallback
{
private:
	int m_percentLast;

	ULONG lastBytes;
	std::chrono::high_resolution_clock::time_point lastTime;

public:
	DownloaderCallbackHandler() : m_percentLast(0), lastBytes(0)
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
		LPCWSTR /*szStatusText*/); /*
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
			// SendMessage(cMainDlg->m_hWnd, SHOW_DOWNLOAD_PROGRESS, NULL, NULL);
			// tcout << _T("Begin download") << endl;
			break;
		case BINDSTATUS_DOWNLOADINGDATA:
		case BINDSTATUS_ENDDOWNLOADDATA:
		{
			int percent = (int)(100.0 * static_cast<double>(ulProgress)
				/ static_cast<double>(ulProgressMax));
			if (m_percentLast < percent)
			{
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
	*/

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
		OnDataAvailable(DWORD      grfBSCF,
		DWORD      dwSize,
		FORMATETC* /*pformatetc*/,
		STGMEDIUM* /*pstgmed*/)
	{
		return S_OK;
		//return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE
		OnObjectAvailable(REFIID    /*riid*/,
		IUnknown* /*punk*/)
	{
		return E_NOTIMPL;
	}
};
#endif
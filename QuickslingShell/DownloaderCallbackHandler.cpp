#include "stdafx.h"
#include "DownloaderCallbackHandler.h"
#include "MainDlg.h"
#include "Constants.h"
#include "Conductor.h"

extern Conductor defaultConductor;

HRESULT STDMETHODCALLTYPE
DownloaderCallbackHandler::OnProgress(ULONG   ulProgress,
ULONG   ulProgressMax,
ULONG   ulStatusCode,
LPCWSTR /*szStatusText*/) {
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
		SendMessage(defaultConductor.orchestrator.cMainDlg->m_hWnd, SHOW_DOWNLOAD_PROGRESS, NULL, NULL);
		// tcout << _T("Begin download") << endl;
		break;
	case BINDSTATUS_DOWNLOADINGDATA:
	case BINDSTATUS_ENDDOWNLOADDATA:
	{
		int percent = (int)(100.0 * static_cast<double>(ulProgress)
			/ static_cast<double>(ulProgressMax));
		if (m_percentLast < percent)
		{
			std::chrono::high_resolution_clock::time_point nowTime = std::chrono::high_resolution_clock::now();
			/*
			if (lastTime < nowTime) {
				std::chrono::milliseconds fsec = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - lastTime);
				
				if (fsec.count() < 0)
					std::cout << "wut";

				float bytesPerSec = (lastBytes) / (fsec.count() * 0.001);

				fsec.count();
				downloader.bytesPerSecond = bytesPerSec;
			}
			*/
			defaultConductor.orchestrator.downloader.bytesTotal = ulProgressMax;

			if (lastTime.time_since_epoch().count() != 0) {
				std::chrono::milliseconds fsec = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - lastTime);
				defaultConductor.orchestrator.downloader.bytesPerSecond = (ulProgress - lastBytes) / (((float)fsec.count()) * 0.001);
			}

			lastBytes = ulProgress;
			lastTime = nowTime;
			if (defaultConductor.orchestrator.downloader.currentDownloadAbort == 0)
				PostMessage(defaultConductor.orchestrator.cMainDlg->m_hWnd, SHOW_DOWNLOAD_PROGRESS, NULL, NULL);
			defaultConductor.orchestrator.downloader.currentProgress = percent;
			// LoadBar(percent, 100);
			m_percentLast = percent;
		}
		if (ulStatusCode == BINDSTATUS_ENDDOWNLOADDATA)
		{
			// tcout << endl << _T("End download") << endl;
			SendMessage(defaultConductor.orchestrator.cMainDlg->m_hWnd, HIDE_DOWNLOAD_PROGRESS, NULL, NULL);

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

	if (defaultConductor.orchestrator.downloader.currentDownloadAbort == 1)
		return E_ABORT;

	return S_OK;
}
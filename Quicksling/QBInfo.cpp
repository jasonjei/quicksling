#pragma once

#include "stdafx.h"
#include "QBInfo.h"
#include <atlapp.h>
#include <atlmisc.h>
#import <msxml3.dll> named_guids raw_interfaces_only
#include <ShlObj.h>
#include "INet.h"
#include "Orchestrator.h"
#include "Conductor.h"
#include "MainDlg.h"

int QBInfo::RegisterConnector() {
	ResetEvent(this->readyForLongPollSignal);

	// You will also use a temporary auth token until 
	CString sURL;
	sURL = URLS::APP_SERVER + "get_started/register_connector.xml?guid=" + this->clientGuid + "&qbxml_versions=" + this->qbxmlVersions + "&company_name=" + UrlEncode(this->companyName);
	if (this->companyTag != "")
		sURL += "&company_tag=" + this->companyTag;

	CInternetSession Session(APP_NAME);
	INTERNET_STATUS_CALLBACK iscCallback;

	iscCallback = InternetSetStatusCallback((HINTERNET) Session, (INTERNET_STATUS_CALLBACK) CallMaster);

	CHttpFile* cHttpFile = NULL;
	int fail = 0;

	try {
		cHttpFile = new CHttpFile(Session, sURL, NULL, 0, INTERNET_FLAG_DONT_CACHE);
	}

	catch(CInternetException&) {
		fail = 1;
	}

	if (! fail) {
		WTL::CString pageSource;

		UINT bytes = (UINT) cHttpFile->GetLength();

		char tChars[2048+1];
		int bytesRead;

		while((bytesRead = cHttpFile->Read((LPVOID) tChars, 2048)) != 0) {
			tChars[bytesRead] = '\0';
			pageSource += CA2W((LPCSTR) tChars, CP_UTF8);
		}

		MSXML2::IXMLDOMDocument* outputXMLDoc = InstantiateXMLDocWithString(pageSource);
		if (outputXMLDoc) {
			CString error = GetValueFromNodeString(_T("error"), outputXMLDoc);

			if (error.Compare(_T("quickbooks_too_old")) == 0) {
				// TODO - Update error message to reflect that the user might not have given our app permission in qb.
				MessageBox(NULL, _T("Please upgrade your Quickbooks to at least version 6.0 to use Levion"), _T("Incompatible Quickbooks Version"), MB_OK);
				return 0;
			} else if (error.Compare(_T("company_already_online")) == 0) {
				MessageBox(NULL, _T("Your company is already connected to Levion. Please close Quickbooks and then reopen it."), _T("Company Already Online"), MB_OK);
				return 0;
			}

			this->RemoveTagFromYaml();
			this->companyTag = GetValueFromNodeString(_T("company-tag"), outputXMLDoc);
			this->authToken = GetValueFromNodeString(_T("auth-token"), outputXMLDoc);
			this->subdomain = GetValueFromNodeString(_T("subdomain"), outputXMLDoc);

			if (this->companyTag == "") {
				return 0;
			}
			// When saving new company tags, the old one remains in the file. Also need to write new tag to QB file
			SaveConfigYaml();

			if (GetValueFromNodeString(_T("need-login"), outputXMLDoc) == "true") {
				/*
				if (this->orchestrator->browser.browser == NULL)
					this->orchestrator->browser.StartThread();
				WaitForSingleObject(this->orchestrator->browser.browserOpenSignal, INFINITE);

				this->orchestrator->browser.browser->GetMainFrame()->LoadURL(std::string(CW2A(URLS::APP_SERVER + "get_started/welcome?guid=" + this->clientGuid, CP_UTF8)));
				*/
			}
			outputXMLDoc->Release();
			SetEvent(this->readyForLongPollSignal);
		}
	}
	delete cHttpFile;

	// Set auth token from response and call SaveAuthTokenToYaml();
	return 1;
}

void QBInfo::LaunchBrowser(CString url) {
	std::string ansiUrl = ATL::CW2A(url, CP_UTF8);
	if (!this->orchestrator->browser.simpleHandler->BrowserAvailable()) {
		CefWindowInfo window_info;

		// window_info.SetAsPopup(this->orchestrator->cMainDlg->m_hWnd, "Quicksling");
		window_info.SetAsPopup(NULL, "Quicksling");
		CefBrowserSettings browser_settings;

		CefBrowserHost::CreateBrowserSync(window_info, this->orchestrator->browser.simpleHandler, ansiUrl,
			browser_settings, NULL);
	}
	else {
		this->orchestrator->browser.simpleHandler->GetBrowser()->GetMainFrame()->LoadURL(ansiUrl);
	}

}
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

extern Conductor defaultConductor;

int QBInfo::Reset() {
	SetEvent(readyForLongPollSignal);

	CString oldAuthToken = authToken;
	this->state = "LINKDEAD";

	CString sURL = URLS::GOLIATH_SERVER + "client/linkdead?auth_key=" + oldAuthToken +
		"&old_auth_key=" + oldAuthToken;

	CInternetSession session(APP_NAME);
	session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 1);
	session.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 1);
	CHttpFile *cHttpFile = NULL;

	int fail = 0;

	try {
		cHttpFile = new CHttpFile(session, sURL, NULL, 0, INTERNET_FLAG_DONT_CACHE);
	}

	catch (CInternetException&) {
		fail = 1;
	}

	if (!fail) {
		WTL::CString pageSource;

		UINT bytes = (UINT)cHttpFile->GetLength();

		char tChars[2048 + 1];
		int bytesRead;

		while ((bytesRead = cHttpFile->Read((LPVOID)tChars, 2048)) != 0) {
			tChars[bytesRead] = '\0';
			pageSource += tChars;
		}

		pageSource.Format(_T("%s"), pageSource);

	}

	delete cHttpFile;
	session.Close();
	return 1;
}

int QBInfo::GetInfoFromQB() {
	if (WaitForSingleObject(this->orchestrator->longPoll.goOfflineSignal, 0) == 0) {
		return 1;
	}
	// std::lock_guard<std::mutex> guard(mutexQBInfo);
	qbXMLRPWrapper qb;
	
	if (WaitForSingleObject(this->orchestrator->longPoll.goOfflineSignal, 0) == 0) {
		return 1;
	}
	
	qb.OpenCompanyFile(_T(""));
	
	if (WaitForSingleObject(this->orchestrator->longPoll.goOfflineSignal, 0) == 0) {
		return 1;
	}

	CString result = qb.ProcessRequest(std::wstring(GET_COMPANY_TAG)).c_str();
	CString originalUniqueId = GetUniqueID();

	SetCompanyInfo(result);

	if (WaitForSingleObject(this->orchestrator->longPoll.goOfflineSignal, 0) == 0) {
		return 1;
	}

	result = qb.ProcessRequest(std::wstring(GET_TEMPLATES)).c_str();

	if (WaitForSingleObject(this->orchestrator->longPoll.goOfflineSignal, 0) == 0) {
		return 1;
	}

	SetCompanyTemplateInfo(result);

	if (WaitForSingleObject(this->orchestrator->longPoll.goOfflineSignal, 0) == 0) {
		return 1;
	}

	// SetQBInfo();

	if (WaitForSingleObject(this->orchestrator->longPoll.goOfflineSignal, 0) == 0) {
		return 1;
	}

	this->qbxmlVersions = qb.GetVersions();

	if (WaitForSingleObject(this->orchestrator->longPoll.goOfflineSignal, 0) == 0) {
		return 1;
	}

	CString versionResult = qb.ProcessRequest(std::wstring(_T("<?xml version=\"1.0\"?><?qbxml version=\"8.0\"?><QBXML><QBXMLMsgsRq onError=\"stopOnError\"><HostQueryRq></HostQueryRq></QBXMLMsgsRq></QBXML>"))).c_str();

	if (WaitForSingleObject(this->orchestrator->longPoll.goOfflineSignal, 0) == 0) {
		return 1;
	}

	MSXML2::IXMLDOMDocument* outputXMLDoc = InstantiateXMLDocWithString(versionResult);

	if (outputXMLDoc) {
		this->productName = GetValueFromNodeString(_T("ProductName"), outputXMLDoc);
		this->country = GetValueFromNodeString(_T("Country"), outputXMLDoc);

		outputXMLDoc->Release();
		// return 1;
	}

	if ((originalUniqueId != GetUniqueID()) && (originalUniqueId != _T(","))) {
		ResetEvent(this->readyForLongPollSignal);
		Reset();
		this->authToken = _T("");
		this->hasRun = false;
	}

	LoadConfigYaml();
	SaveConfigYaml();

	SetEvent(this->readyForLongPollSignal);

	return 1;
}

int QBInfo::SetQBInfo() {
	qbXMLRPWrapper qb;

	if (WaitForSingleObject(this->orchestrator->longPoll.threadHandle, 0) == 0) {
		return 1;
	}

	qb.OpenCompanyFile(_T(""));

	if (WaitForSingleObject(this->orchestrator->longPoll.threadHandle, 0) == 0) {
		return 1;
	}

	this->qbxmlVersions = qb.GetVersions();

	if (WaitForSingleObject(this->orchestrator->longPoll.threadHandle, 0) == 0) {
		return 1;
	}

	CString versionResult = qb.ProcessRequest(std::wstring(_T("<?xml version=\"1.0\"?><?qbxml version=\"8.0\"?><QBXML><QBXMLMsgsRq onError=\"stopOnError\"><HostQueryRq></HostQueryRq></QBXMLMsgsRq></QBXML>"))).c_str();
	
	if (WaitForSingleObject(this->orchestrator->longPoll.threadHandle, 0) == 0) {
		return 1;
	}

	MSXML2::IXMLDOMDocument* outputXMLDoc = InstantiateXMLDocWithString(versionResult);

	if (outputXMLDoc) {
		this->productName = GetValueFromNodeString(_T("ProductName"), outputXMLDoc);
		this->country = GetValueFromNodeString(_T("Country"), outputXMLDoc);

		outputXMLDoc->Release();
		return 1;
	}

	return 0;
}

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
			// this->authToken = GetValueFromNodeString(_T("auth-token"), outputXMLDoc);
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
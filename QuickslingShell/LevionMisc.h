#ifndef LEVION_MISC
#define LEVION_MISC 1

#include "inifile.h"

struct TrayMessage {
	CString Title;
	CString Body;
};

struct UpdatesInfo {
	std::vector<std::wstring> filesToDownload;
	int totalBytes;
	CIniFile serverCRCIni;

	HWND dialogWindow;
};

static TrayMessage* BuildTrayMessage(CString title, CString body) {
	TrayMessage *trayMessage = new TrayMessage;
	trayMessage->Title = title;
	trayMessage->Body = body;

	return trayMessage;
};

static CString UrlEncode(CString url) {
	CString escapedMessage;
	DWORD escapeLength = (url.GetLength() * 9) + 1;

	::UrlEscapeW(url, escapedMessage.GetBufferSetLength(escapeLength), &escapeLength, URL_ESCAPE_SEGMENT_ONLY | URL_ESCAPE_UNSAFE);
	escapedMessage.ReleaseBuffer();
	escapedMessage.Replace(_T(";"), _T("%3B"));
	return escapedMessage;
}

static CString VisitURL(CString url) {
	WTL::CString pageSource = _T("");

	CInternetSession session(APP_NAME);
	INTERNET_STATUS_CALLBACK iscCallback;

	WORD timeout = 40000;

	session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, timeout);
	session.SetOption(INTERNET_OPTION_SEND_TIMEOUT, timeout);

	CHttpFile* cHttpFile = NULL;
	int fail = 0;

	try {
		cHttpFile = new CHttpFile(session, url, NULL, 0, INTERNET_FLAG_DONT_CACHE);
	}

	catch(CInternetException& e) {
		fail = 1;
		e;
	}

	if (! fail) {

		UINT bytes = (UINT) cHttpFile->GetLength();

		char tChars[2048+1];
		int bytesRead;

		while((bytesRead = cHttpFile->Read((LPVOID) tChars, 2048)) != 0) {
			tChars[bytesRead] = '\0';
			pageSource += CA2W((LPCSTR) tChars, CP_UTF8);
		}
	}

	delete cHttpFile;
	session.Close();
	return pageSource;
}

#endif
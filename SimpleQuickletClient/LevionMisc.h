#ifndef LEVION_MISC
#define LEVION_MISC 1

struct TrayMessage {
	CString Title;
	CString Body;
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
	url.Replace(_T("%"), _T("%25"));
	::UrlEscapeW(url, escapedMessage.GetBufferSetLength(escapeLength), &escapeLength, URL_ESCAPE_SEGMENT_ONLY | URL_ESCAPE_UNSAFE);
	escapedMessage.ReleaseBuffer();
	escapedMessage.Replace(_T(";"), _T("%3B"));
	escapedMessage.Replace(_T("'"), _T("%27"));
	escapedMessage.Replace(_T(","), _T("%2C"));
	return escapedMessage;
}


#endif
#pragma once

#include <atlapp.h>
#include <atlmisc.h>

class ResponseEnvelope {
public:
	ResponseEnvelope() : doNotReply(false) {
		qbMessage = 0;
	}

	CString responseKey;
	CString message;
	CString body;
	CString reply;
	int qbMessage;
	bool doNotReply;
};
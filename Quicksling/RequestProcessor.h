#pragma once

#include "stdafx.h"
#include "Constants.h"
#include <atlmisc.h>
#include <string>
#include <map>
#include "qbXMLRPWrapper.h"
#include "Constants.h"

class CMainDlg;
class Orchestrator;
class ResponseEnvelope;

class RequestProcessor {
public:
	typedef int (RequestProcessor::*MFP)(ResponseEnvelope*);
	RequestProcessor(void);
	~RequestProcessor(void);
	DWORD StartThread();
	void ProcessText(MSG *pMsg, ResponseEnvelope *res);

	/* Commands Begin Here */
	int cmd_show_message(ResponseEnvelope* res);
	int cmd_misunderestimate(ResponseEnvelope* res);
	int cmd_ping(ResponseEnvelope* res);
	int cmd_utctime(ResponseEnvelope* res);
	int cmd_utcoffset(ResponseEnvelope* res);
	int cmd_closemodal(ResponseEnvelope* res);
	int cmd_debug_timeout(ResponseEnvelope* res);
	int cmd_get_events(ResponseEnvelope* res);
	int cmd_browser(ResponseEnvelope* res);
	int cmd_clear_events(ResponseEnvelope* res);
	int cmd_qb(ResponseEnvelope* res);
	int cmd_quit(ResponseEnvelope* res);
	int cmd_update(ResponseEnvelope* res);
	int cmd_start_test_sync(ResponseEnvelope* res);
	int cmd_query_lost_data_events(ResponseEnvelope* res);
	int cmd_subscribexml(ResponseEnvelope* res);
	int cmd_crash_self(ResponseEnvelope* res);
	int cmd_crash_shell(ResponseEnvelope* res);
	int cmd_create_process(ResponseEnvelope* res);

	static DWORD WINAPI RunThread(LPVOID lpData);

	Orchestrator *orchestrator;
	HANDLE signal;
	HANDLE threadHandle;
	CMainDlg* mainDlg;
	DWORD threadID;
	MSG lastMsg;
};

struct Actions {
	typedef int (RequestProcessor::*MFP)(ResponseEnvelope*);
	std::map <std::string, MFP> fmap;

	Actions() {
		fmap.insert(std::make_pair("/qb", &RequestProcessor::cmd_qb));
		fmap.insert(std::make_pair("/ping", &RequestProcessor::cmd_ping));
		fmap.insert(std::make_pair("/utctime", &RequestProcessor::cmd_utctime));
		fmap.insert(std::make_pair("/utcoffset", &RequestProcessor::cmd_utcoffset));
		fmap.insert(std::make_pair("/closemodal", &RequestProcessor::cmd_closemodal));
		fmap.insert(std::make_pair("/debug_timeout", &RequestProcessor::cmd_debug_timeout));
		fmap.insert(std::make_pair("/get_events", &RequestProcessor::cmd_get_events));
		fmap.insert(std::make_pair("/browser", &RequestProcessor::cmd_browser));
		fmap.insert(std::make_pair("/clear_events", &RequestProcessor::cmd_clear_events));
		fmap.insert(std::make_pair("/show_message", &RequestProcessor::cmd_show_message));
		fmap.insert(std::make_pair("/offline", &RequestProcessor::cmd_quit));
		fmap.insert(std::make_pair("/update", &RequestProcessor::cmd_update));
		fmap.insert(std::make_pair("/query_lost_data_events", &RequestProcessor::cmd_query_lost_data_events));
		fmap.insert(std::make_pair("/start_test_sync", &RequestProcessor::cmd_start_test_sync));
		fmap.insert(std::make_pair("/subscribexml", &RequestProcessor::cmd_subscribexml));
		fmap.insert(std::make_pair("/crashself", &RequestProcessor::cmd_crash_self));
		fmap.insert(std::make_pair("/crashshell", &RequestProcessor::cmd_crash_shell));
		fmap.insert(std::make_pair("/createproc", &RequestProcessor::cmd_create_process));
	}

	int Call(const std::string & s, RequestProcessor* request, ResponseEnvelope* respEnvelope) {
		MFP fp = fmap[s];
		if (fp)
			return (request->*fp)(respEnvelope);
		else
			return (request->cmd_misunderestimate)(respEnvelope);
		return 1;
	}
};
#pragma once

#include "Orchestrator.h"
#include "MainDlg.h"

class Conductor {
public:
	Orchestrator orchestrator;
	CMainDlg* mainDlg;
	bool askToSendCrashRpt;

	Conductor(void) {
		orchestrator.SetDlgMain(NULL);
		setupMaxConnectionLimit();
	}

	Conductor(CMainDlg* mainDlg) {
		askToSendCrashRpt = true;
		orchestrator = Orchestrator();
		orchestrator.SetDlgMain(mainDlg);
		setupMaxConnectionLimit();
		orchestrator.StartConcert();
	}

	~Conductor(void) {
	}

	static int setupMaxConnectionLimit() {
		int maxConnections = 16;
		InternetSetOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &maxConnections, sizeof(int));
		InternetSetOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER, &maxConnections, sizeof(int));
		return 1;
	}
};
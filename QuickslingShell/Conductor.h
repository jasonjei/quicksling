#pragma once

#include "Orchestrator.h"
#include "MainDlg.h"

class Conductor {
public:
	Orchestrator orchestrator;
	bool askToSendCrashRpt;
	bool updateRequested;

	Conductor(void) : updateRequested(false) {
		askToSendCrashRpt = true;
	}

	~Conductor(void) {
	}

};
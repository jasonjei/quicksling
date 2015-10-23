#pragma once

#include "Orchestrator.h"
#include "MainDlg.h"

class Conductor {
public:
	Orchestrator orchestrator;
	bool askToSendCrashRpt;

	Conductor(void) {
		askToSendCrashRpt = true;
	}

	~Conductor(void) {
	}

};
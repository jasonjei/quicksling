#pragma once

#include "stdafx.h"
#include <queue>
#include "EventHandler.h"
#include "Conductor.h"
#include "Orchestrator.h"
#include <string>
#include "Constants.h"

extern Conductor defaultConductor;
extern Orchestrator *defaultOrchestrator;

EventHandler::EventHandler(void) {
	signal = CreateEvent(NULL, TRUE, FALSE, NULL);
}

EventHandler::~EventHandler(void) {
}

int EventHandler::ClearDataEvents() {
	this->dataEvents.clear();
	return 1;
}

int EventHandler::ProcessEvent(CString strMsg) {
	int isUIEvent = defaultOrchestrator->info.SetState(strMsg);
	
	if (!isUIEvent) {
		dataEvents.push_back(strMsg);
		// CString *newString = new CString(strMsg);
		// ::PostThreadMessage(defaultOrchestrator->pipeWrite.threadID, PIPE_REQUEST, (WPARAM)newString, NULL);
	}
	else {
		if (defaultOrchestrator->info.state.Compare(_T("Open")) == 0) {
			qbOpenEvent = strMsg;
			
			//if (defaultConductor.orchestrator.info.shouldUpdate == true)
			//	defaultConductor.orchestrator.spawnCanary.CheckForUpdates();
			defaultOrchestrator->spawnCanary.StartThread();
		}

		if (defaultOrchestrator->info.state.Compare(_T("Close")) == 0) {
			SetEvent(defaultOrchestrator->goOfflineSignal);
			defaultOrchestrator->spawnCanary.StopBrainProcess();
			defaultOrchestrator->StopConcert();

			PostQuitMessage(0);
			//PostMessage(defaultConductor.orchestrator.request.mainDlg->m_hWnd, WM_CLOSE, NULL, NULL);
		}
	}

	return 1;
}
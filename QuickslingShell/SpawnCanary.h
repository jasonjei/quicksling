#pragma once

#include <atlmisc.h>
#include "Constants.h"
#include "INet.h"
#include <sstream>
#include <iostream>
#include <strsafe.h>
#include <exception>
#include "stdafx.h"

class Orchestrator;

class SpawnCanary {
public:
	SpawnCanary(void);
	~SpawnCanary(void);
	DWORD StartThread();
	BOOL StartBrainProcess();
	void StopBrainProcess();
	void SpoofPipeShutdown();
	int CheckForUpdates();
	int DownloadFile(MapObject serverCRCMap, std::string fileName);
	CString DownloadCRCMap();

	static DWORD WINAPI RunThread(LPVOID lpData);

	DWORD threadID;
	Orchestrator *orchestrator;
	HANDLE threadHandle;
	PROCESS_INFORMATION brainProcessInfo;
};
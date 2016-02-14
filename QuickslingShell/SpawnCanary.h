#pragma once

#include <atlmisc.h>
#include "Constants.h"
#include "INet.h"
#include <sstream>
#include <iostream>
#include <strsafe.h>
#include <exception>
#include "stdafx.h"
#include "Crc32Static.h"
#include "inifile.h"

class Orchestrator;

class SpawnCanary {
public:
	SpawnCanary(void);
	~SpawnCanary(void);
	DWORD StartThread();
	BOOL StartBrainProcess();
	void StopBrainProcess();
	int CheckForUpdates();

	static DWORD WINAPI RunThread(LPVOID lpData);

	DWORD threadID;
	Orchestrator *orchestrator;
	HANDLE threadHandle;
	PROCESS_INFORMATION brainProcessInfo;
};
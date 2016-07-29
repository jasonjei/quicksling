#include "stdafx.h"
#include "NetAware.h"
#include "Conductor.h"
#pragma comment(lib, "ws2_32.lib")

extern Conductor defaultConductor;

NetAware::NetAware()
{
}


NetAware::~NetAware()
{
}

DWORD NetAware::StartThread() {
	// _LongPollRunData *pData = new _LongPollRunData;
	// pData->longPollInstance = this;

	threadHandle = ::CreateThread(NULL, 0, NetAware::RunThread, NULL, NULL, &threadID);
	CloseHandle(threadHandle);
	return threadID;
}


DWORD WINAPI NetAware::RunThread(LPVOID lpData)  {
	WSAData wsa_data = { 0 };
	WSAStartup(MAKEWORD(2, 0), &wsa_data);

	while (WaitForSingleObject(defaultConductor.orchestrator.longPoll.goOfflineSignal, 0) != WAIT_OBJECT_0) {
		wait_for_connection_type_change_with_winsock();
		Sleep(2000);
		SetEvent(defaultConductor.orchestrator.longPoll.tryAgainSignal);
	}

	WSACleanup();
	return 0;
}

void NetAware::wait_for_connection_type_change_with_winsock() {
	WSAQUERYSET querySet = { 0 };
	querySet.dwSize = sizeof(WSAQUERYSET);
	querySet.dwNameSpace = NS_NLA;

	HANDLE LookupHandle = NULL;
	WSALookupServiceBegin(&querySet, LUP_RETURN_ALL, &LookupHandle);
	DWORD BytesReturned = 0;
	WSANSPIoctl(LookupHandle, SIO_NSP_NOTIFY_CHANGE, NULL, 0, NULL, 0, &BytesReturned, NULL);
	WSALookupServiceEnd(LookupHandle);
}
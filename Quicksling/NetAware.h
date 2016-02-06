#pragma once
class NetAware
{
public:
	DWORD threadID;
	HANDLE threadHandle;

	NetAware();
	~NetAware();
	DWORD StartThread();
	static DWORD WINAPI RunThread(LPVOID lpData);
	static void wait_for_connection_type_change_with_winsock();
};


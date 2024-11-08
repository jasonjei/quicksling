#pragma once

#include <atlmisc.h>
#include "Constants.h"
#include "INet.h"
#include "simple_app.h"
#include "simple_handler.h"

class Orchestrator;

class BrowserHandler {
public:
	BrowserHandler(void);
	~BrowserHandler(void);
	DWORD StartThread();

	static DWORD WINAPI RunThread(LPVOID lpData);
	static HWND CreateMessageWindow(HINSTANCE hInstance);
	static LRESULT CALLBACK BrowserHandler::MessageWndProc(HWND hWnd, UINT message, WPARAM wParam,
		LPARAM lParam);
	void StartBrowser();
	void LoadStatusPage();
	void Shutdown();

	DWORD threadID;
	Orchestrator *orchestrator;
	HANDLE signal;
	HANDLE browserOpenSignal;
	HANDLE threadHandle;
	HWND mainDlgHwnd;

	CefMainArgs main_args;
	CefRefPtr<SimpleApp> app;
	CefSettings settings;
	CefRefPtr<SimpleHandler> simpleHandler;
	HINSTANCE hInstance;

private:
};
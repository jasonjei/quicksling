#pragma once

#include "stdafx.h"
#include "Orchestrator.h"
#include "BrowserHandler.h"

extern Orchestrator* defaultOrchestrator;

BrowserHandler::BrowserHandler(void) {
	signal = CreateEvent(NULL, TRUE, FALSE, NULL);
	browserOpenSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
}

BrowserHandler::~BrowserHandler(void) {
	CloseHandle(signal);
	CloseHandle(browserOpenSignal);
	delete app;
	delete simpleHandler;
	// Shut down CEF.
}

DWORD BrowserHandler::StartThread() {
	settings.multi_threaded_message_loop = true;
	threadHandle = ::CreateThread(NULL, 0, BrowserHandler::RunThread, NULL, NULL, &threadID);

	// Optional implementation of the CefApp interface.

	/*
	// Execute the sub-process logic, if any. This will either return immediately for the browser
	// process or block until the sub-process should exit.
	int exit_code = CefExecuteProcess(main_args, app.get());
	if (exit_code >= 0) {
	MessageBox(NULL, _T("Got exit code"), _T("Got exit code"), MB_OK);
	// TODO - handle. The sub-process terminated
	}
	// Initialize CEF in the main process.
	CefInitialize(main_args, settings, app.get());
	scheme_test::InitTest();
	*/
	return threadID;
}

DWORD WINAPI BrowserHandler::RunThread(LPVOID lpData) {
	BrowserHandler* browser = &defaultOrchestrator->browser;
	//browser->StartBrowser();
	browser->app = new SimpleApp;

	// Execute the sub-process logic, if any. This will either return immediately for the browser
	// process or block until the sub-process should exit.
	int exit_code = CefExecuteProcess(browser->main_args, browser->app.get(), NULL);
	if (exit_code >= 0) {
		MessageBox(NULL, _T("Got exit code"), _T("Got exit code"), MB_OK);
		// TODO - handle. The sub-process terminated
	}

	browser->settings.no_sandbox = true;

	// Initialize CEF in the main process.
	CefInitialize(browser->main_args, browser->settings, browser->app.get(), NULL);
	// scheme_test::InitTest();

	// Run the CEF message loop. This will block until CefQuitMessageLoop() is called.
	CefRunMessageLoop();
	/* 
	// Create a hidden window for message processing.
	HWND hMessageWnd = NULL;
	hMessageWnd = CreateMessageWindow(browser->hInstance);
	// ASSERT(hMessageWnd);

	MSG msg;

	// Run the application message loop.
	while (GetMessage(&msg, NULL, 0, 0)) {
		// if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		//  }
	}

	DestroyWindow(hMessageWnd);
	hMessageWnd = NULL;

	return static_cast<int>(msg.wParam);
	*/
	return false;
}

void BrowserHandler::Shutdown() {
	CefQuitMessageLoop();
}

void BrowserHandler::StartBrowser() {
	this->app = new SimpleApp;
	// Execute the sub-process logic, if any. This will either return immediately for the browser
	// process or block until the sub-process should exit.
	int exit_code = CefExecuteProcess(this->main_args, this->app.get(), NULL);
	if (exit_code >= 0) {
		MessageBox(NULL, _T("Got exit code"), _T("Got exit code"), MB_OK);
		// TODO - handle. The sub-process terminated
	}

	// Initialize CEF in the main process.
	CefInitialize(this->main_args, this->settings, this->app.get(), NULL);
	// scheme_test::InitTest();

	// Run the CEF message loop. This will block until CefQuitMessageLoop() is called.
	CefRunMessageLoop();

}

void BrowserHandler::LoadStatusPage() {
	// TODO - should not be dev URL
	CString url = "https://app.levion.dev/qb_client/status?company_tag=" + defaultOrchestrator->qbInfo.companyTag;
	// this->browser->GetMainFrame()->LoadURL(std::string(CW2A(url, CP_UTF8)));
}

HWND BrowserHandler::CreateMessageWindow(HINSTANCE hInstance) {
	static const wchar_t kWndClass[] = L"ClientMessageWindow";

	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = MessageWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = kWndClass;
	RegisterClassEx(&wc);

	return CreateWindow(kWndClass, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0,
		hInstance, 0);
}

LRESULT CALLBACK BrowserHandler::MessageWndProc(HWND hWnd, UINT message, WPARAM wParam,
	LPARAM lParam) {
	//switch (message) {
	//  case WM_COMMAND: {
	//    int wmId = LOWORD(wParam);
	//    switch (wmId) {
	//      case ID_QUIT:
	//        PostQuitMessage(0);
	//        return 0;
	//    }
	//  }
	//}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
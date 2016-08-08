
#include "stdafx.h"
#include "strutil.h"
#include "qbXMLRPWrapper.h"
#include "Constants.h"
#include "INet.h"
#include "inifile.h"
#include <string>
#include <sstream>

int DisabledInstaller(CString version);

UINT __stdcall CustomAction1(MSIHANDLE hInstall)
{
	HRESULT hr = S_OK;
	UINT er = ERROR_SUCCESS;
	
	qbXMLRPWrapper qb;

	hr = WcaInitialize(hInstall, "CustomAction1");
	ExitOnFailure(hr, "Failed to initialize");
	
	WcaLog(LOGMSG_STANDARD, "Initialized.");

	// TODO: Add your custom action code here.
	hr = WcaSetProperty(TEXT("TESTQBINSTALLED"), TEXT("YES"));
	
	if (qb.TestQBInstalled() == false) {
		hr = WcaSetProperty(TEXT("TESTQBINSTALLED"), TEXT("NO"));
	}

	ExitOnFailure(hr, "failed to set TESTQBINSTALLED");

LExit:
	er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
	return WcaFinalize(er);
}

UINT __stdcall CustomAction2(MSIHANDLE hInstall)
{
	HRESULT hr = S_OK;
	UINT er = ERROR_SUCCESS;

	LPWSTR versionStr = NULL;

	hr = WcaInitialize(hInstall, "CustomAction2");
	ExitOnFailure(hr, "Failed to initialize");

	WcaLog(LOGMSG_STANDARD, "Initialized.");

	// TODO: Add your custom action code here.
	hr = WcaSetProperty(TEXT("ALLOWINSTALL"), TEXT("YES"));

	WcaGetProperty(L"ProductVersion", &versionStr);

	if (DisabledInstaller(versionStr)) {
		hr = WcaSetProperty(TEXT("ALLOWINSTALL"), TEXT("NO"));
	}

	ExitOnFailure(hr, "failed to set TESTQBINSTALLED");

LExit:
	ReleaseStr(versionStr);

	er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
	return WcaFinalize(er);
}

// DllMain - Initialize and cleanup WiX custom action utils.
extern "C" BOOL WINAPI DllMain(
	__in HINSTANCE hInst,
	__in ULONG ulReason,
	__in LPVOID
	)
{
	switch(ulReason)
	{
	case DLL_PROCESS_ATTACH:
		WcaGlobalInitialize(hInst);
		break;

	case DLL_PROCESS_DETACH:
		WcaGlobalFinalize();
		break;
	}

	return TRUE;
}

int DisabledInstaller(CString version) {
	CIniFile configIni;
	CString sURL = "http://app.quicklet.dev/client/settings?client=QuickslingInstaller&version=" + version;

	CInternetSession Session(_T("Quicksling Downloader"));
	WORD timeout = 10000;
	DeleteUrlCacheEntry(sURL);

	bool disable = false;

	Session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, timeout);
	Session.SetOption(INTERNET_OPTION_SEND_TIMEOUT, timeout);

	CHttpFile* cHttpFile = NULL;
	int fail = 0;

	try {
		cHttpFile = new CHttpFile(Session, sURL, NULL, 0, INTERNET_FLAG_DONT_CACHE);
	}

	catch (CInternetException& e) {
		fail = 1;
		delete cHttpFile;
		return false;
	}

	if (!fail) {
		WTL::CString pageSource;
		CIniFile configIni;

		CIniSectionW* settingsSec;

		UINT bytes = (UINT)cHttpFile->GetLength();

		char tChars[2048 + 1];
		int bytesRead;

		while ((bytesRead = cHttpFile->Read((LPVOID)tChars, 2048)) != 0) {
			tChars[bytesRead] = '\0';
			pageSource += CA2W((LPCSTR)tChars, CP_UTF8);
		}

		delete cHttpFile;

		std::wistringstream downloadManifestStream((LPCTSTR)pageSource);
		configIni.Load(downloadManifestStream, false);
		settingsSec = configIni.GetSection(_T("settings"));

		if (settingsSec != NULL) {

			CIniKeyW* disableKey = settingsSec->GetKey(_T("disable"));
			if (disableKey != NULL) {
				CString disableVal = disableKey->GetValue().c_str();
				disableVal.TrimLeft();
				disableVal.TrimRight();
				disableVal.MakeLower();

				if (disableVal == "true" || disableVal == "1") {
					disable = true;
				}
			}
		}
	}
	if (disable == true)
		return true;
	return false;
}
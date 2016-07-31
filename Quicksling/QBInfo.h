#pragma once
#include <atlapp.h>
#include <atlmisc.h>
#include "qbXMLRPWrapper.h"
#import <msxml3.dll> named_guids raw_interfaces_only
#include <ShlObj.h>
#include "Constants.h"
#include "LevionMisc.h"
#include "INet.h"
#include "inifile.h"
#include <atlconv.h>
#include <mutex>

#pragma comment(lib, "userenv.lib")

void __stdcall CallMaster(
	HINTERNET hInternet,
	DWORD_PTR dwContext,
	DWORD dwInternetStatus,
	LPVOID lpvStatusInformation,
	DWORD dwStatusInformationLength);

class Orchestrator;
extern std::mutex mutexQBInfo;

class QBInfo {
public:
	CString companyName;
	CString country;
	CString productName;
	CString companyTag;
	CString qbxmlVersions;
	CString subdomain;
	CString authToken;
	CString version;
	CString clientGuid;
	CString state;
	CString productInvoice;
	CString packingSlip;
	Orchestrator *orchestrator;
	HANDLE readyForLongPollSignal;
	qbXMLRPWrapper *persistentQBXMLWrapper;
	int sequence;
	bool processedQBRequest;
	bool hasRun;

	QBInfo(void) : persistentQBXMLWrapper(NULL) {
		this->readyForLongPollSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
		// this->version = LEVION_CLIENT_VER;
		GetAndSetCoreVersion();
		this->hasRun = false;
		this->sequence = 0;
	}

	~QBInfo(void) {
		EndSession();
	}

	void GetAndSetCoreVersion() {
		CString productName;
		CString productVersion;
		GetProductAndVersion(productName, productVersion);
		this->version = productVersion;
#ifdef DEBUG
		this->version += _T("D");
#endif
	}

	bool GetProductAndVersion(CString & strProductName, CString & strProductVersion) {
		// get the filename of the executable containing the version resource
		TCHAR szFilename[MAX_PATH + 1] = { 0 };
		if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0)
		{
			// TRACE("GetModuleFileName failed with error %d\n", GetLastError());
			return false;
		}

		// allocate a block of memory for the version info
		DWORD dummy;
		DWORD dwSize = GetFileVersionInfoSize(szFilename, &dummy);
		if (dwSize == 0)
		{
			// TRACE("GetFileVersionInfoSize failed with error %d\n", GetLastError());
			return false;
		}
		std::vector<BYTE> data(dwSize);

		// load the version info
		if (!GetFileVersionInfo(szFilename, NULL, dwSize, &data[0]))
		{
			// TRACE("GetFileVersionInfo failed with error %d\n", GetLastError());
			return false;
		}

		// get the name and version strings
		LPVOID pvProductName = NULL;
		unsigned int iProductNameLen = 0;
		LPVOID pvProductVersion = NULL;
		unsigned int iProductVersionLen = 0;

		// replace "040904e4" with the language ID of your resources
		UINT                uiVerLen = 0;
		VS_FIXEDFILEINFO*   pFixedInfo = 0;     // pointer to fixed file info structure
		// get the fixed file info (language-independend) 
		if (VerQueryValue(&data[0], TEXT("\\"), (void**)&pFixedInfo, (UINT *)&uiVerLen) == 0)
		{
			return false;
		}

		strProductVersion.Format(_T("%u.%u.%u.%u"),
			HIWORD(pFixedInfo->dwProductVersionMS),
			LOWORD(pFixedInfo->dwProductVersionMS),
			HIWORD(pFixedInfo->dwProductVersionLS),
			LOWORD(pFixedInfo->dwProductVersionLS));

		// strProductName.SetString((LPCSTR)pvProductName, iProductNameLen);
		// strProductVersion.SetString((LPCSTR)pvProductVersion, iProductVersionLen);

		return true;
	}

	CString GetLevionUserAppDir() {
		TCHAR szPath[MAX_PATH];
		SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, szPath);
		PathAppend(szPath, _T("Quicksling"));
		return CString(szPath);
	}

	CString GetLevionUserAppDir(CString File) {
		TCHAR szPath[MAX_PATH];
		SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, szPath);
		PathAppend(szPath, _T("Quicksling"));
		PathAppend(szPath, File);
		return CString(szPath);
	}

	int CreateLevionAppDir() {
		int success = CreateDirectory(GetLevionUserAppDir(), NULL);
		if (success)
			return 1;
		else {
			return PathIsDirectory(GetLevionUserAppDir());
		}
	}

	CString GetLevionAppDataPath() {
		CString path;

		TCHAR szPath[MAX_PATH];

		if (SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, szPath))
			PathAppend(szPath, TEXT("New Doc.txt"));

		return path;
	}

	void StartSession() {
		if (persistentQBXMLWrapper == NULL) {
			persistentQBXMLWrapper = new qbXMLRPWrapper;
			persistentQBXMLWrapper->OpenCompanyFile(_T(""));
		}
	}

	void EndSession() {
		if (persistentQBXMLWrapper != NULL) {
			delete persistentQBXMLWrapper;
			persistentQBXMLWrapper = NULL;
		}
	}

	CString ProcessQBXMLRequest(CString QBXML) {
		qbXMLRPWrapper* session;
		bool isPersistent = false;

		CString errorMsg;

		if (this->persistentQBXMLWrapper != NULL) {
			session = this->persistentQBXMLWrapper;
			isPersistent = true;
		}
		else {
			session = new qbXMLRPWrapper;
			session->OpenCompanyFile(_T(""));

			if (session->GetHasError()) {
				errorMsg.Format(_T("<QBError>%ls</QBError>"), session->GetErrorMsg().c_str());
				delete session;
				return errorMsg;
			}
		}

		CString result = session->ProcessRequest((LPCTSTR) QBXML).c_str();


		if (session->GetHasError()) {
			errorMsg.Format(_T("<QBError>%ls</QBError>"), session->GetErrorMsg().c_str());
			result = errorMsg;
		}

		if (! isPersistent) {
			delete session;
		}



		return result;
	}

	void EnsureYamlKeysExist() {
		// Ensures that these map keys exist - in the case where we don't have a YML file to read - OR -
		// user is playing fire with YML
		// CreateMapNode(map.mapPtr, "client_guid");
		// CreateMapNode(map.mapPtr, "companies");
	}

	int RegenerateGuid() {
		this->authToken = GUIDgen();
		LoadConfigYaml();
	}

	// TODO - Refactor to not keep yaml map loaded in memory. Load the file, get settings from file, and save settings to file.
	int LoadConfigYaml() {
		CreateLevionAppDir();

		FILE *fh;

		CIniFile ini;
		CIniSectionW* authSec;
		CIniSectionW* sequenceSec;
		CIniSectionW* devSec;

		ini.Load((LPCTSTR) GetLevionUserAppDir("config.ini"));
		
		if ((authSec = ini.GetSection(_T("Auth"))) == NULL) 
			authSec = ini.AddSection(_T("Auth"));

		/*
		if ((sequenceSec = ini.GetSection(_T("Sequence"))) == NULL)
			sequenceSec = ini.AddSection(_T("Sequence"));
		*/ 
		
		devSec = ini.GetSection(_T("Development"));

		CString uniqueId;
		uniqueId = this->productInvoice + _T(",") + this->packingSlip;

		CIniKeyW *authKey = authSec->GetKey((LPCTSTR) uniqueId);

		if (authKey == NULL) {
			authKey = authSec->AddKey((LPCTSTR)uniqueId);
			authKey->SetValue((LPCTSTR) GUIDgen());
		}

		if (this->hasRun) {
			authKey->SetValue((LPCTSTR) this->authToken);
		}
		else {
			this->authToken = authKey->GetValue().c_str();
			this->authToken.TrimLeft();
			this->authToken.TrimRight();
		}

		if (devSec != NULL) {
			CIniKeyW* appUrl = devSec->GetKey(_T("app_url"));
			CIniKeyW* commUrl = devSec->GetKey(_T("communicate_url"));

			if (appUrl != NULL)
				URLS::APP_SERVER = appUrl->GetValue().c_str();

			if (commUrl != NULL)
				URLS::GOLIATH_SERVER = commUrl->GetValue().c_str();
		}
		/*

		CIniKeyW *sequenceKey = sequenceSec->GetKey((LPCTSTR)uniqueId);

		if (sequenceKey == NULL) {
			sequenceKey = sequenceSec->AddKey((LPCTSTR)uniqueId);
			sequenceKey->SetValue(std::to_wstring(this->sequence));
		}

		if (this->hasRun) {
			sequenceKey->SetValue(std::to_wstring(this->sequence));
		}
		else {
			this->sequence = std::stoi(sequenceKey->GetValue().c_str());
		}
		*/

		ini.Save((LPCTSTR)GetLevionUserAppDir("config.ini"));
		hasRun = true;

		/* if (authKey == NULL) {
			this->authToken = 
		} */

		// _wfopen_s(&fh, (LPCTSTR) GetLevionUserAppDir("config.yml"), _T("r"));
		/*
		map = MapObject::processYaml(fh);
		EnsureYamlKeysExist();

		if (map._type == MapObject::MAP_OBJ_FAILED) {
			return 0;
		}

		if (map.mapPtr->map["client_guid"].value == "") {
			GUID gidReference;
			HRESULT hCreateGuid = CoCreateGuid( &gidReference );
			CComBSTR tmp(gidReference);
			CString csGuid(tmp);
			this->clientGuid = tmp;
			this->clientGuid.Remove('{');
			this->clientGuid.Remove('}');
		} else
			this->clientGuid = map.mapPtr->map["client_guid"].value.c_str();
		
		if (!map.mapPtr->map["app_server"].value.empty()) {
			URLS::APP_SERVER = CString(map.mapPtr->map["app_server"].value.c_str());
		}
		if (!map.mapPtr->map["goliath_server"].value.empty()) {
			URLS::GOLIATH_SERVER = CString(map.mapPtr->map["goliath_server"].value.c_str());
		}
		if (!map.mapPtr->map["download_server"].value.empty()) {
			URLS::DOWNLOAD_SERVER = CString(map.mapPtr->map["download_server"].value.c_str());
		}
		if (!map.mapPtr->map["help_server"].value.empty()) {
			URLS::HELP_SERVER = CString(map.mapPtr->map["help_server"].value.c_str());
		}
		*/
		return 1;
	}

	int SaveConfigYaml() {
		/* 
		if (!this->companyTag.IsEmpty()) {
			CreateMapNode(map.mapPtr->map["companies"].mapPtr, std::string(CW2A(this->companyTag, CP_UTF8)));
			map.mapPtr->map["companies"].mapPtr->map[std::string(CW2A(this->companyTag, CP_UTF8))].value = CW2A(this->authToken, CP_UTF8);
		}

		map.mapPtr->map["client_guid"].value = std::string(CW2A(this->clientGuid, CP_UTF8));

		map.exportYaml((LPCTSTR) GetLevionUserAppDir("config.yml"));
		*/
		return 1;
	}

	int RegisterConnector();

	int SetState(CString str) {
		MSXML2::IXMLDOMDocument* outputXMLDoc = InstantiateXMLDocWithString(str);
		long isUIEvent = 0;

		if (outputXMLDoc) {
			MSXML2::IXMLDOMNodeList *nodeList;

			outputXMLDoc->getElementsByTagName(_T("CompanyFileEventOperation"), &nodeList);
			nodeList->get_length(&isUIEvent);

			if (isUIEvent) 
			{
				this->state = GetValueFromNodeString(_T("CompanyFileEventOperation"), outputXMLDoc);

				if (state.Compare(_T("Open")) == 0) {
					GetInfoFromQB();
				}
				else if (state.Compare(_T("Close")) == 0) {
					//MessageBox(NULL, _T("Exiting..."), _T("Exited QB"), MB_OK);
				}
				else
					MessageBox(NULL, _T("COM msg received is neither Open nor Close"), _T("Oops"), MB_OK);
			}
			nodeList->Release();
			outputXMLDoc->Release();
		}
		return isUIEvent;
	}

	int TestQBWorks() {
		qbXMLRPWrapper qb;
		qb.OpenCompanyFile(_T(""));
		if (qb.GetHasError()) {
			MessageBox(NULL, qb.GetErrorMsg().c_str(), _T("Error Starting SimpleQuickletClient"), MB_OK);
			return 0;
		}
		return 1;
	}

	int Reset();

	int GetInfoFromQB();

	int SetupQuickslingWithQBData() {
		qbXMLRPWrapper qb;

		qb.OpenCompanyFile(_T(""));
		CString result = qb.ProcessRequest(std::wstring(GET_COMPANY_TAG)).c_str();
		return 1;
	}

	int SetCompanyInfo(CString str) {
		MSXML2::IXMLDOMDocument* outputXMLDoc = InstantiateXMLDocWithString(str);

		if (outputXMLDoc) {
			this->companyName = GetValueFromNodeString(_T("CompanyName"), outputXMLDoc);
			this->companyTag = FindCompanyTag(outputXMLDoc);
			
			outputXMLDoc->Release();
			return 1;
		}

		return 0;
	}

	int SetCompanyTemplateInfo(CString str) {
		MSXML2::IXMLDOMDocument* outputXMLDoc = InstantiateXMLDocWithString(str);

		if (outputXMLDoc) {
			MSXML2::IXMLDOMNodeList *nodeList;
			long size;

			HRESULT hr = outputXMLDoc->getElementsByTagName(CComBSTR(_T("TemplateRet")), &nodeList);

			if (hr == S_OK) {
				nodeList->get_length(&size);

				for (int i = 0; i < size; i++) {
					MSXML2::IXMLDOMNode *node;
					nodeList->get_item(i, &node);

					if (GetValueFromNode("Name", node).Compare(_T("Intuit Packing Slip")) == 0) {
						packingSlip = GetValueFromNode("ListID", node);
					}
					else if (GetValueFromNode("Name", node).Compare(_T("Intuit Product Invoice")) == 0) {
						productInvoice = GetValueFromNode("ListID", node);
					}
					node->Release();
				}
				nodeList->Release();
			}
			outputXMLDoc->Release();
			return 1;
		}
		return 0;
	}

	int SetQBInfo();

	void RemoveTagFromYaml() {
		// map.mapPtr->map["companies"].mapPtr->map.erase(std::string(CW2A(this->companyTag, CP_UTF8)));
		SaveConfigYaml();
	}

	int LoadAuthToken() {
		if (this->companyTag == "") {
			return RegisterConnector();
		} else {
			// this->authToken = map.mapPtr->map["companies"].mapPtr->map[std::string(CW2A(this->companyTag, CP_UTF8))].value.c_str();
			if (this->authToken == "")
				return RegisterConnector();
		}

		return 1;
	}

	CString GetValueFromNodeString(CString value, MSXML2::IXMLDOMDocument *outputXMLDoc) {
		MSXML2::IXMLDOMNodeList *nodeList;
		long size;

		HRESULT hr = outputXMLDoc->getElementsByTagName(CComBSTR(value), &nodeList);

		if (hr == S_OK) {
			CString txtReturn;
			MSXML2::IXMLDOMNode *node, *childNode;

			nodeList->get_length(&size);

			if (size) {
				CComVariant content;

				nodeList->get_item(0, &node);
				node->get_firstChild(&childNode);
				childNode->get_nodeValue(&content);

				txtReturn = content.bstrVal;
				node->Release();
				childNode->Release();
			}
			nodeList->Release();

			return txtReturn;
		}
		else 
			return CString(_T(""));
	}

	CString GetValueFromNode(CString value, MSXML2::IXMLDOMNode* node) {
		MSXML2::IXMLDOMNode *childNode;

		HRESULT hr = node->selectSingleNode(CComBSTR(value), &childNode);

		if (hr == S_OK) {
			CString txtReturn;
			CComVariant content;
			MSXML2::IXMLDOMNode *childChildNode;

			if (childNode->get_firstChild(&childChildNode) == S_OK) {
				childChildNode->get_nodeValue(&content);
				txtReturn = content.bstrVal;
				childChildNode->Release();
			}

			childNode->Release();
			return txtReturn;
		}
		else 
			return CString(_T(""));
	}

	CString FindCompanyTag(MSXML2::IXMLDOMDocument *outputXMLDoc) {
		MSXML2::IXMLDOMNodeList *nodeList;
		long size;

		HRESULT hr = outputXMLDoc->getElementsByTagName(CComBSTR(_T("DataExtRet")), &nodeList);

		if (hr == S_OK) {
			CString txtReturn;
			nodeList->get_length(&size);

			for (int i=0; i < size; i++) {
				MSXML2::IXMLDOMNode *node;
				nodeList->get_item(i, &node);

				if (GetValueFromNode("DataExtName", node).Compare(_T("CompanyUUID")) == 0) {
					txtReturn = GetValueFromNode("DataExtValue", node);
				}

				node->Release();
			}
			nodeList->Release();
			return txtReturn;
		}
		else 
			return CString(_T(""));
	}

	MSXML2::IXMLDOMDocument* InstantiateXMLDoc() {
		MSXML2::IXMLDOMDocument *outputXMLDoc = NULL;

		HRESULT hr = CoCreateInstance(__uuidof(MSXML2::DOMDocument),
			NULL,
			CLSCTX_INPROC_SERVER,
			__uuidof(MSXML2::IXMLDOMDocument),
			reinterpret_cast<void**>(&outputXMLDoc));

		if (hr == S_OK) 
			return outputXMLDoc;
		else
			return NULL;
	}

	MSXML2::IXMLDOMDocument* InstantiateXMLDocWithString(CString data) {
		MSXML2::IXMLDOMDocument *outputXMLDoc = NULL;
		CComBSTR cbstrXML(data);
		VARIANT_BOOL vBool;

		HRESULT hr = CoCreateInstance(__uuidof(MSXML2::DOMDocument),
			NULL,
			CLSCTX_INPROC_SERVER,
			__uuidof(MSXML2::IXMLDOMDocument),
			reinterpret_cast<void**>(&outputXMLDoc));

		if (hr == S_OK) {
			hr = outputXMLDoc->loadXML(cbstrXML.m_str, &vBool);
			return outputXMLDoc;
		}
		else {
			return NULL;
		}
	}

	CString GUIDgen()
	{
		GUID guid;
		CoCreateGuid(&guid);

		BYTE * str;
		UuidToString((UUID*)&guid, (RPC_WSTR*) &str);

		CString unique((LPTSTR) str);

		RpcStringFree((RPC_WSTR*) &str);

		// unique.Replace(_T("-"), _T("_"));

		return unique;
	}

	void LaunchBrowser(CString url);

	CString GetUniqueID() {
		CString uniqueId;
		uniqueId = this->productInvoice + _T(",") + this->packingSlip;
		return uniqueId;
	}
};
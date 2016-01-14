#pragma once
#include <atlapp.h>
#include <atlmisc.h>
#include "qbXMLRPWrapper.h"
#import <msxml3.dll> named_guids raw_interfaces_only
#include <ShlObj.h>
#include "Constants.h"
#include "LevionMisc.h"
#include "INet.h"

#pragma comment(lib, "userenv.lib")

void __stdcall CallMaster(
	HINTERNET hInternet,
	DWORD_PTR dwContext,
	DWORD dwInternetStatus,
	LPVOID lpvStatusInformation,
	DWORD dwStatusInformationLength);

class Orchestrator;

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
	Orchestrator *orchestrator;
	HANDLE readyForLongPollSignal;
	qbXMLRPWrapper *persistentQBXMLWrapper;
	bool processedQBRequest;

	QBInfo(void) : persistentQBXMLWrapper(NULL) {
		this->readyForLongPollSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
		this->version = LEVION_CLIENT_VER;
	}

	~QBInfo(void) {
		EndSession();
	}

	CString GetLevionUserAppDir() {
		TCHAR szPath[MAX_PATH];
		SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, szPath);
		PathAppend(szPath, _T("Levion"));
		return CString(szPath);
	}

	CString GetLevionUserAppDir(CString File) {
		TCHAR szPath[MAX_PATH];
		SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, szPath);
		PathAppend(szPath, _T("Levion"));
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

	// TODO - Refactor to not keep yaml map loaded in memory. Load the file, get settings from file, and save settings to file.
	int LoadConfigYaml() {
		CreateLevionAppDir();

		FILE *fh;

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

	int GetInfoFromQB() {
		qbXMLRPWrapper qb;

		qb.OpenCompanyFile(_T(""));
		CString result = qb.ProcessRequest(std::wstring(GET_COMPANY_TAG)).c_str();

		SetCompanyInfo(result);
		SetQBInfo();

		LoadConfigYaml();
		LoadAuthToken();
		SaveConfigYaml();

		SetEvent(this->readyForLongPollSignal);

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

	int SetQBInfo() {
		qbXMLRPWrapper qb;
		qb.OpenCompanyFile(_T(""));

		this->qbxmlVersions = qb.GetVersions();
		CString versionResult = qb.ProcessRequest(std::wstring(_T("<?xml version=\"1.0\"?><?qbxml version=\"8.0\"?><QBXML><QBXMLMsgsRq onError=\"stopOnError\"><HostQueryRq></HostQueryRq></QBXMLMsgsRq></QBXML>"))).c_str();
		MSXML2::IXMLDOMDocument* outputXMLDoc = InstantiateXMLDocWithString(versionResult);

		if (outputXMLDoc) {
			this->productName = GetValueFromNodeString(_T("ProductName"), outputXMLDoc);
			this->country = GetValueFromNodeString(_T("Country"), outputXMLDoc);

			outputXMLDoc->Release();
			return 1;
		}

		return 0;
	}

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

};
#pragma once
#include <atlapp.h>
#include <atlmisc.h>
#include "qbXMLRPWrapper.h"
#import <msxml3.dll> named_guids raw_interfaces_only
#include <ShlObj.h>

#pragma comment(lib, "userenv.lib")

class Info {
public:
	CString state;
	BOOL shouldUpdate;

	Info(void) {
		// this->version = LEVION_CLIENT_VER;
		shouldUpdate = true;
	}

	~Info(void) {
	}

	CString GetQuickslingUserAppDir() {
		TCHAR szPath[MAX_PATH];
		SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, szPath);
		PathAppend(szPath, _T("Quicksling"));
		return CString(szPath);
	}

	CString GetQuickslingUserAppDir(CString File) {
		TCHAR szPath[MAX_PATH];
		SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, szPath);
		PathAppend(szPath, _T("Quicksling"));
		PathAppend(szPath, File);
		return CString(szPath);
	}

	int CreateQuickslingAppDir() {
		int success = CreateDirectory(GetQuickslingUserAppDir(), NULL);
		if (success)
			return 1;
		else {
			return PathIsDirectory(GetQuickslingUserAppDir());
		}
	}

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
					//GetInfoFromQB();
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
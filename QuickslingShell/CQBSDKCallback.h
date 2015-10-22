#if !defined(AFX_QBSDKCALLBACK_H__5AE97D5E_ABB1_4B8A_9DBA_4B8B46308ADE__INCLUDED_)
#define AFX_QBSDKCALLBACK_H__5AE97D5E_ABB1_4B8A_9DBA_4B8B46308ADE__INCLUDED_

#import  "sdkevent.dll" no_namespace named_guids
#include "QuickslingShell.h"
#include "atlmisc.h"
#include "atlcom.h"
// #include "Orchestrator.h"

// extern Conductor defaultConductor;
CString firstMsg;

class ATL_NO_VTABLE CQBSDKCallback : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CQBSDKCallback, &CLSID_QBSDKCallback>,
    public IQBEventCallback {
public:
	CQBSDKCallback() {}

BEGIN_COM_MAP(CQBSDKCallback)
	COM_INTERFACE_ENTRY(IQBEventCallback)
END_COM_MAP()
DECLARE_PROTECT_FINAL_CONSTRUCT()

DECLARE_REGISTRY_RESOURCEID(IDR_QUICKSLINGSHELL)

	STDMETHOD(inform) (/*[in]*/ BSTR eventXML) {
		// MessageBox(NULL, eventXML, _T("TEST---------FROM SHELL"), MB_OK);
		if (firstMsg.Compare(_T("")) == 0) 
			firstMsg = eventXML;

		CString toSendMsg(eventXML);
		// defaultConductor.orchestrator.eventHandler.Inform(toSendMsg);
		ATLTRACE2(atlTraceUI, 0, TEXT("Received event from QB, %s\n"), toSendMsg);
		return 1;
	};

    HRESULT __stdcall raw_inform (/*[in]*/ BSTR eventXML )  { return inform(eventXML); };
};

#endif // !defined(AFX_QBSDKCALLBACK_H__5AE97D5E_ABB1_4B8A_9DBA_4B8B46308ADE__INCLUDED_)
#pragma once

#include <atlmisc.h>

static UINT LEVION_REQUEST = ::RegisterWindowMessage(_T("LEVION_REQUEST"));
static UINT LEVION_RESPONSE = ::RegisterWindowMessage(_T("LEVION_RESPONSE"));
static UINT LEVION_END_SESSION = ::RegisterWindowMessage(_T("LEVION_END_SESSION"));
static UINT LEVION_MESSAGE_BOX = ::RegisterWindowMessage(_T("LEVION_MESSAGE_BOX"));
static UINT LEVION_EVENT = ::RegisterWindowMessage(_T("LEVION_EVENT"));
static UINT LEVION_TRAYICON_MSG = ::RegisterWindowMessage(_T("LEVION_TRAYICON_MSG"));
static UINT LEVION_SHUTDOWN = ::RegisterWindowMessage(_T("LEVION_SHUTDOWN"));
static UINT PIPE_REQUEST = ::RegisterWindowMessage(_T("PIPE_REQUEST"));
static UINT QUICKLET_CONNECT_UPD = ::RegisterWindowMessage(_T("QUICKLET_CONNECT_UPD"));
static UINT LAUNCH_BROWSER = ::RegisterWindowMessage(_T("LAUNCH_BROWSER"));

// static const CString CLIENT_GUID = "{E6F0542E-CE62-41B0-A89C-11E5EDEABF81}";
static const CString CLIENT_GUID = "{575D3E6F-170D-48D3-B746-A5A20A869949}";
static const CString SUBSCRIBER_ID = "{50189E8B-938E-40B8-ABD3-18A9812ADEC0}";

#ifdef DEBUG
static const CString APP_NAME = "Quicklet";
#else
static const CString APP_NAME = "Quicklet";
#endif

#define LEVION_CLIENT_VER "1.0"


static const CString UI_EVENT_QUERY_SUBSCRIPTION = "<?xml version=\"1.0\"?>" \
"<?qbxml version=\"8.0\"?>" \
"<QBXML><QBXMLSubscriptionMsgsRq><UIEventSubscriptionQueryRq>" \
"<UIEventSubscriptionQuery>" \
"<SubscriberID>" + SUBSCRIBER_ID + "</SubscriberID>" \
"</UIEventSubscriptionQuery>" \
"</UIEventSubscriptionQueryRq></QBXMLSubscriptionMsgsRq></QBXML>";

static const CString UI_EVENT_SUBSCRIPTION = "<?xml version=\"1.0\"?>" \
"<?qbxml version=\"8.0\"?>" \
"<QBXML><QBXMLSubscriptionMsgsRq><UIEventSubscriptionAddRq>" \
"<UIEventSubscriptionAdd>" \
"<SubscriberID>" + SUBSCRIBER_ID + "</SubscriberID>" \
"<COMCallbackInfo>" \
"<AppName>" + APP_NAME + "</AppName>" \
"<CLSID>" + CLIENT_GUID + "</CLSID>" \
"</COMCallbackInfo>" \
"<DeliveryPolicy>DeliverAlways</DeliveryPolicy>" \
"<CompanyFileEventSubscription>" \
"<CompanyFileEventOperation >Open</CompanyFileEventOperation>" \
"<CompanyFileEventOperation >Close</CompanyFileEventOperation>" \
"</CompanyFileEventSubscription>" \
"</UIEventSubscriptionAdd>" \
"</UIEventSubscriptionAddRq></QBXMLSubscriptionMsgsRq></QBXML>";

#define DATA_EVENT_SUBSCRIPTION "<?xml version=\"1.0\"?>" \
            "<?qbxml version=\"8.0\"?>" \
            "<QBXML><QBXMLSubscriptionMsgsRq><DataEventSubscriptionAddRq>" \
            "<DataEventSubscriptionAdd>" \
                "<SubscriberID>" + SUBSCRIBER_ID + "</SubscriberID>" \
                "<COMCallbackInfo>" \
                    "<AppName>" + APP_NAME + "</AppName>" \
                    "<CLSID>" + CLIENT_GUID + "</CLSID>" \
                "</COMCallbackInfo>" \
                "<DeliveryPolicy>DeliverOnlyIfRunning</DeliveryPolicy>" \
                "<TrackLostEvents>All</TrackLostEvents>" \
                "<DeliverOwnEvents>false</DeliverOwnEvents>" \
                "<ListEventSubscription>" \
                    "<ListEventType>Account</ListEventType>" \
                    "<ListEventType>Class</ListEventType>" \
                    "<ListEventType>Customer</ListEventType>" \
                    "<ListEventType>CustomerMsg</ListEventType>" \
                    "<ListEventType>CustomerType</ListEventType>" \
                    "<ListEventType>DateDrivenTerms</ListEventType>" \
                    "<ListEventType>Employee</ListEventType>" \
                    "<ListEventType>ItemDiscount</ListEventType>" \
                    "<ListEventType>ItemFixedAsset</ListEventType>" \
                    "<ListEventType>ItemGroup</ListEventType>" \
                    "<ListEventType>ItemInventory</ListEventType>" \
                    "<ListEventType>ItemInventoryAssembly</ListEventType>" \
                    "<ListEventType>ItemNonInventory</ListEventType>" \
                    "<ListEventType>ItemOtherCharge</ListEventType>" \
                    "<ListEventType>ItemPayment</ListEventType>" \
                    "<ListEventType>ItemSalesTax</ListEventType>" \
                    "<ListEventType>ItemSalesTaxGroup</ListEventType>" \
                    "<ListEventType>ItemService</ListEventType>" \
                    "<ListEventType>ItemSubtotal</ListEventType>" \
                    "<ListEventType>JobType</ListEventType>" \
                    "<ListEventType>OtherName</ListEventType>" \
                    "<ListEventType>PaymentMethod</ListEventType>" \
                    "<ListEventType>SalesRep</ListEventType>" \
                    "<ListEventType>SalesTaxCode</ListEventType>" \
                    "<ListEventType>ShipMethod</ListEventType>" \
                    "<ListEventType>StandardTerms</ListEventType>" \
                    "<ListEventType>ToDo</ListEventType>" \
                    "<ListEventType>Vendor</ListEventType>" \
                    "<ListEventType>VendorType</ListEventType>" \
                    "<ListEventOperation>Add</ListEventOperation>" \
                    "<ListEventOperation>Modify</ListEventOperation>" \
                    "<ListEventOperation>Delete</ListEventOperation>" \
                    "<ListEventOperation>Merge</ListEventOperation>" \
                "</ListEventSubscription>" \
                "<TxnEventSubscription>" \
                    "<TxnEventType>ARRefundCreditCard</TxnEventType>" \
                    "<TxnEventType>Bill</TxnEventType>" \
                    "<TxnEventType>BillPaymentCheck</TxnEventType>" \
                    "<TxnEventType>BillPaymentCreditCard</TxnEventType>" \
                    "<TxnEventType>Charge</TxnEventType>" \
                    "<TxnEventType>Check</TxnEventType>" \
                    "<TxnEventType>CreditCardCharge</TxnEventType>" \
                    "<TxnEventType>CreditCardCredit</TxnEventType>" \
                    "<TxnEventType>CreditMemo</TxnEventType>" \
                    "<TxnEventType>Deposit</TxnEventType>" \
                    "<TxnEventType>Estimate</TxnEventType>" \
                    "<TxnEventType>InventoryAdjustment</TxnEventType>" \
                    "<TxnEventType>Invoice</TxnEventType>" \
                    "<TxnEventType>ItemReceipt</TxnEventType>" \
                    "<TxnEventType>JournalEntry</TxnEventType>" \
                    "<TxnEventType>PurchaseOrder</TxnEventType>" \
                    "<TxnEventType>ReceivePayment</TxnEventType>" \
                    "<TxnEventType>SalesOrder</TxnEventType>" \
                    "<TxnEventType>SalesReceipt</TxnEventType>" \
                    "<TxnEventType>SalesTaxPaymentCheck</TxnEventType>" \
                    "<TxnEventType>TimeTracking</TxnEventType>" \
                    "<TxnEventType>VendorCredit</TxnEventType>" \
                    "<TxnEventOperation>Add</TxnEventOperation>" \
                    "<TxnEventOperation>Modify</TxnEventOperation>" \
                    "<TxnEventOperation>Delete</TxnEventOperation>" \
                "</TxnEventSubscription>" \
            "</DataEventSubscriptionAdd>" \
            "</DataEventSubscriptionAddRq></QBXMLSubscriptionMsgsRq></QBXML>"

#define UI_EVENT_UNSUBSCRIPTION "<?xml version=\"1.0\"?>" \
            "<?qbxml version=\"8.0\"?>" \
            "<QBXML>" \
            "<QBXMLSubscriptionMsgsRq>" \
            "<SubscriptionDelRq>" \
                "<SubscriberID>" + SUBSCRIBER_ID + "</SubscriberID>" \
                "<SubscriptionType>UI</SubscriptionType>" \
            "</SubscriptionDelRq>" \
            "</QBXMLSubscriptionMsgsRq>" \
            "</QBXML>"

#define DATA_EVENT_UNSUBSCRIPTION "<?xml version=\"1.0\"?>" \
            "<?qbxml version=\"8.0\"?>" \
            "<QBXML>" \
            "<QBXMLSubscriptionMsgsRq>" \
            "<SubscriptionDelRq>" \
                "<SubscriberID>" + SUBSCRIBER_ID + "</SubscriberID>" \
                "<SubscriptionType>Data</SubscriptionType>" \
            "</SubscriptionDelRq>" \
            "</QBXMLSubscriptionMsgsRq>" \
            "</QBXML>"

static const CString GET_COMPANY_TAG = "<?xml version=\"1.0\"?>" \
"<?qbxml version=\"8.0\"?>" \
"<QBXML>" \
"<QBXMLMsgsRq onError=\"stopOnError\">" \
"<CompanyQueryRq>" \
"<OwnerID>{E09C86CF-9D6E-4EF2-BCBE-4D66B6B0F754}</OwnerID>" \
"</CompanyQueryRq>" \
"</QBXMLMsgsRq>" \
"</QBXML>";

static const CString GET_TEMPLATES = "<?xml version=\"1.0\"?>" \
"<?qbxml version=\"8.0\"?>" \
"<QBXML>" \
"<QBXMLMsgsRq onError=\"stopOnError\">" \
"<TemplateQueryRq>" \
"</TemplateQueryRq>" \
"</QBXMLMsgsRq>" \
"</QBXML>";

static class URLS {
public:
	static CString APP_SERVER;
	static CString GOLIATH_SERVER;
	static CString DOWNLOAD_SERVER;
	static CString HELP_SERVER;
};
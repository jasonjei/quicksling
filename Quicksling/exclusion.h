#define UNIQUE_TO_SYSTEM  0
#define UNIQUE_TO_DESKTOP 1
#define UNIQUE_TO_SESSION 2
#define UNIQUE_TO_TRUSTEE 4
CString createExclusionName(LPCTSTR GUID, UINT kind = UNIQUE_TO_SYSTEM);

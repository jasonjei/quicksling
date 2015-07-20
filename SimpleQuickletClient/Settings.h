#ifndef SETTINGS_H
#define SETTINGS_H

#define __ATLSTR_H__ 1

#include "rsettings.h"
#include "stdafx.h"
#include "atlapp.h"
#include "atlmisc.h"
#include "Constants.h"


// Sample application configuration
class Settings : public CRegSettings
{
public:
	CString URL; // DWORD option
	CString ClientKey; // String option
	DWORD RequiredValue;

	BEGIN_REG_MAP(Settings)
		REG_ITEM(URL, URLS::GOLIATH_SERVER)
		REG_ITEM(ClientKey, _T("123"))
	END_REG_MAP()
};
#endif
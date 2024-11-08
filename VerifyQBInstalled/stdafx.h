// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#pragma warning(disable: 4995)

// Windows Header Files:
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <msiquery.h>

// WiX Header Files:
#include <wcautil.h>


// TODO: reference additional headers your program requires here
#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>

#pragma warning(default: 4995)


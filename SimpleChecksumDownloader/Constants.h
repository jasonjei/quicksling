#include "stdafx.h"
#pragma once

#ifndef CONSTANTS_H
#define CONSTANTS_H

static UINT SHOW_DOWNLOAD_PROGRESS = ::RegisterWindowMessage(_T("SHOW_DOWNLOAD_PROGRESS"));
static UINT HIDE_DOWNLOAD_PROGRESS = ::RegisterWindowMessage(_T("HIDE_DOWNLOAD_PROGRESS"));

#endif
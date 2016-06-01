#pragma once

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <thread>
#include <mutex>
#include <atomic>
#include "atlapp.h"
#include "atlmisc.h"
#include "inifile.h"
#include <ShlObj.h>
#include "DownloaderCallbackHandler.h"
#include "INet.h"
#include <sstream>
#include <sys/stat.h>
#include "Crc32Static.h"
#include <regex>
#include "XUnzip.h"
#include <io.h>
#include <memory>
#include <condition_variable>

class Downloader {
public:
	Downloader(void);
	~Downloader(void);
	DWORD StartDownload();
	int DoDownload();
	DWORD GetFile(CString url);
	CString GetDownloadManifest();
	CString GetLevionUserAppDir();
	CString GetLevionUserAppDir(CString File);
	CString HexStrToDecStr(const CString& sHex);
	DWORD HexStrToDecDword(const CString& sHex);
	DWORD ValidateCrcFile(CString filePath, CString hexCrc);
	DWORD GetSectionDownload(CIniKeyW* key);
	CString GuessFileNameFromURL(CString guessUrl);
	void AddToLogs(CString logEntry);
	int CreateLevionAppDir();

	std::shared_ptr<std::thread> thread;
	std::condition_variable cvThreadFinished;
	std::mutex download_mutex;
	std::mutex start_download_mutex;
	std::atomic<bool> isRunning;
	CIniFile ini;
	int currentProgress;
	CString currentFile;
	int currentDownloadAbort;
	int showErrors;
	float bytesPerSecond;
	ULONG bytesTotal;
};
#endif /* DOWNLOADER_H */
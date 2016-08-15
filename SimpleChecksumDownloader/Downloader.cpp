#include "stdafx.h"
#include "Downloader.h"
#include "spdlog\spdlog.h"

extern Downloader downloader;

Downloader::Downloader(void) : isRunning(0), showErrors(1) {

}

Downloader::~Downloader(void) {

}

CString Downloader::GuessFileNameFromURL(CString guessUrl) {
	const wchar_t *guessFileNameRegex = _T("[^/]+$");
	std::wregex rgx(guessFileNameRegex);

	std::wstring stdUrl(guessUrl);

	std::match_results<std::wstring::const_iterator> matchResults;

	bool match = std::regex_search(stdUrl, matchResults, rgx);
	return matchResults[0].str().c_str();
}

DWORD Downloader::GetFile(CString url) {
	CString filePath = GetLevionUserAppDir(GuessFileNameFromURL(url));

	//tcout << _T("Downloading   : ") << url << endl;
	//tcout << _T("To local file : ") << filePath << endl;

	// invalidate cache, so file is always downloaded from web site
	// (if not called, the file will be retieved from the cache if
	// it's already been downloaded.)
	url.TrimLeft();
	url.TrimRight();

	DeleteUrlCacheEntry(url);

	DownloaderCallbackHandler callbackHandler;
	IBindStatusCallback* pBindStatusCallback = NULL;
	callbackHandler.QueryInterface(IID_IBindStatusCallback,
		reinterpret_cast<void**>(&pBindStatusCallback));
	
	CString logEntry;
	logEntry.Format(_T("Downloading file: %s to %s"), url, filePath);
	AddToLogs(logEntry);

	this->currentDownloadAbort = 0;
	this->currentFile = GuessFileNameFromURL(url);
	this->currentProgress = 0;
	this->bytesPerSecond = 0.0;
	this->bytesTotal = 0;

	SetFileAttributes(filePath,
		GetFileAttributes(filePath) & ~FILE_ATTRIBUTE_READONLY);
	DeleteFile(filePath);

	HRESULT hr = URLDownloadToFile(
		NULL,   // A pointer to the controlling IUnknown interface
		url,
		filePath,
		0,      // Reserved. Must be set to 0.
		pBindStatusCallback);

	if (hr == S_OK)
		return 1;

	logEntry.Format(_T("Couldn't download file %s to %s"), url, filePath);
	AddToLogs(logEntry);

	return -1;
}

CString Downloader::GetDownloadManifest() {
	CIniFile configIni;
	CIniSectionW* devSec;
	CString sURL = _T("http://download.quicklet.io/download.ini");

	configIni.Load((LPCTSTR)GetLevionUserAppDir("config.ini"));

	devSec = configIni.GetSection(_T("Development"));
	if (devSec != NULL) {
		CIniKeyW* downloadUrl = devSec->GetKey(_T("download_url"));

		if (downloadUrl != NULL)
			sURL = downloadUrl->GetValue().c_str();
	}

	CInternetSession Session(_T("Quicksling Downloader"));
	WORD timeout = 40000;

	DeleteUrlCacheEntry(sURL);

	Session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, timeout);
	Session.SetOption(INTERNET_OPTION_SEND_TIMEOUT, timeout);

	CHttpFile* cHttpFile = NULL;
	int fail = 0;

	try {
		cHttpFile = new CHttpFile(Session, sURL, NULL, 0, INTERNET_FLAG_DONT_CACHE);
	}

	catch (CInternetException& e) {
		fail = 1;
	}

	if (!fail) {
		WTL::CString pageSource;

		UINT bytes = (UINT)cHttpFile->GetLength();

		char tChars[2048 + 1];
		int bytesRead;

		while ((bytesRead = cHttpFile->Read((LPVOID)tChars, 2048)) != 0) {
			tChars[bytesRead] = '\0';
			pageSource += CA2W((LPCSTR)tChars, CP_UTF8);
		}

		delete cHttpFile;

		CString logEntry;
		logEntry.Format(_T("Downloaded file manifest from %s"), sURL);
		AddToLogs(logEntry);

		return pageSource;
	}

	CString logEntry;
	logEntry.Format(_T("Couldn't download file manifest from %s"), sURL);
	AddToLogs(logEntry);

	delete cHttpFile;
	return _T("");
}

DWORD Downloader::GetSectionDownload(CIniKeyW* key) {
	GetFile(key->GetValue().c_str());

	if (ini.GetSection(_T("checksums")) == NULL) {
		CString logEntry;
		logEntry.Format(_T("No checksum for %s"), key->GetValue().c_str());
		AddToLogs(logEntry);

		if (showErrors)
			MessageBox(NULL, _T("No checksum provided for file to download"), _T("Quicklet File Download Error"), MB_OK);
		return -1;
	}

	CString fileToDownloadCheckHash = ini.GetSection(_T("checksums"))->GetKey(key->GetKeyName())->GetValue().c_str();
	if (ValidateCrcFile(GetLevionUserAppDir( GuessFileNameFromURL(key->GetValue().c_str()) ), fileToDownloadCheckHash) == 1)
		return 1;

	CString logEntry;
	logEntry.Format(_T("Checksum mismatched for %s"), key->GetValue().c_str());
	AddToLogs(logEntry);

	if (showErrors)
		MessageBox(NULL, _T("File checksum doesn't match with Quicklet's provided checksum"), _T("Quicklet File Download Error"), MB_OK);
	return -2;
}

void Downloader::AddToLogs(CString logEntry) {
	ATLTRACE2(atlTraceUI, 0, _T("%s\n"), logEntry);
}

DWORD Downloader::ValidateCrcFile(CString filePath, CString hexCrc) {
	DWORD dwCrc32 = NO_ERROR;
	DWORD dwErrorCode = NO_ERROR;

	hexCrc.TrimLeft();
	hexCrc.TrimRight();
	DWORD goodCrc = HexStrToDecDword(hexCrc);
	dwErrorCode = CCrc32Static::FileCrc32Assembly(filePath, dwCrc32);

	if (dwCrc32 == goodCrc) {
		CString logEntry;
		logEntry.Format(_T("%s validated for checksum %s"), filePath, hexCrc);
		AddToLogs(logEntry);

		return 1;
	}

	CString logEntry;
	logEntry.Format(_T("NO VALIDATION: %s DIDN'T VALIDATE for checksum %s"), filePath, hexCrc);
	AddToLogs(logEntry);

	return 0;
}

CString Downloader::GetLevionUserAppDir() {
	TCHAR szPath[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, szPath);
	PathAppend(szPath, _T("Quicksling"));
	return CString(szPath);
}

CString Downloader::GetLevionUserAppDir(CString File) {
	TCHAR szPath[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, szPath);
	PathAppend(szPath, _T("Quicksling"));
	PathAppend(szPath, File);
	return CString(szPath);
}

int Downloader::CreateLevionAppDir() {
	int success = CreateDirectory(GetLevionUserAppDir(), NULL);
	if (success)
		return 1;
	else {
		return PathIsDirectory(GetLevionUserAppDir());
	}
}

int Downloader::DoDownload() {
	std::unique_lock<std::mutex> lock(this->download_mutex, std::defer_lock);
	if (lock.try_lock() == false)
		return -1; 

	int filesMismatched = 0;

	CString downloadManifest = GetDownloadManifest();
	if (downloadManifest.IsEmpty()) {
		if (showErrors)
			MessageBox(NULL, _T("Couldn't download Quicklet file manifest"), _T("Quicklet File Download Error"), MB_OK);
		return -1;
	}

	std::wistringstream downloadManifestStream((LPCTSTR) downloadManifest);

	ini.Load(downloadManifestStream, false);
	CIniSectionW* downloads = ini.GetSection(_T("downloads"));

	if (downloads == NULL) {
		CString logEntry;
		logEntry.Format(_T("No files to download in manifest"));
		AddToLogs(logEntry);

		if (showErrors)
			MessageBox(NULL, _T("Quicklet file manifest doesn't contain files to download"), _T("Quicklet File Download Error"), MB_OK);
		return -2;
	}

	if (CreateLevionAppDir() == 0) {
		CString logEntry;
		logEntry.Format(_T("Couldn't make folder at %s"), GetLevionUserAppDir());
		AddToLogs(logEntry);

		if (showErrors)
			MessageBox(NULL, logEntry, _T("Quicklet Setup Error"), MB_OK);
		return -6; // No Quicksling folder in AppData/Local
	}

	for (KeyIndex::const_iterator kitr = downloads->GetKeys().begin();
		kitr != downloads->GetKeys().end(); kitr++) {

		CString sectionName = (*kitr)->GetKeyName().c_str();
		CIniSectionW* section = ini.GetSection((LPCTSTR) sectionName);

		CString logEntry;
		logEntry.Format(_T("== SECTION CHECKING TO BEGIN: %s =="), sectionName);
		AddToLogs(logEntry);

		for (KeyIndex::const_iterator inner_kitr = section->GetKeys().begin();
			inner_kitr != section->GetKeys().end(); inner_kitr++) {

			CString filePath = GetLevionUserAppDir((*inner_kitr)->GetKeyName().c_str());

			// Check if file exists
			struct _stat buffer;
			int success = _wstat((LPCTSTR) filePath, &buffer);

			if (success == -1) {
				logEntry.Format(_T("DOWNLOAD REQUIRED: %s DOESN'T EXIST in %s"), (*inner_kitr)->GetKeyName().c_str(), filePath);
				AddToLogs(logEntry);
			}

			if (success == -1 || (ValidateCrcFile(filePath, (*inner_kitr)->GetValue().c_str()) == 0)) {
				// Request download of ZIP file and extract
				// Break out of the loop
				filesMismatched = 1;
				int result = GetSectionDownload(*kitr);

				if (result == -1) {
					return -3; // no checksum
				}
				else if (result == -2) {
					return -4; // mismatch checksum
				}

				// Time to extract files
				SetCurrentDirectory(GetLevionUserAppDir());

				CString zipFile = GetLevionUserAppDir(GuessFileNameFromURL((*kitr)->GetValue().c_str()));
				// TODO: Verify ZIP file is there
				HZIP hz = OpenZip((void*) ((LPCTSTR) zipFile), 0, ZIP_FILENAME);
				ZIPENTRYW ze; GetZipItem(hz,-1,&ze); int numitems=ze.index;
				for (int i=0; i<numitems; i++){ 
					GetZipItem(hz,i,&ze);

					logEntry.Format(_T("Unzipping file: %s"), ze.name);
					AddToLogs(logEntry);

					ZRESULT unzip_result = UnzipItem(hz,i,ze.name,0,ZIP_FILENAME);
					if (unzip_result != ZR_OK) {
						return -5; // couldn't unzip
					}

				}
				CloseZip(hz);
				break;
			}
		}
		logEntry.Format(_T("== SECTION CHECKING COMPLETED: %s =="), sectionName);
		AddToLogs(logEntry);
	}

	if (filesMismatched) {
		CString logEntry;
		logEntry.Format(_T("== FILES WERE MISMATCHED =="));
		AddToLogs(logEntry);
	}
	else {
		CString logEntry;
		logEntry.Format(_T("== NO FILES WERE CHANGED =="));
		AddToLogs(logEntry);
	}

	isRunning = false;
	lock.unlock();
	cvThreadFinished.notify_one();
	return 0;
}

DWORD Downloader::StartDownload() {
	namespace spd = spdlog;
	try {
		auto file_logger = spd::rotating_logger_mt("file_logger", std::string(CW2A(GetLevionUserAppDir(_T("checksum")), CP_UTF8)), 250000, 3);
		for (int i = 0; i < 10; ++i)
			file_logger->info("{} * {} equals {:>10}", i, i, i*i);
	}
	catch (const spd::spdlog_ex& ex) {
		std::cout << "Log failed: " << ex.what() << std::endl;
	}

	std::unique_lock<std::mutex> lock(this->start_download_mutex, std::defer_lock);
	if (lock.try_lock() == false)
		return -1;

	if (this->thread && this->thread->joinable() == true)
		return -1;

	if (this->isRunning == true)
		return -1;

	isRunning = true;
	this->thread = std::shared_ptr<std::thread>(new std::thread(&Downloader::DoDownload, this));
	this->thread->detach();
	return 0;
}

CString Downloader::HexStrToDecStr(const CString& sHex) {
	CString sDec;
	wchar_t *p;
	sDec.Format(_T("%d"), wcstol((TCHAR*)((LPCTSTR)sHex), &p, 16));
	return sDec;
}

DWORD Downloader::HexStrToDecDword(const CString& sHex) {
	// CString sDec;
	// wchar_t *p;
	// sDec.Format(_T("%d"), wcstol((TCHAR*)((LPCTSTR)sHex), &p, 16));

	std::string tempString = std::string(CW2A(sHex, CP_UTF8));
	DWORD m_dwIP = std::strtoul(tempString.c_str(), NULL, 16);
	return m_dwIP;
}
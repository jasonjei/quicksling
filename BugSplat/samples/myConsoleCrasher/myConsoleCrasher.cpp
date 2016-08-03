//
// console application to produce native Windows crashes
//

#pragma optimize( "", off) // prevent optimizer from interfering with our crash-producing code

#include "stdafx.h"
#include <windows.h>
#include "BugSplat.h"

void StackOverflow(void *p);
bool ExceptionCallback(UINT nCode, LPVOID lpVal1, LPVOID lpVal2);

MiniDmpSender *mpSender;

int wmain(int argc, wchar_t **argv)
{

    if (IsDebuggerPresent())
    {
        wprintf(L"Run this application without the debugger attached to allow BugSplat exception handling.\n");
        DebugBreak();
        exit(0);
    }

    // BugSplat initialization
    mpSender = new MiniDmpSender(L"Fred123", L"myConsoleCrasher", L"1.0", NULL);
	
    // Set (optional) default values for user and email.  These aren't used unless no values are supplied in the crash report dialog.
    mpSender->setDefaultUserName(_T("Fred"));
    mpSender->setDefaultUserEmail(_T("fred@bedrock.com"));

    // process command line args that we need prior to crashing
    for (int i = 1; i < argc; i++) {

        if (!_wcsicmp(argv[i], L"/AttachFiles")) {
            mpSender->setCallback(ExceptionCallback); // files are attached in the callback after the exception occurs
        }

    }

    // crash
    for (int i = 1; i < argc; i++) {

        if (!_wcsicmp(argv[i], L"/Crash")) {
            // don't let the BugSplat dialog appear
            mpSender->setFlags(MDSF_NONINTERACTIVE | mpSender->getFlags());
        }

        if (!_wcsicmp(argv[i], L"/MemoryException") || !_wcsicmp(argv[i], L"/Crash")) {
            // dereferencing a null pointer results in a memory exception
            *(volatile int *)0 = 0;
        }

        else if (!_wcsicmp(argv[i], L"/StackOverflow")) {
            // calling a recursive function with no exit results in a stack overflow
            StackOverflow(NULL);
        }

        else if (!_wcsicmp(argv[i], L"/DivByZero")) {
            // no surprises here
            volatile int x, y, z;
            x = 1;
            y = 0;
            z = x / y;
        }

        else if (!_wcsicmp(argv[i], L"/Abort")) {
            abort(); // must build in release configuration to catch this one
        }

    }

    // default if no crash resulted from command line args
    *(volatile int *)0 = 0;

    return 0;
}

void StackOverflow(void *p)
{
    volatile char q[10000];
    while (true) {
        StackOverflow((void *)q);
    }
}

// BugSplat exception callback
bool ExceptionCallback(UINT nCode, LPVOID lpVal1, LPVOID lpVal2)
{

    switch (nCode)
    {
        case MDSCB_EXCEPTIONCODE:
        {
            EXCEPTION_RECORD *p = (EXCEPTION_RECORD *)lpVal1;
            DWORD code = p ? p->ExceptionCode : 0;

            // create some files in the %temp% directory and attach them
            wchar_t cmdString[2 * MAX_PATH];
            wchar_t filePath[MAX_PATH];
            wchar_t tempPath[MAX_PATH];
            GetTempPathW(MAX_PATH, tempPath);

            wsprintf(filePath, L"%sfile1.txt", tempPath);
            wsprintf(cmdString, L"echo Exception Code = 0x%08x > %s", code, filePath);
            _wsystem(cmdString);
            mpSender->sendAdditionalFile(filePath);

            wsprintf(filePath, L"%sfile2.txt", tempPath);
            wsprintf(cmdString, L"echo Crash reporting is so clutch! > %s", filePath);
            _wsystem(cmdString);
            mpSender->sendAdditionalFile(filePath);

        }
        break;
    }

    return false;
}

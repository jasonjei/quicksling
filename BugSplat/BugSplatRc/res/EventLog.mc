;#ifndef __EVENTLOG_MC__
;#define __EVENTLOG_MC__
;


LanguageNames =
    (
        English = 0x0409:Messages_ENU
    )


;////////////////////////////////////////
;// Events
;//

MessageId       = +1
SymbolicName    = EVENT_CRASH_LOGGED
Language        = English   
This event has been logged by the BugSplat crash reporting library (http://www.bugsplatsoftware.com) 
in partnership with your vendor %1.
A crash report from the application '%2' has been successfully logged into the BugSplat database with id=%3.
Please contact your vendor for more information.
.

;
;#endif  //__EVENTLOG_MC__
;



/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.00.0603 */
/* at Tue Aug 02 20:32:14 2016
 */
/* Compiler settings for QuickslingShell.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.00.0603 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __QuickslingShell_h__
#define __QuickslingShell_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __QBSDKCallback_FWD_DEFINED__
#define __QBSDKCallback_FWD_DEFINED__

#ifdef __cplusplus
typedef class QBSDKCallback QBSDKCallback;
#else
typedef struct QBSDKCallback QBSDKCallback;
#endif /* __cplusplus */

#endif 	/* __QBSDKCallback_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __QuickslingShellLib_LIBRARY_DEFINED__
#define __QuickslingShellLib_LIBRARY_DEFINED__

/* library QuickslingShellLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_QuickslingShellLib;

EXTERN_C const CLSID CLSID_QBSDKCallback;

#ifdef __cplusplus

class DECLSPEC_UUID("675D3E6F-170D-48D3-B746-A5A20A869949")
QBSDKCallback;
#endif
#endif /* __QuickslingShellLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



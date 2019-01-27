// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__D955DDC2_F770_11D7_80CF_B48F3533E25F__INCLUDED_)
#define AFX_STDAFX_H__D955DDC2_F770_11D7_80CF_B48F3533E25F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers

#define STRICT
#define _WIN32_WINNT 0x0400
#define _WIN32_IE    0x0400
#define _ATL_APARTMENT_THREADED

// The following is required to use riched32.dll in a Unicode build
#define _RICHEDIT_VER 0x100
#define ATL_TRACE_LEVEL 4

#if _MSC_VER > 1200
#define WINVER 0x0500
#endif

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlcom.h>
#include <atlctl.h>

#include "CommonIncludes.h"
#include "UtilitesIncludes.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__D955DDC2_F770_11D7_80CF_B48F3533E25F__INCLUDED_)

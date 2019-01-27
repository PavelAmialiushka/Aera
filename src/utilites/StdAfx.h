#if !defined(AFX_STDAFX_H__506BE683_F108_11D7_80CF_C11FB94CA558__INCLUDED_)
#define AFX_STDAFX_H__506BE683_F108_11D7_80CF_C11FB94CA558__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers

#define STRICT
#define _WIN32_WINNT 0x0400
#define _WIN32_IE    0x0400
#define _ATL_APARTMENT_THREADED

#define _RICHEDIT_VER 0x100
#define ATL_TRACE_LEVEL 4

// 'function': was declared deprecated 
#pragma warning (disable : 4996) 

#if _MSC_VER > 1200
#define WINVER 0x0500
#endif

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlcom.h>
#include <atlctl.h>

#include "CommonIncludes.h "

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__506BE683_F108_11D7_80CF_C11FB94CA558__INCLUDED_)

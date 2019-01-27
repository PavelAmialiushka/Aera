#if !defined(AFX_STDAFX_H__BA16622E_8616_41F3_B0D2_99BA861A36EA__INCLUDED_)
#define AFX_STDAFX_H__BA16622E_8616_41F3_B0D2_99BA861A36EA__INCLUDED_

#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers

#define STRICT
#define _WIN32_WINNT 0x0400
#define _WIN32_IE    0x0400
#define _ATL_APARTMENT_THREADED

#define _RICHEDIT_VER 0x200
#define ATL_TRACE_LEVEL 4

// 'function': was declared deprecated 
#pragma warning (disable : 4996) 

#if _ATL_VER >= 0x0700
#define _WTL_NO_CSTRING 1
#endif

#if _MSC_VER > 1200
#define WINVER 0x0500
#endif

#include <atlbase.h>

#include <atlapp.h>
extern CAppModule _Module;

#include <atlmisc.h>

#if _ATL_VER >= 0x0700
# include <atlcoll.h>
//# include <atlstr.h>
#endif

#include <shellapi.h>

#include <atlcom.h>
#include <atlwin.h>
#include <atlctl.h>

#include <atlctrls.h>
#include <atlctrlx.h>
#include <atlcrack.h>


#include "CommonIncludes.h"
#include "utilitesIncludes.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>

//////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__BA16622E_8616_41F3_B0D2_99BA861A36EA__INCLUDED_)

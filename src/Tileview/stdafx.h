#define _WIN32_WINNT  0x0400
#define _RICHEDIT_VER 0x0100
#define _WIN32_IE     0x0400
#define WIN32_LEAN_AND_MEAN

// 4996 'function': was declared deprecated 
#pragma warning (disable : 4996) 

#if _MSC_VER > 1200
#define WINVER 0x0500
#endif

#include <atlbase.h>
#include <atlapp.h>

#include <winbase.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include <atlcrack.h>
#include <atlmisc.h>

#include "CommonIncludes.h"
#include "utilitesIncludes.h"


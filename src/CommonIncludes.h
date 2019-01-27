#pragma once

// 4800 'type' : forcing value to bool 'true' or 'false' (performance warning)
// 4996 'function': was declared deprecated 
// 4503 'identifier' : decorated name length exceeded, name was truncated
#pragma warning (disable : 4800 4996 4503)

#pragma warning (push)
// 4018 'expression' : signed/unsigned mismatch
// 4786 'identifier' : identifier was truncated to 'number' characters in the debug information
// 4284 return type for 'identifier::operator –>' is not a UDT or reference to a UDT
// 4275 non – DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
// 4251 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
// 4273 'function' : inconsistent DLL linkage
// 4290 C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
// 4761 integral size mismatch in argument : conversion supplied
// 4146  unary minus operator applied to unsigned type, result still unsigned
#pragma warning (disable : 4018 4786 4284 4275 4251 4273 4290 4761 4146)

// Platform
#include <crtdbg.h>
#include <process.h>

// STLPort

#pragma warning (disable : 4244)

#ifdef NDEBUG
# define _DEBUG_ false
#else
# define _DEBUG_ true
#endif

#if _MSC_VER <= 1200

#  include <limits>

#  if 0 && !defined(_STLPORT_VERSION)
#    define auto_ptr dinkumware_auto_ptr
#    include <memory>
#    undef auto_ptr
#    include "stl_auto_ptr.h"
#  endif

#endif

// BoostLibrary

#include <boost\config.hpp>

#include <boost\array.hpp>

#include <boost\smart_ptr.hpp>

#include <boost\bind.hpp>
#include <boost\function.hpp>

#include <boost\thread\thread.hpp>

#pragma warning (pop)

#define MAKE_SHARED(a) class a; typedef shared_ptr<a> p##a
#define MAKE_SHARED_STRUCT(a) struct a; typedef shared_ptr<a> p##a


#define _CRT_SECURE_NO_WARNINGS 

//using boost::array;
using boost::bind;
using boost::mem_fn;
using boost::shared_ptr;
using boost::scoped_ptr;
using boost::weak_ptr;

using boost::function0;
using boost::function1;
using boost::function2;
using boost::function3;

#include <memory>

#include <functional>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <limits>

#include <strstream>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <utility>

#include <string>
#include <vector>
#include <deque>
#include <stack>
#include <list>
#include <map>
#include <set>

#include <typeinfo>


#undef min
#undef max

using std::max;
using std::min;
using std::auto_ptr;

#define for if (0); else for

#define M_PI  3.1415926535897932384626433832795
#define M_360 6.2831853071795864769252867665590
#define M_180 M_PI
#define M_90  1.5707963267948966192313216916398


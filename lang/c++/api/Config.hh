/*

 */

#ifndef aingle_Config_hh
#define aingle_Config_hh

// Windows DLL support

#ifdef _WIN32
#pragma warning(disable : 4275 4251)

#if defined(AINGLE_DYN_LINK)
#ifdef AINGLE_SOURCE
#define AINGLE_DECL __declspec(dllexport)
#else
#define AINGLE_DECL __declspec(dllimport)
#endif // AINGLE_SOURCE
#endif // AINGLE_DYN_LINK

#include <intsafe.h>
using ssize_t = SSIZE_T;
#endif // _WIN32

#ifndef AINGLE_DECL
#define AINGLE_DECL
#endif

#endif

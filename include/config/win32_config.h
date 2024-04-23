#ifndef __WIN32_CONFIG_H__
#define __WIN32_CONFIG_H__

#define EXCEPTIONS_DISABLED             1

#undef _MIN
#undef _MAX

#if defined( _MSC_VER )
#define strncasecmp _strnicmp

#if !defined( __MINGW32__ )
#if _MSC_VER < 1500
#define vsnprintf _vsnprintf
#endif

// defined in VS2013
#if _MSC_VER < 1800
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif

// defined in VS2015
#if _MSC_VER < 1900
#define snprintf _snprintf
#define isnan _isnan
#endif

#endif


#endif // _MSC_VER


typedef int socklen_t;

#define ioctl   ioctlsocket

#if defined( _MSC_VER )
#pragma warning( disable : 4244 )   // conversion
#pragma warning( disable : 4786 )   // identifier length
#pragma warning( disable : 4800 )   //
#pragma warning( disable : 4305 )   // truncation from const double to float etc

#endif

namespace nbe
{
#if defined(_WIN64)

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned int uint32;
typedef unsigned long long int uint64;

typedef signed char int8;
typedef short int int16;
typedef int int32;
typedef long long int int64;

#else

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;
typedef unsigned long long int uint64;

typedef signed char int8;
typedef short int int16;
typedef long int int32;
typedef long long int int64;

#endif
}

#endif // !__WIN32_CONFIG_H__

#ifndef __LINUX_CONFIG_H__
#define __LINUX_CONFIG_H__

namespace nbe
{
#if defined(__x86_64__)

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned int uint32;
typedef unsigned long int uint64;

typedef signed char  int8;
typedef  short int int16;
typedef int int32;
typedef long int int64;

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

#endif // !__LINUX_CONFIG_H__


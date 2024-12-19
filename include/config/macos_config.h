#ifndef __MACOS_CONFIG_H__
#define __MACOS_CONFIG_H__

#define STD std


#define STD_MIN std::min
#define STD_MAX std::max

namespace dag
{
// 20140805 AMT: Define some standard types for different integer sizes under 32 and 64 bit instances of the build
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

#endif // !__MACOS_CONFIG_H__

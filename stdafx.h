#ifndef STDAFX_H_INCLUDED
#define STDAFX_H_INCLUDED

#ifdef __linux__
typedef unsigned char U8;
typedef char S8;
typedef unsigned short int U16;
typedef short int S16;
typedef unsigned int U32;
typedef int S32;
typedef unsigned long long U64;
typedef long long S64;
#include <smmintrin.h>
#else
#define U64 unsigned __int64
#define U32 unsigned __int32
#define U16 unsigned __int16
#define U8  unsigned __int8
#define S64 signed   __int64
#define S32 signed   __int32
#define S16 signed   __int16
#define S8  signed   __int8
#include <intrin.h>
#endif




#endif
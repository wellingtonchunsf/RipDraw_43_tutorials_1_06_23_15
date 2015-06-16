//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux
//
//

#ifndef _RIPDRAWCOMMON_H_
#define _RIPDRAWCOMMON_H_

#include "../cbase/StringUtils.h"
#include "../cbase/ProtoBuffer.h"
#include "../cbase/Log.h"

#ifdef  __cplusplus
extern "C"
{
#endif

	// macro definitions

#if defined (_RD_OS_WINDOWS)
#define RDAPI extern
#define RDAPI_INLINE static __inline
#define	RD_SLEEP(x)		Sleep(x);
#else
#define RDAPI extern
#define RDAPI_INLINE static inline
#define	RD_SLEEP(x)	usleep(x*1000);
#endif

#define RD_TEST_ERROR(x) \
ret = x; \
if (ret != RdErrorSuccess) \
{ \
	return ret;\
}

#define RD_TEST(x) \
ret = x; \
if (ret != RdErrorSuccess) \
{ \
	RD_ERR("\nFailed with code %x", ret); \
	getchar(); \
	return ret;\
}

#define _RD_CHECK_INTERFACE()\
if (rdi == NULL)\
{\
	RD_ERR("\ninterface should not NULL\n"); \
	return RdErrorInterfaceNull; \
}\
if (rdi->isOpen != 1)\
{\
	RD_ERR("\nport not opened\n"); \
	return RdErrorPortOpenFailed; \
}

#define RD_INTERFACE_RESPONSE_RETRY_COUNT_DEFAULT 200
#define RD_INTERFACE_RESPONSE_RETRY_DELAY_DEFAULT 1

int RdInitialize(RdInterface* rdi, int argc, char **argv);
int RdClose(RdInterface* rdi);

RdPosition RdCreatePosition(RdUWord x, RdUWord y);
RdSize RdCreateSize(RdUWord width, RdUWord height);
RdColor RdCreateColor(RdByte red, RdByte green, RdByte blue, RdByte alpha);

#ifdef  __cplusplus
}
#endif

#endif


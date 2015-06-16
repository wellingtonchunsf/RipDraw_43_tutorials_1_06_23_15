//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux


#ifndef __RIPDRAW_PROTOBUFFER_H__
#define __RIPDRAW_PROTOBUFFER_H__

#include "RipdrawTypes.h"

#ifdef  __cplusplus
extern "C"
{
#endif

RdUWord RdChecksum(RdByte* data, int length);

RdStatus RdBufferCheckAndAllocate(RdInterfaceBuffer* buffer, int required_capacity);

RdStatus RdWriteByte(RdInterfaceBuffer* buffer, RdByte input);

RdStatus RdWriteUWord(RdInterfaceBuffer* buffer, RdUWord input);

RdStatus RdWriteUInt32(RdInterfaceBuffer* buffer, RdUInt32 input);

RdStatus RdWriteFlag(RdInterfaceBuffer* buffer, RdFlag input);

RdStatus RdWritePosition(RdInterfaceBuffer* buffer, RdPosition input);

RdStatus RdWriteSize(RdInterfaceBuffer* buffer, RdSize input);

RdStatus RdWriteColor(RdInterfaceBuffer* buffer, RdColor input);

RdStatus RdWriteHDirection(RdInterfaceBuffer* buffer, RdHDirection input);

RdStatus RdWriteDirection(RdInterfaceBuffer* buffer, RdDirection input);

RdStatus RdWriteString(RdInterfaceBuffer* buffer, const char* input);

RdStatus RdReadByte(RdInterfaceBuffer* buffer, RdByte* output);

RdStatus RdReadUWord(RdInterfaceBuffer* buffer, RdUWord* output);

RdStatus RdReadUInt32(RdInterfaceBuffer* buffer, RdUInt32* output);

RdStatus RdReadFlag(RdInterfaceBuffer* buffer, RdFlag output);

RdStatus RdReadPosition(RdInterfaceBuffer* buffer, RdPosition* output);

RdStatus RdReadSize(RdInterfaceBuffer* buffer, RdSize* output);

RdStatus RdReadColor(RdInterfaceBuffer* buffer, RdColor* output);

RdStatus RdReadHDirection(RdInterfaceBuffer* buffer, RdHDirection* output);

RdStatus RdReadDirection(RdInterfaceBuffer* buffer, RdDirection* output);

RdStatus RdReadString(RdInterfaceBuffer* buffer, char** output);

// for debug
int RdBufferDump(const char* label, RdInterfaceBuffer* buffer, int printIfNonZero);

#ifdef  __cplusplus
}
#endif

#endif

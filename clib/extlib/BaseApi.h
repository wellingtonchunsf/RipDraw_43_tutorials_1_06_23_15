//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux

#ifndef __RIPDRAW_EXTLIB_BASE_API_H__
#define __RIPDRAW_EXTLIB_BASE_API_H__

#include "DownloadApi.h"

RdStatus RdResourceInfoLoad(RdInterface *rdi,  RdResourceLoadInfo resourceLoadTable[], RdUInt32 arraySize, RdFlag autoDownload, const char* projectPath);
RdId RdResourceInfoTableGetId(RdResourceLoadInfo resourceLoadTable[], RdUInt32 arraySize, const char* resLabel);

#endif

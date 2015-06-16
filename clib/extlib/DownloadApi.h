//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux
//
//

#include "../cbase/StringUtils.h"
#include "../common/Ripdraw.h"

#ifndef __RIPDRAW_EXTLIB_DOWNLOAD_API_H__
#define __RIPDRAW_EXTLIB_DOWNLOAD_API_H__

#ifdef __cplusplus
extern "C"{
#endif
RdStatus RdDeviceDataDownloadFile(RdInterface* rdi,const char* dirPath,const char* fileName);
RdStatus RdDeviceDataDownloadAll(RdInterface* rdi, const char* dirPath);
RdStatus RdDeviceDeleteAll(RdInterface* rdi);
RdStatus RdDownloadUnavailableResources(RdInterface *rdi,RdResourceLoadInfo downloadResources[],const char* projectPath,RdId failedCount);

#ifdef __cplusplus
}
#endif
#endif

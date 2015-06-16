//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux

#ifndef __RIPDRAW_LOGGER_H__
#define __RIPDRAW_LOGGER_H__

#include "RipdrawTypes.h"
#include "StringUtils.h"

typedef enum _RdDbgLevel
{
	RdDbgLevelDefault = 0,
	RdDbgLevelError = 1,
	RdDbgLevelWarning = 2,
	RdDbgLevelInfo = 3,
	RdDbgLevelDbg1 = 4,
	RdDbgLevelDbg2 = 5,
} RdDbgLevel;

typedef enum _RdLogType
{
	RdLogTypeConsole = 0,
	RdLogTypeFile = 1,
	RdLogTypeBoth= 2,
} RdLogType;

typedef struct _RdLogInfo
{
	RdDbgLevel level;
	RdLogType type;
	char logFileName[RD_PATH_MAX_LEN];
} RdLogInfo;

#ifdef  __cplusplus
extern "C"
{
#endif

#define RD_LOG_PRINT_TIME_DIFF 1
#define RD_LOG_PRINT_DBGLEVEL_FUNCNAME 1

void _RdLogPrint(RdDbgLevel level, const char* functionName, int lineEnd, const char* format, ...);
void _RdLogPrintNoPrefix(RdDbgLevel level, const char* format, ...);

void RdSetLogInfo(RdLogType logType, RdDbgLevel logLevel, const char* logFileName);
RdLogInfo* RdGetLogInfo();
int RdInitLogInfo(int argc, char **list);

#define RD_INFO(format, ...) \
		_RdLogPrint(RdDbgLevelInfo, __FUNCTION__, 1, format, ##__VA_ARGS__);

#define RD_ERR(format, ...) \
		_RdLogPrint(RdDbgLevelError, __FUNCTION__, 1, format, ##__VA_ARGS__);

#define RD_WARN(format, ...) \
		_RdLogPrint(RdDbgLevelWarning, __FUNCTION__, 1, format, ##__VA_ARGS__);

#define RD_DBG(format, ...) \
		_RdLogPrint(RdDbgLevelDbg1, __FUNCTION__, 1, format, ##__VA_ARGS__);

#define RD_DBG1(format, ...) \
		_RdLogPrint(RdDbgLevelDbg2, __FUNCTION__, 1, format, ##__VA_ARGS__);

#define RDLOG_NO_PREFIX(level, format, ...) \
		_RdLogPrintNoPrefix(level, format, ##__VA_ARGS__);

#ifdef  __cplusplus
}
#endif

#endif

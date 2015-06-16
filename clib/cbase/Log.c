//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux
//
//

#include "Log.h"

RdLogInfo rdLogInfo = { 0 };

void _RdLogGetTime(char *timeBuf, int timeBufMaxSize)
{
#if RD_LOG_PRINT_TIME_DIFF
	unsigned int value = 0;
	static unsigned int lastTimeCount = 0;
	int diff;
#if defined (_RD_OS_WINDOWS)
	SYSTEMTIME st;
	GetLocalTime(&st);
	value = (st.wHour * 3600000) + (st.wMinute * 60000) + (st.wSecond * 1000) + st.wMilliseconds;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	value = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#endif

	if (lastTimeCount == 0)
	{
		diff = 0;
	}
	else
	{
		diff = value - lastTimeCount;
	}
	lastTimeCount = value;
#if defined (_RD_OS_WINDOWS)
	sprintf_s(timeBuf, timeBufMaxSize, "%06d", diff);
#else
	snprintf(timeBuf, timeBufMaxSize, "%06d", diff);
#endif

#else

#if defined (_RD_OS_WINDOWS)
	SYSTEMTIME str_t;
	GetLocalTime(&str_t);
	sprintf_s(timeBuf, timeBufMaxSize, "%d:%02d:%02d %02d:%02d:%02d.%03ld", str_t.wYear, str_t.wMonth, str_t.wDay, str_t.wHour, str_t.wMinute, str_t.wSecond,
			str_t.wMilliseconds);
#else
	struct tm *dateandtime;
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	dateandtime = localtime(&tv.tv_sec);

	snprintf(timeBuf, timeBufMaxSize, "%d:%02d:%02d %02d:%02d:%02d.%03d", (1900 + dateandtime->tm_year), dateandtime->tm_mon, dateandtime->tm_mday,
			dateandtime->tm_hour, dateandtime->tm_min, dateandtime->tm_sec, (int) (tv.tv_usec / 1000));
#endif
#endif
}

int _RdPrintOnConsole(const char* output, int lineEnd)
{
	if (lineEnd)
	{
		printf("%s\n", output);
	}
	else
	{
		printf("%s", output);
	}
	return RdErrorSuccess;
}

int _RdPrintOnFile(const char* output, int lineEnd)
{
	FILE* fp = NULL;
#if defined (_RD_OS_WINDOWS)
	errno_t errNo;
	errNo = fopen_s(&fp, rdLogInfo.logFileName, "a+");
	if (errNo != 0)
	{
		fp = NULL;
	}
#else
	fp = fopen(rdLogInfo.logFileName, "a+");
#endif
	if (fp == NULL)
	{
		fprintf(stderr, "Failed to open log file: %s\n", rdLogInfo.logFileName);
		return RdErrorLogFileOpenFailed;
	}

	if (lineEnd)
	{
		fprintf(fp, "%s\n", output);
	}
	else
	{
		fprintf(fp, "%s", output);
	}

	fclose(fp);
	return RdErrorSuccess;
}

int _RdPrint(const char* output, int lineEnd)
{
	int ret = 0;
	switch (rdLogInfo.type)
	{
	case RdLogTypeConsole:
		ret = _RdPrintOnConsole(output, lineEnd);
		break;

	case RdLogTypeFile:
		ret = _RdPrintOnFile(output, lineEnd);
		break;

	case RdLogTypeBoth:
		ret = _RdPrintOnConsole(output, lineEnd);
		ret = _RdPrintOnFile(output, lineEnd);
		break;

	default:
		puts("Log type Invalid\n");
		break;
	}

	return ret;
}

void _RdLogPrint(RdDbgLevel level, const char* functionName, int lineEnd, const char* format, ...)
{
	va_list list;
	char time[255] =
	{ 0 };
	char variableargString[RD_STRING_MAX_LEN];
	char outputString[RD_STRING_MAX_LEN];

	if (rdLogInfo.level <= 0)
	{
		rdLogInfo.level = RdDbgLevelInfo;
	}
	if (level > rdLogInfo.level)
	{
		return;
	}

	va_start(list, format);
#if defined (_RD_OS_WINDOWS)
	vsprintf_s(variableargString, RD_STRING_MAX_LEN, format, list);
#else
	vsnprintf(variableargString, RD_STRING_MAX_LEN, format, list);
#endif
	va_end(list);

	_RdLogGetTime(time, sizeof(time));
	RdStringCopy(outputString, RD_STRING_MAX_LEN, time, RD_STRING_MAX_LEN);
	RdStringCat(outputString, RD_STRING_MAX_LEN, ": ", RD_STRING_MAX_LEN);

#if RD_LOG_PRINT_DBGLEVEL_FUNCNAME
	switch (level)
	{
	case RdDbgLevelError:
		RdStringCat(outputString, RD_STRING_MAX_LEN, "ERR ", RD_STRING_MAX_LEN);
		break;

	case RdDbgLevelWarning:
		RdStringCat(outputString, RD_STRING_MAX_LEN, "WRN ", RD_STRING_MAX_LEN);
		break;

	case RdDbgLevelInfo:
		RdStringCat(outputString, RD_STRING_MAX_LEN, "INFO", RD_STRING_MAX_LEN);
		break;

	case RdDbgLevelDbg1:
		RdStringCat(outputString, RD_STRING_MAX_LEN, "DBG1", RD_STRING_MAX_LEN);
		break;

	case RdDbgLevelDbg2:
		RdStringCat(outputString, RD_STRING_MAX_LEN, "DBG2", RD_STRING_MAX_LEN);
		break;

	default:
		break;
	}

	RdStringCat(outputString, RD_STRING_MAX_LEN, ": ", RD_STRING_MAX_LEN);
	RdStringCat(outputString, RD_STRING_MAX_LEN, functionName, RD_STRING_MAX_LEN);
	RdStringCat(outputString, RD_STRING_MAX_LEN, ": ", RD_STRING_MAX_LEN);
#endif
	RdStringCat(outputString, RD_STRING_MAX_LEN, variableargString, RD_STRING_MAX_LEN);
	_RdPrint(outputString, lineEnd);
}

void _RdLogPrintNoPrefix(RdDbgLevel level, const char* format, ...)
{
	va_list list;
	char variableargString[RD_STRING_MAX_LEN];

	if (level > rdLogInfo.level)
	{
		if (rdLogInfo.level <= 0)
		{
			rdLogInfo.level = RdDbgLevelInfo;
		}
		else
		{
			return;
		}
	}

	va_start(list, format);
#if defined (_RD_OS_WINDOWS)
	vsprintf_s(variableargString, RD_STRING_MAX_LEN, format, list);
#else
	vsnprintf(variableargString, RD_STRING_MAX_LEN, format, list);
#endif
	va_end(list);

	_RdPrint(variableargString, 0);
}

RdLogInfo* RdGetLogInfo()
{
	return &rdLogInfo;
}

void RdSetLogInfo(RdLogType logType, RdDbgLevel logLevel, const char* logFileName)
{
	char currentTime[255];

	rdLogInfo.level = logLevel;
	rdLogInfo.type = logType;
	RD_DBG("log level: %d type: %d", rdLogInfo.level, rdLogInfo.type);
	if (rdLogInfo.type == RdLogTypeFile || rdLogInfo.type == RdLogTypeBoth)
	{
		if (logFileName && RdStringLen(logFileName, RD_PATH_MAX_LEN) > 0)
		{
			RdStringCopy(rdLogInfo.logFileName, sizeof(rdLogInfo.logFileName) - 1, logFileName, RD_PATH_MAX_LEN);
		}
		else if (RdStringLen(rdLogInfo.logFileName, sizeof(rdLogInfo.logFileName)) == 0)
		{
#if defined (_RD_OS_WINDOWS)
			SYSTEMTIME str_t;
			GetLocalTime(&str_t);
			sprintf_s(currentTime, sizeof(currentTime), "log_%d-%02d-%02d_%02d-%02d-%02d.log", str_t.wYear, str_t.wMonth, str_t.wDay, str_t.wHour,
					str_t.wMinute, str_t.wSecond);
#else
			struct tm *dateandtime;
			struct timeval tv;
			struct timezone tz;
			gettimeofday(&tv, &tz);
			dateandtime = localtime(&tv.tv_sec);

			snprintf(currentTime, sizeof(currentTime), "log_%d-%02d-%02d_%02d-%02d-%02d.log", (1900 + dateandtime->tm_year), dateandtime->tm_mon,
					dateandtime->tm_mday, dateandtime->tm_hour, dateandtime->tm_min, dateandtime->tm_sec);
#endif
			RdFsMakePath(rdLogInfo.logFileName, sizeof(rdLogInfo.logFileName) - 1, ".", 1, currentTime, RD_STRING_MAX_LEN);
			RD_DBG("log fileName: %s", rdLogInfo.logFileName);
		}
	}
}

int RdInitLogInfo(int argc, char **list)
{
	int argsCount;
	RdLogInfo* currentValueBeforeInit = RdGetLogInfo();
	RdLogType level = currentValueBeforeInit->level;
	RdLogType type = currentValueBeforeInit->type;
	const char* logFileName = NULL;

	//parse command line
	for (argsCount = 1; argsCount < argc; argsCount++)
	{
		if ((RdStringCompare(list[argsCount], RD_STRING_MAX_LEN, "--loglevel", RD_STRING_MAX_LEN) == 0)
				|| (RdStringCompare(list[argsCount], RD_STRING_MAX_LEN, "-l", RD_STRING_MAX_LEN) == 0))
		{
			if (argc > (++argsCount))
			{
				RdUInt32 debugLevel;
				debugLevel = RdStringToInt(list[argsCount], RD_STRING_MAX_LEN);

				if (debugLevel < RdDbgLevelDefault || debugLevel > RdDbgLevelDbg2)
				{
					level = RdDbgLevelInfo;
				}
				else
				{
					level = debugLevel;
				}
			}
			else
			{
				RD_ERR("Log level argument value not provided");
				return RdDebugLevelNotProvided;
			}
		}
		else if ((RdStringCompare(list[argsCount], RD_STRING_MAX_LEN, "--logtype", RD_STRING_MAX_LEN) == 0)
				|| (RdStringCompare(list[argsCount], RD_STRING_MAX_LEN, "-t", RD_STRING_MAX_LEN) == 0))
		{
			if (argc > (++argsCount))
			{
				RdUInt32 typeInt;
				typeInt = RdStringToInt(list[argsCount], RD_STRING_MAX_LEN);
				if (typeInt < RdLogTypeConsole || typeInt > RdLogTypeBoth)
				{
					typeInt = RdLogTypeConsole;
				}
				else
				{
					type = typeInt;
				}
			}
			else
			{
				RD_ERR("Log type argument value not provided");
				return RdLogTypeNotProvided;
			}
		}
		else if ((RdStringCompare(list[argsCount], RD_STRING_MAX_LEN, "--logfilename", RD_STRING_MAX_LEN) == 0)
				|| (RdStringCompare(list[argsCount], RD_STRING_MAX_LEN, "-f", RD_STRING_MAX_LEN) == 0))
		{
			if (argc > (++argsCount))
			{
				logFileName = list[argsCount];
			}
			else
			{
				RD_ERR("Log file name not provided");
				return RdLogFileNotProvided;
			}
		}
	}

	RdSetLogInfo(type, level, logFileName);
	return RdErrorSuccess;
}

//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux

#ifndef __RIPDRAW_CBASE_H__
#define __RIPDRAW_CBASE_H__

#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <stdio.h>
#include <memory.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <memory.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#endif
#include <stdarg.h>


#if defined(_WIN32) || defined(_WIN64)
#define _RD_OS_WINDOWS 1
#endif

#define RD_STRING_MAX_LEN 2048

// define path maximum len
#if defined(MAX_PATH)
// windows
#define RD_PATH_MAX_LEN	MAX_PATH
#elif defined(PATH_MAX)
// linux
#define RD_PATH_MAX_LEN	PATH_MAX
#else
#define RD_PATH_MAX_LEN	1024
#endif

#if defined(_RD_OS_WINDOWS)
#define RD_PATH_SEPARATOR		'\\'
#define RD_PATH_SEPARATOR_STR	"\\"
#else
#define RD_PATH_SEPARATOR		'/'
#define RD_PATH_SEPARATOR_STR	"/"
#endif

#if defined(_WIN32) || defined(_WIN64)
#define _RD_OS_WINDOWS 1
#endif

// define path maximum len
#if defined(MAX_PATH)
// windows
#define RD_PATH_MAX_LEN	MAX_PATH
#elif defined(PATH_MAX)
// linux
#define RD_PATH_MAX_LEN	PATH_MAX
#else
#define RD_PATH_MAX_LEN	1024
#endif

#if defined(_RD_OS_WINDOWS)
#define RD_PATH_SEPARATOR		'\\'
#define RD_PATH_SEPARATOR_STR	"\\"
#else
#define RD_PATH_SEPARATOR		'/'
#define RD_PATH_SEPARATOR_STR	"/"
#endif

#endif

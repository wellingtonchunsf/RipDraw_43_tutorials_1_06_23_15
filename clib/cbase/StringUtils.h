//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux

#ifndef __RIPDRAW_STRING_H__
#define __RIPDRAW_STRING_H__

#include "_cbase.h"

#ifdef  __cplusplus
extern "C"
{
#endif

int RdStringLen(const char* src, int srcMaxSize);
void RdStringCopy(char* dst, int destMaxSize, const char* src, int srcMaxSize);
void RdStringCat(char* dst, int dstMaxSize, const char* src, int srcMaxSize);
int RdStringCompare(const char* left, int leftMaxSize, const char* right, int rightMaxSize);
char* RdFindLastChar(char* Input, int MaxSize, const char Ch);
int RdStringToInt(char* Input, int MaxSize);

void RdFsMakePath(char* dst, int dstMaxSize, const char* path1, int path1MaxSize, const char* path2, int path2MaxSize);

#ifdef  __cplusplus
}
#endif

#endif


//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux
//
//

#include "StringUtils.h"

int RdStringLen(const char* src, int srcMaxSize)
{
	int len;

	if (!src || srcMaxSize <= 0)
	{
		return 0;
	}

	for (len = 0; len < srcMaxSize && *src; len++, src++)
	{
		;
	}
	return len;
}

void RdStringCopy(char* dst, int destMaxSize, const char* src, int srcMaxSize)
{
	char* tmp = dst;
	// check pointers and size
	if ((!dst) || (destMaxSize <= 0) || (!src) || (srcMaxSize <= 0))
	{
		return;
	}
	// copy string
	while (((*tmp++ = *src++) != 0) && --destMaxSize > 0 && --srcMaxSize > 0)
	{
	}
	// put null, the loop breaks due to size
	if (destMaxSize == 0 || srcMaxSize == 0)
	{
		if (srcMaxSize == 0 && destMaxSize > 0)
		{
			// set null, dest had more space
			*tmp = 0;
		}
		else
		{
			// set null at previous position
			*--tmp = 0;
		}
	}
}

void RdStringCat(char* dst, int dstMaxSize, const char* src, int srcMaxSize)
{
	char* tmp = dst;
	// check pointers and size
	if ((!dst) || (dstMaxSize <= 0) || (!src) || (srcMaxSize <= 0))
	{
		return;
	}
	// skip exsting string
	while ((*tmp != 0) && --dstMaxSize > 0)
	{
		tmp++;
	}

	// copy new string
	while (((*tmp++ = *src++) != 0) && --dstMaxSize > 0 && --srcMaxSize > 0)
	{
	}

	// put null, the loop breaks due to size
	if (dstMaxSize == 0 || srcMaxSize == 0)
	{
		if (srcMaxSize == 0 && dstMaxSize > 0)
		{
			// set null, dest had more space
			*tmp = 0;
		}
		else
		{
			// set null at previous position
			*--tmp = 0;
		}
	}
}

int RdStringCompare(const char* left, int leftMaxSize, const char* right, int rightMaxSize)
{
	int Ret = 0;
	// check pointers and size
	if ((!left) || (leftMaxSize <= 0) || (!right) || (rightMaxSize <= 0))
	{
		return (!left || leftMaxSize == 0) ? ((!right || rightMaxSize == 0) ? 0 : -1) : 1;
	}
	// compare string

	while ((Ret = *left - *right) == 0 && *left && *right)
	{
		// is it break due to short length
		if (leftMaxSize-- <= 0)
		{
			return -1;
		}

		if (rightMaxSize-- <= 0)
		{
			return 1;
		}
		left++;
		right++;
	}
	return (Ret == 0) ? 0 : ((Ret > 0) ? 1 : -1);
}

char* RdFindLastChar(char* Input, int MaxSize, const char Ch)
{
	int len;
	char* tmp;
	// check pointers and size
	if ((!Input) || (MaxSize <= 0))
	{
		return NULL ;
	}
	len = RdStringLen(Input, MaxSize);
	tmp = Input + len;
	if (len <= 0)
	{
		return NULL ;
	}
	// serch char into string.
	while (len-- > 0)
	{
		tmp--;
		if (*tmp == Ch)
		{
			return tmp;
		}
	}
	return NULL ;
}

int RdStringToInt(char* Input, int MaxSize)
{
	int IsNegative = 0;
	char *tmp = Input;
	long digit;
	long ret = 0;

	if (MaxSize > 0 && *tmp == '-')
	{
		MaxSize--;
		tmp++;
		IsNegative = 1;
	}
	// return value, must init to 0

	while (MaxSize-- > 0 && *tmp)
	{
		digit = *tmp - ('0');

		if (digit < 0 || digit > 9)
		{
			return 0;
		}

		ret = (ret * 10) + digit;
		tmp++;
	}
	if (IsNegative == 1)
	{
		return (ret * -1);
	}
	return ret;
}

void RdFsMakePath(char* dst, int dstMaxSize, const char* path1, int path1MaxSize, const char* path2, int path2MaxSize)
{
	int path1Len = RdStringLen(path1, path1MaxSize);
	RdStringCopy(dst, dstMaxSize, path1, path1MaxSize);

	if (dst[path1Len - 1] != RD_PATH_SEPARATOR)
	{
		RdStringCat(dst, dstMaxSize, RD_PATH_SEPARATOR_STR, 1);
	}
	RdStringCat(dst, dstMaxSize, path2, path2MaxSize);
}

//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux

#include "RipdrawCommon.h"

int RdClose(RdInterface* rdi)
{
#if defined(_RD_CLIB_STANDALONE)
	RdReleaseStandalone(rdi);
#else
	if (rdi->writebuffer.ptr)
	{
		free(rdi->writebuffer.ptr);
		rdi->writebuffer.ptr = NULL;
	}
	if (rdi->readbuffer.ptr)
	{
		free(rdi->readbuffer.ptr);
		rdi->readbuffer.ptr = NULL;
	}
	rdi->close(rdi);
#endif
	return RdErrorSuccess;
}

RdPosition RdCreatePosition(RdUWord x, RdUWord y)
{
	RdPosition ret;
	ret.x = x;
	ret.y = y;
	return ret;
}

RdSize RdCreateSize(RdUWord width, RdUWord height)
{
	RdSize ret;
	ret.width = width;
	ret.height = height;
	return ret;
}

RdColor RdCreateColor(RdByte red, RdByte green, RdByte blue, RdByte alpha)
{
	RdColor ret;
	ret.rgba.red = red;
	ret.rgba.green = green;
	ret.rgba.blue = blue;
	ret.rgba.alpha = alpha;
	return ret;
}


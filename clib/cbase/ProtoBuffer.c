//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux
//
//


#include "ProtoBuffer.h"
#include "Log.h"

// calculate the checksum
RdUWord RdChecksum(RdByte* data, int length)
{
	//calculate checksum
	int i;
	RdUWord ret = 0;
	for (i = 0; i < length; i++)
	{
		ret += data[i];
	}
	return ret;
}

// allocate buffer bytes
RdStatus RdBufferCheckAndAllocate(RdInterfaceBuffer* buffer, int requiredCapacity)
{
	RdByte* tmp;
	if (buffer == NULL)
	{
		RD_ERR("buffer should not NULL");
		return RdErrorBufferNull;
	}
	// required space available in buffer, if not then allocate it
	if ((buffer->ptr) && (buffer->capacity >= requiredCapacity))
	{
		return 0;
	}
	requiredCapacity = (requiredCapacity < 32) ? 32 : requiredCapacity;
	tmp = (RdByte*) malloc(requiredCapacity + buffer->capacity);
	if (!tmp)
	{
		RD_ERR("Insufficient resource");
		return RdErrorInsufficientMemory;
	}
	// copy old ptr to new ptr
	if ((buffer->ptr) && (buffer->capacity > 0))
	{
		memcpy(tmp, buffer->ptr, buffer->capacity);
		buffer->ptr = NULL;
		free(buffer->ptr);
		buffer->ptr = NULL;
	}
	buffer->ptr = tmp;
	buffer->capacity = requiredCapacity;
	return 0;
}

// write bytes
RdStatus RdWriteByte(RdInterfaceBuffer* buffer, RdByte input)
{
	RdStatus ret;

	ret = RdBufferCheckAndAllocate(buffer, buffer->size + 1);
	if (ret != 0)
	{
		return ret;
	}
	*(buffer->ptr + buffer->size) = input;
	buffer->size++;
	return 0;
}

// write uword
RdStatus RdWriteUWord(RdInterfaceBuffer* buffer, RdUWord input)
{
	RdStatus ret;

	ret = RdBufferCheckAndAllocate(buffer, buffer->size + 2);
	if (ret != 0)
	{
		return ret;
	}

#ifndef _RA_ARCH_BIG_ENDIAN
	*((RdUWord*) (buffer->ptr + buffer->size)) = input;
#else
	Byte* tmp = (Byte*)&input;
	*((RdUWord*)(buffer->ptr + buffer->size)) = tmp + 1;
	*((RdUWord*)(buffer->ptr + buffer->size + 1)) = tmp;
#endif

	buffer->size += 2;

	return 0;
}

// write uint32
RdStatus RdWriteUInt32(RdInterfaceBuffer* buffer, RdUInt32 input)
{
	RdStatus ret;

	ret = RdBufferCheckAndAllocate(buffer, buffer->size + 4);
	if (ret != 0)
	{
		return ret;
	}

#ifndef _RA_ARCH_BIG_ENDIAN
	*((RdUInt32*) (buffer->ptr + buffer->size)) = input;
#else
	Byte* tmp = (Byte*)&input;
	*((RdUInt32*)(buffer->ptr + buffer->size)) = tmp + 3;
	*((RdUInt32*)(buffer->ptr + buffer->size + 1)) = tmp + 2;
	*((RdUInt32*)(buffer->ptr + buffer->size + 2)) = tmp + 1;
	*((RdUInt32*)(buffer->ptr + buffer->size + 3)) = tmp;
#endif

	buffer->size += 4;

	return 0;
}

// write flag
RdStatus RdWriteFlag(RdInterfaceBuffer* buffer, RdFlag input)
{
	RdStatus ret;
	RdByte tmp = (input != RdFlagFalse) ? 1 : 0;
	ret = RdWriteByte(buffer, tmp);
	if (ret != 0)
	{
		return ret;
	}
	return 0;
}

// write position
RdStatus RdWritePosition(RdInterfaceBuffer* buffer, RdPosition input)
{
	RdStatus ret;
	ret = RdWriteUWord(buffer, input.x);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(buffer, input.y);
	if (ret != 0)
	{
		return ret;
	}

	return 0;
}

// write size
RdStatus RdWriteSize(RdInterfaceBuffer* buffer, RdSize input)
{
	RdStatus ret;
	ret = RdWriteUWord(buffer, input.width);
	if (ret != 0)
	{
		return ret;
	}

	ret = RdWriteUWord(buffer, input.height);
	if (ret != 0)
	{
		return ret;
	}
	return 0;
}

// write color
RdStatus RdWriteColor(RdInterfaceBuffer* buffer, RdColor input)
{
	RdStatus ret;
	ret = RdWriteByte(buffer, input.rgba.red);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteByte(buffer, input.rgba.green);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteByte(buffer, input.rgba.blue);
	if (ret != 0)
	{
		return ret;
	}

	ret = RdWriteByte(buffer, input.rgba.alpha);
	if (ret != 0)
	{
		return ret;
	}
	return 0;
}

// write hdirection
RdStatus RdWriteHDirection(RdInterfaceBuffer* buffer, RdHDirection input)
{
	RdStatus ret;
	RdByte tmp = (input != RdHDirectionLeft) ? 1 : 0;
	ret = RdWriteByte(buffer, tmp);
	if (ret != 0)
	{
		return ret;
	}
	return 0;
}

// write direction
RdStatus RdWriteDirection(RdInterfaceBuffer* buffer, RdDirection input)
{
	RdStatus ret;
	RdByte tmp = (input != RdDirectionHorizontal) ? 1 : 0;
	ret = RdWriteByte(buffer, tmp);
	if (ret != 0)
	{
		return ret;
	}
	return 0;
}

// write string
RdStatus RdWriteString(RdInterfaceBuffer* buffer, const char* input)
{
	RdStatus ret;
	int len = strlen(input);
	ret = RdWriteUWord(buffer, (RdUWord)len);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdBufferCheckAndAllocate(buffer, buffer->size + len);
	if (ret != 0)
	{
		return ret;
	}
	memcpy(buffer->ptr + buffer->size, input, len);
	buffer->size += len;
	return 0;
}

// read Byte
RdStatus RdReadByte(RdInterfaceBuffer* buffer, RdByte* output)
{
	int checkBytes;

	checkBytes = buffer->position + 1;
	if (buffer->ptr == NULL || (buffer->size < checkBytes))

	{
		RD_ERR("invalid response received");
		return RdErrorInvalidResponse;
	}

	*output = *((RdByte*) (buffer->ptr + buffer->position));
	buffer->position++;

	return 0;

}

// read Word
RdStatus RdReadUWord(RdInterfaceBuffer* buffer, RdUWord* output)
{
	int checkBytes;

	checkBytes = buffer->position + 2;
	if (buffer->ptr == NULL || (buffer->size < checkBytes))
	{
		RD_ERR("invalid response received");
		return RdErrorInvalidResponse;
	}
#ifndef _RA_ARCH_BIG_ENDIAN
	*output = *((RdUWord*) (buffer->ptr + buffer->position));
#else
	Byte* tmp1 = buffer->ptr + buffer->position;
	Byte* tmp2 = (Byte*)output;
	tmp2[0] = tmp1[1];
	tmp2[1] = tmp1[0];
#endif

	buffer->position += 2;

	return 0;
}

RdStatus RdReadUInt32(RdInterfaceBuffer* buffer, RdUInt32* output)
{
	int checkBytes;

	checkBytes = buffer->position + 4;
	if (buffer->ptr == NULL || (buffer->size < checkBytes))
	{
		RD_ERR("invalid response received");
		return RdErrorInvalidResponse;
	}
#ifndef _RA_ARCH_BIG_ENDIAN
	*output = *((RdUInt32*) (buffer->ptr + buffer->position));
#else
	Byte* tmp1 = buffer->ptr + buffer->position;
	Byte* tmp2 = (Byte*)output;
	tmp2[0] = tmp1[3];
	tmp2[1] = tmp1[2];
	tmp2[2] = tmp1[1];
	tmp2[3] = tmp1[0];
#endif

	buffer->position += 4;

	return 0;
}
// read Flag
RdStatus RdReadFlag(RdInterfaceBuffer* buffer, RdFlag output)
{
	RdStatus ret;
	RdByte tmp;
	ret = RdReadByte(buffer, &tmp);
	if (ret != 0)
	{
		return ret;
	}
	output = tmp == 0 ? RdFlagFalse : RdFlagTrue;
	return 0;
}

// read position
RdStatus RdReadPosition(RdInterfaceBuffer* buffer, RdPosition* output)
{
	RdStatus ret;
	RdUWord x;
	RdUWord y;
	ret = RdReadUWord(buffer, &x);
	if (ret != 0)
	{
		return ret;
	}

	ret = RdReadUWord(buffer, &y);
	if (ret != 0)
	{
		return ret;
	}

	output->x = x;
	output->y = y;
	return 0;
}

// read size
RdStatus RdReadSize(RdInterfaceBuffer* buffer, RdSize* output)
{
	RdStatus ret;
	RdUWord w;
	RdUWord h;
	ret = RdReadUWord(buffer, &w);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdReadUWord(buffer, &h);
	if (ret != 0)
	{
		return ret;
	}

	output->width = w;
	output->height = h;
	return 0;
}

// read color
RdStatus RdReadColor(RdInterfaceBuffer* buffer, RdColor* output)
{
	unsigned char red, green, blue, Alpha;
	RdStatus ret = 0;

	ret = RdReadByte(buffer, &red);
	if (ret != 0)
	{
		return ret;
	}

	ret = RdReadByte(buffer, &green);
	if (ret < 0)
	{
		return ret;
	}

	ret = RdReadByte(buffer, &blue);
	if (ret < 0)
	{
		return ret;
	}

	ret = RdReadByte(buffer, &Alpha);
	if (ret < 0)
	{
		return ret;
	}
	//output->rgbargba.red=red;
	output->rgba.red = red;
	output->rgba.green = green;
	output->rgba.blue = blue;
	output->rgba.alpha = Alpha;
	return 0;
}

// read HDirection
RdStatus RdReadHDirection(RdInterfaceBuffer* buffer, RdHDirection* output)
{
	RdByte value;
	RdStatus ret = 0;
	ret = RdReadByte(buffer, &value);
	if (ret != 0)
	{
		return ret;
	}

	*output = (value == 0) ? RdHDirectionLeft : RdHDirectionRight;
	return 0;
}

// read Direction
RdStatus RdReadDirection(RdInterfaceBuffer* buffer, RdDirection* output)
{
	RdStatus ret = 0;
	RdByte value;
	ret = RdReadByte(buffer, &value);
	if (ret != 0)
	{
		return ret;
	}

	*output = (value == 0) ? RdDirectionHorizontal : RdDirectionVertical;
	return 0;
}

// read String
RdStatus RdReadString(RdInterfaceBuffer* buffer, char** output)
{
	RdStatus ret;
	int checkBytes;
	RdUWord length;
	char* tmp = NULL;

	ret = RdReadUWord(buffer, &length);
	if (ret != 0)
	{
		return ret;
	}

	checkBytes = buffer->position + length;
	if (buffer->ptr == NULL || (buffer->size < checkBytes))
	{
		RD_ERR("invalid response received");
		return RdErrorInvalidResponse;
	}
	tmp = (char*) malloc(length + 1);
	if (!tmp)
	{
		RD_ERR("unable to allocate memory");
		return RdErrorInsufficientMemory;
	}
	memcpy(tmp, buffer->ptr + buffer->position, length);
	*(tmp + length) = '\0';
	*output = tmp;
	buffer->position += length;
	return 0;
}

int RdBufferDump(const char* label, RdInterfaceBuffer* buffer, int printIfNonZero)
{
	int i = 0;
	if (printIfNonZero)
	{
		// print only we have non zero value
		for (i = 0; i < buffer->size; i++)
		{
			if (buffer->ptr[i] != 0x00)
			{
				break;
			}
		}
		if (i >= buffer->size)
		{
			return 0;
		}
	}

	RD_DBG("%s: %d", label, buffer->size);
	for (i = 0; i < buffer->size; i++)
	{
		RDLOG_NO_PREFIX(RdDbgLevelDbg1, " 0x%02X", buffer->ptr[i]);
	}
	RDLOG_NO_PREFIX(RdDbgLevelDbg1, "\n");
	return 1;
}

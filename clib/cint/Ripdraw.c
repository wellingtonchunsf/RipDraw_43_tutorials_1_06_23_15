//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux

#include "../common/Ripdraw.h"

#define RD_PROTO_POS_CMD			0
#define RD_PROTO_POS_SEQ			2
#define RD_PROTO_POS_PL				4
#define RD_PROTO_POS_BYTE_0			6
#define RD_PROTO_POS_BYTE_1			8

RdStatus RdSetRetryCntAndDelay(RdInterface* rd_interface, int ResponseRetryCount, int ResponseRetryDelay)
{
	rd_interface->ResponseRetryCount = ResponseRetryCount;
	rd_interface->ResponseRetryDelay = ResponseRetryDelay;
	return RdErrorSuccess;
}

RdStatus RdSetDelay(RdInterface* rd_interface, int writeDelay, int cmdDelay)
{
	rd_interface->writeDelay = writeDelay;
	rd_interface->cmdDelay = cmdDelay;
	return RdErrorSuccess;
}

RdStatus RdFlushSetDefaults(RdInterface* rd_interface, int len)
{
	RdByte flushData = 0x00;
	RD_SLEEP(1000);
	while (len--)
	{
		rd_interface->write(rd_interface, &flushData, 1);
	}
	//set default delay
	RdSetDelay(rd_interface, 0, 0);
	//set default Response Retry count and delay
	RdSetRetryCntAndDelay(rd_interface, RD_INTERFACE_RESPONSE_RETRY_COUNT_DEFAULT, RD_INTERFACE_RESPONSE_RETRY_DELAY_DEFAULT);
	return RdErrorSuccess;
}

RdStatus RdInitRequest(RdInterface* rdi, RdCommand cmdId)
{
	RdStatus ret;

	RD_DBG("IN");
	_RD_CHECK_INTERFACE();

	rdi->writebuffer.size = 0;
	rdi->readbuffer.size = 0;
	// add command id
	rdi->lastCmdId = cmdId;
	ret = RdWriteUWord(&rdi->writebuffer, (RdUWord)cmdId);
	if (ret != 0)
	{
		return ret;
	}
	// add sequence number
	rdi->seqNo++;
	if (rdi->seqNo > 65535)
	{
		rdi->seqNo = 1;
	}
	ret = RdWriteUWord(&rdi->writebuffer, (RdUWord)rdi->seqNo);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	// add dummy payload length, later on while sending update it
	return RdWriteUWord(&rdi->writebuffer, 0);
}

RdStatus RdProcessResponse(RdInterface* rdi)
{
	RdStatus ret;
	int retryCount;
	int retrySleep;
	RdId cmdId;
	RdUWord seqNo;
	RdUWord checksum;
	RdUWord responseChecksum;
	RdUWord payloadLen;
	RdLogInfo* logInfo;

	RD_DBG("IN");
	_RD_CHECK_INTERFACE();
	logInfo = RdGetLogInfo();

	ret = RdBufferCheckAndAllocate(&rdi->readbuffer, 16);
	if (ret != 0)
	{
		return ret;
	}

	retryCount = 0;
	retrySleep = 0;
retry:
	// read command id and check with last id
	if (retryCount == 0)
	{
		ret = (RdStatus)rdi->read(rdi, rdi->readbuffer.ptr, 2);
	}
	else
	{
		// sleep to give time device for processing
		RD_SLEEP(retrySleep);
		// remove first byte and read next byte
		rdi->readbuffer.ptr[0] = rdi->readbuffer.ptr[1];
		ret = (RdStatus)rdi->read(rdi, rdi->readbuffer.ptr + 1, 1);
	}
	if (ret != 0)
	{
		return ret;
	}
	rdi->readbuffer.size = 2;

	if (logInfo->level >= RdDbgLevelDbg2)
	{
		RdBufferDump("Response Buffer", &rdi->readbuffer, 1);
	}

	rdi->readbuffer.position = RD_PROTO_POS_CMD;
	ret = RdReadUWord(&rdi->readbuffer, &cmdId);
	if (ret != 0)
	{
		return ret;
	}
	if (rdi->lastCmdId != cmdId)
	{
		if (retryCount > rdi->ResponseRetryCount)
		{
			RD_ERR("timeout, not response from device");
			return RdErrorResponseTimeOut;
		}
		retryCount++;
		retrySleep = rdi->ResponseRetryDelay;
		RD_DBG("retryCount: %d retrySleep: %d", retryCount, retrySleep);
		goto retry;
	}

	// read sequence number and payload length
	ret = (RdStatus)rdi->read(rdi, rdi->readbuffer.ptr + 2, 4);
	if (ret != 0)
	{
		return ret;
	}
	rdi->readbuffer.size = RD_PROTO_POS_BYTE_0;

	if (logInfo->level >= RdDbgLevelDbg2)
	{
		RdBufferDump("Response Buffer", &rdi->readbuffer, 0);
	}

	// check seq
	rdi->readbuffer.position = RD_PROTO_POS_SEQ;
	ret = RdReadUWord(&rdi->readbuffer, &seqNo);
	if (ret != 0)
	{
		return ret;
	}
	if (rdi->seqNo != seqNo)
	{
		RD_ERR("sequence number not match %d != %d", rdi->seqNo, seqNo);
		return RdErrorSeqNoMissmatch;
	}
	// extract payload length
	rdi->readbuffer.position = RD_PROTO_POS_PL;

	ret = RdReadUWord(&rdi->readbuffer, &payloadLen);

	if (ret != 0)
	{
		return ret;
	}
	// required space available in response buffer, if not then allocate it
	ret = RdBufferCheckAndAllocate(&rdi->readbuffer, payloadLen + RD_PROTO_POS_BYTE_1);
	if (ret != 0)
	{
		return ret;
	}

	// read payload and checksum
	ret = (RdStatus)rdi->read(rdi, rdi->readbuffer.ptr + RD_PROTO_POS_BYTE_0, payloadLen + 2);
	if (ret != 0)
	{
		return ret;
	}
	rdi->readbuffer.size = payloadLen + RD_PROTO_POS_BYTE_0 + 2;

	if (logInfo->level >= RdDbgLevelDbg2)
	{
		RdBufferDump("Response Buffer", &rdi->readbuffer, 0);
	}

	// extract checksum
	rdi->readbuffer.position = payloadLen + RD_PROTO_POS_BYTE_0;
	ret = RdReadUWord(&rdi->readbuffer, &responseChecksum);
	if (ret != 0)
	{
		return ret;
	}
	// validate checksum
	checksum = RdChecksum(rdi->readbuffer.ptr, payloadLen + RD_PROTO_POS_BYTE_0);
	if (responseChecksum != checksum)
	{
		RD_ERR("response checksum not match");
		return RdErrorChecksumMissmatch;
	}
	// extract status
	rdi->readbuffer.position = RD_PROTO_POS_BYTE_0;
	ret = RdReadUWord(&rdi->readbuffer, &rdi->lastResponseStatus);
	if (ret != 0)
	{
		return ret;
	}

	if (rdi->lastResponseStatus != 0)
	{
		RD_ERR("device returns failed status: 0x%X", rdi->lastResponseStatus);
		return rdi->lastResponseStatus;
	}

	RD_DBG("OUT");
	RD_SLEEP(rdi->cmdDelay);
	return RdErrorSuccess;
}

RdStatus RdProcessRequest(RdInterface* rdi)
{
	RdStatus ret;
	RdUWord payloadLen;
	RdUWord checksum;
	RdLogInfo* logInfo;
	int retryCount;

	RD_DBG("IN");
	_RD_CHECK_INTERFACE();
	logInfo = RdGetLogInfo();

	payloadLen = (RdUWord)rdi->writebuffer.size - RD_PROTO_POS_BYTE_0;
	// update data length
	*((RdUWord*)(rdi->writebuffer.ptr + RD_PROTO_POS_PL)) = payloadLen;
	// checksum
	checksum = RdChecksum(rdi->writebuffer.ptr, rdi->writebuffer.size);
	ret = RdWriteUWord(&rdi->writebuffer, checksum);
	if (ret != 0)
	{
		return ret;
	}

	retryCount = 0;
retry:
	if (logInfo->level >= RdDbgLevelDbg2)
	{
		RdBufferDump("Request Buffer", &rdi->writebuffer, 0);
	}	
	// send to device
	ret = (RdStatus)rdi->write(rdi, rdi->writebuffer.ptr, rdi->writebuffer.size);
	if (ret != 0)
	{
		return ret;
	}

	RD_SLEEP(rdi->writeDelay);
	ret = RdProcessResponse(rdi);
	if (ret != 0)
	{
		if (retryCount == 0)
		{
			retryCount++;
			goto retry;
		}
		else
		{
			return ret;
		}
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdFreeData(void* data)
{
	if (data)
	{
		free(data);
	}
	data = NULL;

	return RdErrorSuccess;
}

RdStatus RdSetLayerEnable(RdInterface* rdi, RdId layerId, RdFlag enable)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandSetLayerEnable);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, layerId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteFlag(&rdi->writebuffer, enable);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdSetLayerOriginAndSize(RdInterface* rdi, RdId layerId, RdPosition position, RdSize size)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandSetLayerOriginAndSize);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, layerId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.x);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.y);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, size.width);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, size.height);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdSetLayerBackColor(RdInterface* rdi, RdId layerId, RdColor backColor)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandSetLayerBackColor);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, layerId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteColor(&rdi->writebuffer, backColor);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdSetLayerTransparency(RdInterface* rdi, RdId layerId, RdByte transparencyPercentage)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandSetLayerTransparency);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, layerId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteByte(&rdi->writebuffer, transparencyPercentage);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdLayerClear(RdInterface* rdi, RdId layerId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandLayerClear);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, layerId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdLayerMove(RdInterface* rdi, RdId layerId, RdUWord moveLeft, RdUWord moveTop, RdUWord moveRight, RdUWord moveBottom)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandLayerMove);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, layerId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, moveLeft);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, moveTop);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, moveRight);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, moveBottom);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdLayerWriteRawPixels(RdInterface* rdi, RdId layerId, RdPosition position, RdSize pixelSize, const RdColor* pixels)
{
	RdStatus ret;
	int pixelsLenInBytes, pixelLen;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandLayerWriteRawPixels);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, layerId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.x);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.y);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, pixelSize.width);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, pixelSize.height);
	if (ret != 0)
	{
		return ret;
	}
	pixelLen = pixelSize.height * pixelSize.width;
	ret = RdWriteUWord(&rdi->writebuffer, (RdUWord)pixelLen);
	if (ret != 0)
	{
		return ret;
	}
	pixelsLenInBytes = pixelSize.height * pixelSize.width * sizeof(RdColor);
	ret = RdBufferCheckAndAllocate(&rdi->writebuffer, rdi->writebuffer.size + pixelsLenInBytes);
	if (ret != 0)
	{
		return ret;
	}
	memcpy(rdi->writebuffer.ptr + rdi->writebuffer.size, pixels, pixelsLenInBytes);
	rdi->writebuffer.size += pixelsLenInBytes;
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdComposeLayersToPage(RdInterface* rdi, RdId pageId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandComposeLayersToPage);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, pageId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdPageToScreen(RdInterface* rdi, RdId pageId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandPageToScreen);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, pageId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdPartialComposeLayersToScreen(RdInterface* rdi, RdId pageId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandPartialComposeLayersToScreen);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, pageId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdLineGraphCreateWindow(RdInterface* rdi, RdId layerId, RdPosition position, RdSize size, RdByte lineWidth, RdByte lineGlowWidth, RdFlag autoCompose,
	RdId* graphId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandLineGraphCreateWindow);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, layerId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.x);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.y);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, size.width);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, size.height);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteByte(&rdi->writebuffer, lineWidth);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteByte(&rdi->writebuffer, lineGlowWidth);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteFlag(&rdi->writebuffer, autoCompose);
	if (ret != 0)
	{
		return ret;
	}

	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = RD_PROTO_POS_BYTE_1;

	RD_DBG("OUT");
	if (graphId == NULL)
	{
		return 0;
	}
	else
	{
		return RdReadUWord(&rdi->readbuffer, graphId);
	}
}

RdStatus RdLineGraphInsertPoints(RdInterface* rdi, RdId graphId, RdColor pointColor, RdUWord pointLength, const RdPosition* points)
{
	RdStatus ret;
	int pointsLenInBytes;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandLineGraphInsertPoints);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, graphId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteColor(&rdi->writebuffer, pointColor);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, pointLength);
	if (ret != 0)
	{
		return ret;
	}
	pointsLenInBytes = pointLength * sizeof(RdColor);
	ret = RdBufferCheckAndAllocate(&rdi->writebuffer, rdi->writebuffer.size + pointsLenInBytes);
	if (ret != 0)
	{
		return ret;
	}
	memcpy((rdi->writebuffer.ptr + rdi->writebuffer.size), points, pointsLenInBytes);

	rdi->writebuffer.size += pointsLenInBytes;
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdLineGraphMove(RdInterface* rdi, RdId graphId, RdUWord left, RdUWord top, RdUWord right, RdUWord bottom)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandLineGraphMove);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, graphId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, left);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, top);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, right);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, bottom);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdLineGraphDeleteWindow(RdInterface* rdi, RdId graphId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandLineGraphDeleteWindow);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, graphId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdBarGraphCreateWindow(RdInterface* rdi, RdId layerId, RdPosition position, RdSize size, RdByte stackSize, RdDirection stackDirection,
	RdFlag autoCompose, RdId* graphId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandBarGraphCreateWindow);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, layerId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.x);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.y);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, size.width);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, size.height);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteByte(&rdi->writebuffer, stackSize);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteByte(&rdi->writebuffer, stackDirection);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteFlag(&rdi->writebuffer, autoCompose);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = RD_PROTO_POS_BYTE_1;
	RD_DBG("OUT");

	if (graphId == NULL)
	{
		return 0;
	}
	else
	{
		return RdReadUWord(&rdi->readbuffer, graphId);
	}
}

RdStatus RdBarGraphInsertStacks(RdInterface* rdi, RdId graphId, RdByte noOfStack, RdId imageId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandBarGraphInsertStacks);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, graphId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteByte(&rdi->writebuffer, noOfStack);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, imageId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdBarGraphRemoveStacks(RdInterface* rdi, RdId graphId, RdByte noOfStack)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandBarGraphRemoveStacks);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, graphId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteByte(&rdi->writebuffer, noOfStack);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdBarGraphDeleteWindow(RdInterface* rdi, RdId graphId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandBarGraphDeleteWindow);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, graphId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdTouchMapRectangle(RdInterface* rdi, RdPosition position, RdSize size, const char* label, RdId* touchId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandTouchMapRectangle);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.x);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.y);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, size.width);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, size.height);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteString(&rdi->writebuffer, label);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = RD_PROTO_POS_BYTE_1;
	RD_DBG("OUT");

	if (touchId == NULL)
	{
		return 0;
	}
	else
	{
		return RdReadUWord(&rdi->readbuffer, touchId);
	}
}

RdStatus RdTouchMapCircle(RdInterface* rdi, RdPosition position, RdUWord outerCircleRadius, RdUWord innerCircleRadius, const char* label, RdId* touchId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandTouchMapCircle);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.x);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.y);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, outerCircleRadius);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, innerCircleRadius);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteString(&rdi->writebuffer, label);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = RD_PROTO_POS_BYTE_1;
	RD_DBG("OUT");

	if (touchId == NULL)
	{
		return 0;
	}
	else
	{
		return RdReadUWord(&rdi->readbuffer, touchId);
	}
}

RdStatus RdTouchMapDelete(RdInterface* rdi, RdId touchId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandTouchMapDelete);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, touchId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdTouchMapClear(RdInterface* rdi)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandTouchMapClear);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdReset(RdInterface* rdi)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandReset);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdTestEcho(RdInterface* rdi, const char* label, char** output)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandTestEcho);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteString(&rdi->writebuffer, label);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = 8;

	RD_DBG("OUT");
	return RdReadString(&rdi->readbuffer, output);
}

RdStatus RdSystemInfo(RdInterface* rdi, RdSystemInfoType type, char** output)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandSystemInfo);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, type);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = 8;

	RD_DBG("OUT");
	return RdReadString(&rdi->readbuffer, output);
}

RdStatus RdImageLoad(RdInterface* rdi, const char* imageLable, RdId* imageId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandImageLoad);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteString(&rdi->writebuffer, imageLable);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = RD_PROTO_POS_BYTE_1;

	RD_DBG("OUT");
	if (imageId == NULL)
	{
		return 0;
	}
	else
	{
		return RdReadUWord(&rdi->readbuffer, imageId);
	}
}

RdStatus RdImageRelease(RdInterface* rdi, RdId imageId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandImageRelease);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, imageId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdImageWrite(RdInterface* rdi, RdId layerId, RdId imageId, RdPosition position, RdId* imageWriteId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandImageWrite);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, layerId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, imageId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.x);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.y);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = RD_PROTO_POS_BYTE_1;

	RD_DBG("OUT");
	if (imageWriteId == NULL)
	{
		return 0;
	}
	else
	{
		return RdReadUWord(&rdi->readbuffer, imageWriteId);
	}
}

RdStatus RdImageDelete(RdInterface* rdi, RdId imageWriteId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandImageDelete);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, imageWriteId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdImageReplace(RdInterface* rdi, RdId imageWriteId, RdId imageReplaceId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandImageReplace);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, imageWriteId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, imageReplaceId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdImageMove(RdInterface* rdi, RdId imageWriteId, RdPosition position)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandImageMove);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, imageWriteId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.x);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.y);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdImageListLoad(RdInterface* rdi, const char* prefix, RdUWord indexStart, RdUWord indexStep, RdUWord indexCount, RdId* imageListId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandImageListLoad);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteString(&rdi->writebuffer, prefix);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, indexStart);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, indexStep);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, indexCount);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = RD_PROTO_POS_BYTE_1;

	RD_DBG("OUT");
	if (imageListId == NULL)
	{
		return 0;
	}
	else
	{
		return RdReadUWord(&rdi->readbuffer, imageListId);
	}
}

RdStatus RdImageListRelease(RdInterface* rdi, RdId imageListId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandImageListRelease);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, imageListId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdImageListWrite(RdInterface* rdi, RdId layerId, RdPosition position, RdId imageListId, RdUWord imageIndex, RdId* imageListWriteId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandImageListWrite);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, layerId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.x);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.y);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, imageListId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, imageIndex);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = RD_PROTO_POS_BYTE_1;

	RD_DBG("OUT");
	if (imageListWriteId == NULL)
	{
		return 0;
	}
	else
	{
		return RdReadUWord(&rdi->readbuffer, imageListWriteId);
	}
}

RdStatus RdImageListReplace(RdInterface* rdi, RdId imageListWriteId, RdUWord imageIndex)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandImageListReplace);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, imageListWriteId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, imageIndex);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdImageListDelete(RdInterface* rdi, RdId imageListWriteId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandImageListDelete);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, imageListWriteId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdAnimationPlay(RdInterface* rdi, RdId layerId, RdPosition position, RdId imageListId, RdUWord frequency, RdId* animationPlayId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandAnimationPlay);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, layerId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.x);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.y);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, imageListId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, frequency);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = RD_PROTO_POS_BYTE_1;

	RD_DBG("OUT");
	if (animationPlayId == NULL)
	{
		return 0;
	}
	else
	{
		return RdReadUWord(&rdi->readbuffer, animationPlayId);
	}
}

RdStatus RdAnimationStop(RdInterface* rdi, RdId animationPlayId, RdUWord stopIndex)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandAnimationStop);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, animationPlayId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, stopIndex);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdAnimationContinue(RdInterface* rdi, RdId animationPlayId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandAnimationContinue);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, animationPlayId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdAnimationDelete(RdInterface* rdi, RdId animationPlayId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandAnimationDelete);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, animationPlayId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdFontLoad(RdInterface* rdi, const char* fontLabel, RdUWord* fontId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandFontLoad);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteString(&rdi->writebuffer, fontLabel);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = RD_PROTO_POS_BYTE_1;

	RD_DBG("OUT");
	if (fontId == NULL)
	{
		return 0;
	}
	else
	{
		return RdReadUWord(&rdi->readbuffer, fontId);
	}
}

RdStatus RdFontRelease(RdInterface* rdi, RdId fontId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandFontRelease);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, fontId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdSetFontPadding(RdInterface* rdi, RdId fontId, const RdByte padding)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandSetFontPadding);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, fontId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteByte(&rdi->writebuffer, padding);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdStringWrite(RdInterface* rdi, RdId layerId, RdPosition position, RdId fontId, RdColor color, RdHDirection hDirection, const char* data,
	RdId* stringWriteId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandStringWrite);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, layerId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.x);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.y);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, fontId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteColor(&rdi->writebuffer, color);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteHDirection(&rdi->writebuffer, hDirection);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteString(&rdi->writebuffer, data);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = RD_PROTO_POS_BYTE_1;

	RD_DBG("OUT");
	if (stringWriteId == NULL)
	{
		return 0;
	}
	else
	{
		return RdReadUWord(&rdi->readbuffer, stringWriteId);
	}
}

RdStatus RdStringReplace(RdInterface* rdi, RdId stringWriteId, const char* data)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandStringReplace);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, stringWriteId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteString(&rdi->writebuffer, data);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdStringDelete(RdInterface* rdi, RdId stringWriteId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandStringDelete);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, stringWriteId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdCharacterWrite(RdInterface* rdi, RdId layerId, RdPosition position, RdId fontId, RdColor color, RdByte c, RdId* characterWriteId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandCharacterWrite);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, layerId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.x);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.y);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, fontId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteColor(&rdi->writebuffer, color);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteByte(&rdi->writebuffer, c);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = RD_PROTO_POS_BYTE_1;

	RD_DBG("OUT");
	if (characterWriteId == NULL)
	{
		return 0;
	}
	else
	{
		return RdReadUWord(&rdi->readbuffer, characterWriteId);
	}
}

RdStatus RdCharacterReplace(RdInterface* rdi, RdId characterWriteId, RdByte c)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandCharacterReplace);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, characterWriteId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteByte(&rdi->writebuffer, c);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdCharacterDelete(RdInterface* rdi, RdId characterWriteId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandCharacterDelete);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, characterWriteId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdTextWindowCreate(RdInterface* rdi, RdId layerId, RdPosition position, RdSize size, RdId fontId, RdColor fontColor, RdHDirection scrollDirection,
	RdUWord* textWindowId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandTextWindowCreate);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, layerId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.x);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.y);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, size.width);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, size.height);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, fontId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteColor(&rdi->writebuffer, fontColor);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteHDirection(&rdi->writebuffer, scrollDirection);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = RD_PROTO_POS_BYTE_1;

	RD_DBG("OUT");
	if (textWindowId == NULL)
	{
		return 0;
	}
	else
	{
		return RdReadUWord(&rdi->readbuffer, textWindowId);
	}
}

RdStatus RdTextWindowSetInsertionPoint(RdInterface* rdi, RdId textWindowId, RdPosition position)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandTextWindowSetInsertionPoint);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, textWindowId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.x);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, position.y);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdTextWindowInsertText(RdInterface* rdi, RdId textWindowId, const char* stringData)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandTextWindowInsertText);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, textWindowId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteString(&rdi->writebuffer, stringData);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdTextWindowDelete(RdInterface* rdi, RdId textWindowId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandTextWindowDelete);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, textWindowId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdGetMaxBackLightBrightness(RdInterface* rdi, RdUWord* maxBacklightBrightness)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandGetMaxBackLightBrightness);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = RD_PROTO_POS_BYTE_1;

	RD_DBG("OUT");
	return RdReadUWord(&rdi->readbuffer, maxBacklightBrightness);
}

RdStatus RdGetBackLightBrightness(RdInterface* rdi, RdUWord* backlightBrightness)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandGetBackLightBrightness);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = RD_PROTO_POS_BYTE_1;

	RD_DBG("OUT");
	return RdReadUWord(&rdi->readbuffer, backlightBrightness);
}

RdStatus RdSetBackLightBrightness(RdInterface* rdi, RdUWord backlightBrightness)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandSetBackLightBrightness);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, backlightBrightness);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdFlashWriteEnable(RdInterface* rdi, RdFlag enable)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandFlashWriteEnable);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteFlag(&rdi->writebuffer, enable);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdFlashImage(RdInterface* rdi, RdUWord type, const char* filename, RdUInt32 length, RdId* transferId)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandFlashImage);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, type);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteString(&rdi->writebuffer, filename);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUInt32(&rdi->writebuffer, length);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = RD_PROTO_POS_BYTE_1;

	RD_DBG("OUT");
	return RdReadUWord(&rdi->readbuffer, transferId);
}

RdStatus RdFlashData(RdInterface* rdi, RdId transferId, RdUWord type, const char* data, RdUWord len)
{
	RdStatus ret;
	RdUInt32 i = 0;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandFlashData);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, transferId);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, type);
	if (ret != 0)
	{
		return ret;
	}

	ret = RdWriteUWord(&rdi->writebuffer, len);
	if (ret != 0)
	{
		return ret;
	}

	for (i = 0; i < len; i++)
	{
		ret = RdWriteByte(&rdi->writebuffer, data[i]);
		if (ret != 0)
		{
			return ret;
		}
	}

	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdFlashDelete(RdInterface* rdi, RdUWord type, const char* filename)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandFlashDelete);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, type);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteString(&rdi->writebuffer, filename);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdFlashDeleteAll(RdInterface* rdi)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandFlashDeleteAll);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdSetEventType(RdInterface* rdi, RdUWord eventType)
{
	RdStatus ret;
	RD_DBG("IN");

	ret = RdInitRequest(rdi, RdCommandSetEventType);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdWriteUWord(&rdi->writebuffer, eventType);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT");
	return RdErrorSuccess;
}

RdStatus RdGetEventData(RdInterface* rdi, RdEvent** event, RdUWord* count)
{
	RdStatus ret;
	int checkBytes;
	RdUWord packetCount = 0;
	RdEvent* tempEvent;
	int i;

	RD_DBG("IN");
	_RD_CHECK_INTERFACE();

	ret = RdInitRequest(rdi, RdCommandEventMessage);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdProcessRequest(rdi);
	if (ret != 0)
	{
		return ret;
	}

	rdi->readbuffer.position = 10;

	ret = RdReadUWord(&rdi->readbuffer, &packetCount);
	if (ret != 0)
	{
		return ret;
	}

	if (packetCount > 0)
	{
		RdUWord hasMoreData = 0;
		ret = RdReadUWord(&rdi->readbuffer, &hasMoreData);
		if (ret != 0)
		{
			return ret;
		}

		tempEvent = (RdEvent*)malloc(packetCount * sizeof(RdEvent));
		if (!tempEvent)
		{
			RD_ERR("unable to allocate memory");
			return RdErrorMemoryAllocationFailed;
		}
		memset(tempEvent, 0, sizeof(RdEvent) * packetCount);

		for (i = 0; i < packetCount; i++)
		{
			RdUWord length;
			ret = RdReadUWord(&rdi->readbuffer, &length);
			if (ret != 0)
			{
				return ret;
			}

			length = length - 1;

			ret = RdReadByte(&rdi->readbuffer, &tempEvent[i].eventType);
			if (ret != 0)
			{
				return ret;
			}

			checkBytes = rdi->readbuffer.position + length;
			if (rdi->readbuffer.ptr == NULL || (rdi->readbuffer.size < checkBytes))
			{
				RD_ERR("invalid response received");
				return RdErrorInvalidResponse;
			}

			tempEvent[i].data = (RdByte*)malloc(length);
			if (!tempEvent[i].data)
			{
				RD_ERR("unable to allocate memory");
				return RdErrorMemoryAllocationFailed;
			}
			memcpy(tempEvent[i].data, rdi->readbuffer.ptr + rdi->readbuffer.position, length);
			rdi->readbuffer.position += length;
		}
		*event = tempEvent;
	}
	*count = packetCount;

	RD_DBG("OUT");
	return RdErrorSuccess;
}

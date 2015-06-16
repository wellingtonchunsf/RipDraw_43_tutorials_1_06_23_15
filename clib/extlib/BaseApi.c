//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux

#include "BaseApi.h"

RdStatus RdResourceInfoLoad(RdInterface *rdi,  RdResourceLoadInfo resourceLoadTable[], RdUInt32 arraySize, RdFlag autoDownload, const char* projectPath)
{
	RdId ret = 0,i,failedCount = 0;

	const int size = arraySize/ sizeof(RdResourceLoadInfo);
	RdResourceLoadInfo* downloadResources = (RdResourceLoadInfo*)malloc(size*sizeof(RdResourceLoadInfo));

    LoadAgain:

	for (i = 0; i < size; i++)
	{
		if (resourceLoadTable[i].resLoadType == RdResourceTypeImage)
		{
			ret = RdImageLoad(rdi, resourceLoadTable[i].resLabel, &resourceLoadTable[i].resId);
		}

		if (resourceLoadTable[i].resLoadType == RdResourceTypeFont)
		{
			ret = RdFontLoad(rdi, resourceLoadTable[i].resLabel, &resourceLoadTable[i].resId);
		}	
		if (resourceLoadTable[i].resLoadType == RdResourceTypeImageList)
		{
			ret = RdImageListLoad(rdi, resourceLoadTable[i].resLabel, resourceLoadTable[i].imageListIndexStart, resourceLoadTable[i].imageListIndexStep, resourceLoadTable[i].imageListIndexCount, &resourceLoadTable[i].resId);
		}

		if (ret != RdErrorSuccess)
		{
			if (!autoDownload)
			{
				if(ret == RdErrorImageInvalidLabel)
				{
					RD_ERR("RdImageLoadInfo failed : %s", resourceLoadTable[i].resLabel);
				}
				return ret;
			}
			else
			{
				downloadResources[failedCount].resLabel = resourceLoadTable[i].resLabel;
				downloadResources[failedCount].resLoadType = resourceLoadTable[i].resLoadType;
				failedCount++;
			}
		}
	}

	if (autoDownload && failedCount > 0)
	{
	   RdReset(rdi);

	   //will check for available space for all file's total size and if memory not available it will return
	   ret = RdDownloadUnavailableResources(rdi,downloadResources,projectPath,failedCount);
	   if(ret != 0)
	   {
		    return ret;
	   }
	   else
	   {
		    failedCount =0;
		    goto LoadAgain;
	   }
	   return ret;
	}
    return ret;
}


RdId RdResourceInfoTableGetId(RdResourceLoadInfo resourceLoadTable[], RdUInt32 arraySize, const char* resLabel)
{
	int i;
	int size = arraySize / sizeof(RdResourceLoadInfo);
	for (i = 0; i < size; i++)
	{
 		if (RdStringCompare(resLabel ,RD_PATH_MAX_LEN,  resourceLoadTable[i].resLabel, RD_PATH_MAX_LEN) == 0)
		{
			return resourceLoadTable[i].resId;
		}
	}
	return 0;
}


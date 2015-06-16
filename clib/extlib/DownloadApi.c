//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux

#include "DownloadApi.h"

#if defined(_RD_CLIB_STANDALONE)
RdStatus RdDeviceDataDownloadFile(RdInterface* rdi, const char* dirPath, const char* fileName)
{
	return 0;
}
RdStatus RdDeviceDataDownloadAll(RdInterface* rdi, const char* dirPath)
{
	return 0;
}
RdStatus RdDeviceDeleteAll(RdInterface* rdi)
{
	return 0;
}
RdStatus RdDownloadUnavailableResources(RdInterface *rdi,RdResourceLoadInfo downloadResources[],const char* projectPath,RdId failedCount)
{
	return 0;
}
#else
void _GetDirectory(char* dst, RdId dstMaxSize, const char* path, RdId pathMaxSize)
{
	char *lastStr;
	RdStringCopy(dst, dstMaxSize, path, pathMaxSize);

	lastStr = RdFindLastChar(dst, RD_PATH_MAX_LEN, RD_PATH_SEPARATOR);
	if (lastStr != NULL)
	{
		*lastStr = '\0';
	}
	lastStr = RdFindLastChar(dst, RD_PATH_MAX_LEN, RD_PATH_SEPARATOR);
	RdStringCopy(dst, dstMaxSize, (lastStr + 1), pathMaxSize);
}

void _GetTitle(char* dst, RdId dstMaxSize, const char* path, RdId pathMaxSize)
{
	char *lastDot;
	RdStringCopy(dst, dstMaxSize, path, pathMaxSize);

	lastDot = strrchr(dst, '.');
	if (lastDot != NULL)
	{
		*lastDot = '\0';
	}
}

void _GetExtension(char* dst, RdId dstMaxSize, const char* path, RdId pathMaxSize)
{
	char* ext;
	RdStringCopy(dst, dstMaxSize, path, pathMaxSize);

	ext = strrchr(dst, '.');
	if (ext != NULL)
	{
		RdStringCopy(dst, dstMaxSize, ext, pathMaxSize);
	}
}

RdStatus _WriteData(RdInterface* rdi, RdId transferId, RdByte* data, RdUInt32 fileLength)
{
	RdStatus ret = 0;
	// 4096 - 8 (payload len + command id + type + checksum)
	const RdUInt32 maxSendSize = 4088;
	RdUInt32 pointer = 0;
	while (pointer < fileLength)
	{
		RdUInt32 remainLen = fileLength - pointer;
		RdUWord writeLen = (remainLen > maxSendSize) ? maxSendSize : remainLen;

		ret = RdFlashData(rdi, transferId, RdFlashDataTypeChunk, data + pointer, writeLen);
		if (ret != 0)
		{
			return ret;
		}
		// update returned length
		pointer += writeLen;
	}
	return ret;
}

RdStatus _RdDeviceDataSendFileToDevice(RdInterface* rdi, RdId type, const char* filePath, char* fileName)
{
	RdStatus ret = 0;
	RdUInt32 fileLength;
	FILE* fd = NULL;
	RdByte* fileBuffer;
	RdId transferId;

#if _RD_OS_WINDOWS
	HANDLE handle;
	DWORD HighPart = 0;
	DWORD bytesRead = 0;
	BOOL res;
	DWORD desiredAccess = GENERIC_READ;
	DWORD shareMode = FILE_SHARE_READ;
	DWORD creationDis = OPEN_EXISTING;
	DWORD flagsAndAttr = FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE;

	handle = CreateFile(filePath, desiredAccess, shareMode, NULL, creationDis, flagsAndAttr, NULL);
	SetFilePointer(handle, 0, 0, FILE_BEGIN);

	fileLength = GetFileSize(handle, &HighPart);
	fileBuffer = (RdByte*)malloc(fileLength);

	res = ReadFile(handle, fileBuffer, fileLength, &bytesRead, NULL);
	CloseHandle(handle);
#else
	int  handle;
	int flags = O_RDONLY;
	handle = open(filePath, flags, S_IRUSR | S_IWUSR | S_IRGRP);
	lseek(handle, 0, SEEK_SET);

	struct stat st;
	fstat(handle, &st);
	fileLength = st.st_size;
	fileBuffer = (RdByte*)malloc(fileLength);
	int bytesRead = read(handle, fileBuffer, fileLength);
	close(handle);

#endif

	if (type == RdFlashImageTypeImageList)
	{
		char FilePathTemp[RD_PATH_MAX_LEN];
		_GetDirectory(FilePathTemp, RD_PATH_MAX_LEN, filePath, RD_PATH_MAX_LEN);
		RdStringCat(FilePathTemp, RD_PATH_MAX_LEN, "/", RD_PATH_MAX_LEN);
		RdStringCat(FilePathTemp, RD_PATH_MAX_LEN, fileName, RD_PATH_MAX_LEN);
		RdStringCopy(fileName, RD_PATH_MAX_LEN, FilePathTemp, RD_PATH_MAX_LEN);
	}
	// send file to device
	ret = RdFlashWriteEnable(rdi, RdFlagTrue);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdFlashImage(rdi, type, fileName, fileLength, &transferId);
	if (ret != 0)
	{
		return ret;
	}
	ret = _WriteData(rdi, transferId, fileBuffer, fileLength);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdFlashData(rdi, transferId, RdFlashDataTypeEnd, NULL, 0);
	if (ret != 0)
	{
		return ret;
	}
	ret = RdFlashWriteEnable(rdi, RdFlagFalse);
	if (ret != 0)
	{
		return ret;
	}

	free(fileBuffer);
	return ret;
}

RdStatus _CheckFreeSpaceOnDevice(RdInterface* rdi, RdUInt32 size)
{
	RdStatus ret = 0;
	char* deviceFreeMemory;
	RdUInt32 deviceFreeMemoryInt;

	float checkSize = (float)size / 1000000;	//convert in MB
	ret = RdSystemInfo(rdi, 5, &deviceFreeMemory);
	if (ret != 0)
	{
		return ret;
	}
	deviceFreeMemoryInt = atoi(deviceFreeMemory);
	if (deviceFreeMemoryInt < checkSize)
	{
		RD_ERR("Insufficient free space available on device. Please reduce total project data size ");
		return RdErrorFileDownloadFailed;
	}
	return ret;

}

RdStatus _CheckForAvailableSizeOnDevice(RdInterface* rdi, const char* downloadFilePath)
{
	RdStatus ret = 0;
	RdUInt32 size = 0;

	char filePath[RD_PATH_MAX_LEN];
	char ext[RD_PATH_MAX_LEN];

#if _RD_OS_WINDOWS
	WIN32_FIND_DATA fd;
	HANDLE hFind;

	if (GetFileAttributes(downloadFilePath) & FILE_ATTRIBUTE_DIRECTORY)
	{
		char dirPath[RD_PATH_MAX_LEN];
		RdFsMakePath(dirPath, RD_PATH_MAX_LEN, downloadFilePath, RD_PATH_MAX_LEN, "*", RD_PATH_MAX_LEN);

		hFind = FindFirstFile(dirPath, &fd);

		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					_GetExtension(ext, RD_PATH_MAX_LEN, fd.cFileName, RD_PATH_MAX_LEN);
					if (!((RdStringCompare(".bmp", RD_PATH_MAX_LEN, ext, RD_PATH_MAX_LEN)) && (RdStringCompare(".dat", RD_PATH_MAX_LEN, ext, RD_PATH_MAX_LEN))))
					{
						RdFsMakePath(filePath, RD_PATH_MAX_LEN, downloadFilePath, RD_PATH_MAX_LEN, fd.cFileName, RD_PATH_MAX_LEN);
						size += fd.nFileSizeLow;
					}
				}
			} while (FindNextFile(hFind, &fd) != 0);
		}
	}
	else
	{
		HANDLE handle = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (handle != INVALID_HANDLE_VALUE)
		{
			DWORD highPart = 0;
			DWORD lowPart = 0;
			lowPart = GetFileSize(handle, &highPart);
			size += lowPart;
			CloseHandle(handle);
		}
	}
#else
	struct dirent *ent;
	struct stat buf;
	DIR *dir;

	if ((dir = opendir(downloadFilePath)) != NULL) //ImageList
	{
		while ((ent = readdir(dir)) != NULL)
		{
			_GetExtension(ext, RD_PATH_MAX_LEN, ent->d_name, RD_PATH_MAX_LEN);
			if (!((RdStringCompare(".bmp", RD_PATH_MAX_LEN, ext, RD_PATH_MAX_LEN)) && (RdStringCompare(".dat", RD_PATH_MAX_LEN, ext, RD_PATH_MAX_LEN))))
			{
				RdFsMakePath(filePath, RD_PATH_MAX_LEN, downloadFilePath, RD_PATH_MAX_LEN, ent->d_name, RD_PATH_MAX_LEN);
				stat(filePath, &buf);
				size += buf.st_size;
			}
		}
	}
	else  											//Image and fonts
	{
		stat(downloadFilePath, &buf);
		size = buf.st_size;
	}
#endif
	return _CheckFreeSpaceOnDevice(rdi, size);
}

RdUInt32 _GetTotalSizeOfDownloadData(RdInterface* rdi, const char* downloadFilePath)
{
	RdStatus ret = 0;
	static RdUInt32 size;
	char ext[RD_PATH_MAX_LEN];
	char dirPath[RD_PATH_MAX_LEN];
	char filePath[RD_PATH_MAX_LEN];

#if _RD_OS_WINDOWS
	WIN32_FIND_DATA fd;
	HANDLE hFind;

	if (GetFileAttributes(downloadFilePath) & FILE_ATTRIBUTE_DIRECTORY)
	{
		RdFsMakePath(dirPath, RD_PATH_MAX_LEN, downloadFilePath, RD_PATH_MAX_LEN, "*", RD_PATH_MAX_LEN);

		hFind = FindFirstFile(dirPath, &fd);

		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (RdStringCompare(fd.cFileName, RD_PATH_MAX_LEN, ".", RD_PATH_MAX_LEN) != 0 && RdStringCompare(fd.cFileName, RD_PATH_MAX_LEN, "..", RD_PATH_MAX_LEN) != 0)
				{
					RdFsMakePath(filePath, RD_PATH_MAX_LEN, downloadFilePath, RD_PATH_MAX_LEN, fd.cFileName, RD_PATH_MAX_LEN);
					_GetTotalSizeOfDownloadData(rdi, filePath);
				}
			} while (FindNextFile(hFind, &fd) != 0);
		}
	}
	else
	{
		_GetExtension(ext, RD_PATH_MAX_LEN, downloadFilePath, RD_PATH_MAX_LEN);
		if (!((RdStringCompare(".bmp", RD_PATH_MAX_LEN, ext, RD_PATH_MAX_LEN)) && (RdStringCompare(".dat", RD_PATH_MAX_LEN, ext, RD_PATH_MAX_LEN))))
		{
			HANDLE handle = CreateFile(downloadFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (handle != INVALID_HANDLE_VALUE)
			{
				DWORD highPart = 0;
				DWORD lowPart = 0;
				lowPart = GetFileSize(handle, &highPart);
				size += lowPart;
				CloseHandle(handle);
			}
		}
		return 0;
	}
#else
	struct dirent *ent;
	struct stat buf;
	DIR *dir;

	if ((dir = opendir(downloadFilePath)) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			if (ent->d_name[0] != '.')
			{
				RdFsMakePath(filePath, RD_PATH_MAX_LEN, downloadFilePath, RD_PATH_MAX_LEN, ent->d_name, RD_PATH_MAX_LEN);
				_GetTotalSizeOfDownloadData(rdi, filePath);
			}
		}
		closedir(dir);
	}
	else
	{
		_GetExtension(ext, RD_PATH_MAX_LEN, downloadFilePath, RD_PATH_MAX_LEN);
		if (!((RdStringCompare(".bmp", RD_PATH_MAX_LEN, ext, RD_PATH_MAX_LEN)) && (RdStringCompare(".dat", RD_PATH_MAX_LEN, ext, RD_PATH_MAX_LEN))))
		{
			stat(downloadFilePath, &buf);
			size += buf.st_size;
		}
		return size;
	}
#endif
	return size;
}


RdStatus RdDownloadUnavailableResources(RdInterface *rdi,RdResourceLoadInfo downloadResources[],const char* projectPath,RdId failedCount)
{
   RdUInt32 totalResourceSize = 0;
   RdId ret,i;
   char directoryPath[RD_PATH_MAX_LEN];

   for(i = 0; i < failedCount; i++) // calculate size of failed resource
    {
		 if(downloadResources[i].resLoadType == RdResourceTypeImage)
	     {
			 RdFsMakePath(directoryPath,RD_PATH_MAX_LEN,projectPath,RD_PATH_MAX_LEN,"images/",RD_PATH_MAX_LEN);
			 RdStringCat(directoryPath,RD_PATH_MAX_LEN,downloadResources[i].resLabel,RD_PATH_MAX_LEN);
			 RdStringCat(directoryPath,RD_PATH_MAX_LEN,".bmp",RD_PATH_MAX_LEN);
	         totalResourceSize = _GetTotalSizeOfDownloadData(rdi,directoryPath);
	     }

		 if(downloadResources[i].resLoadType == RdResourceTypeFont)
	     {
			 RdFsMakePath(directoryPath,RD_PATH_MAX_LEN,projectPath,RD_PATH_MAX_LEN,"fonts/",RD_PATH_MAX_LEN);
			 RdStringCat(directoryPath,RD_PATH_MAX_LEN,downloadResources[i].resLabel,RD_PATH_MAX_LEN);
			 RdStringCat(directoryPath,RD_PATH_MAX_LEN,".bmp",RD_PATH_MAX_LEN);
	         totalResourceSize = _GetTotalSizeOfDownloadData(rdi,directoryPath);


			 RdFsMakePath(directoryPath,RD_PATH_MAX_LEN,projectPath,RD_PATH_MAX_LEN,"fonts/",RD_PATH_MAX_LEN);
			 RdStringCat(directoryPath,RD_PATH_MAX_LEN,downloadResources[i].resLabel,RD_PATH_MAX_LEN);
			 RdStringCat(directoryPath,RD_PATH_MAX_LEN,"-info.dat",RD_PATH_MAX_LEN);
	         totalResourceSize = _GetTotalSizeOfDownloadData(rdi,directoryPath);
	     }

		 if(downloadResources[i].resLoadType == RdResourceTypeImageList)
	     {
			 RdFsMakePath(directoryPath,RD_PATH_MAX_LEN,projectPath,RD_PATH_MAX_LEN,"imagelists/",RD_PATH_MAX_LEN);
			 RdStringCat(directoryPath,RD_PATH_MAX_LEN,downloadResources[i].resLabel,RD_PATH_MAX_LEN);
	         totalResourceSize = _GetTotalSizeOfDownloadData(rdi,directoryPath);
	     }

    }

   ret = _CheckFreeSpaceOnDevice(rdi,totalResourceSize);

   if(ret != 0) // if device has not sufficient space then it will return
   {
	   return ret;
   }
   else       // if device has sufficient space then it will Download images on Device 
   {
	   for(i = 0; i < failedCount; i++)
	   {
		   ret = RdDeviceDataDownloadFile(rdi, projectPath, downloadResources[i].resLabel);
		   if(ret != 0)
		   {
				return ret;
		   }
	   }
   }
   return ret;
}

RdStatus _ProcessFolder(RdInterface* rdi, RdId type, const char* directoryPath)
{
	RdStatus ret = 0;
	char ext[RD_PATH_MAX_LEN];
	char fileName[RD_PATH_MAX_LEN];
	char filePath[RD_PATH_MAX_LEN];
#if _RD_OS_WINDOWS
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	if (GetFileAttributes(directoryPath) & FILE_ATTRIBUTE_DIRECTORY)
	{
		char dirPath[RD_PATH_MAX_LEN];
		RdFsMakePath(dirPath, RD_PATH_MAX_LEN, directoryPath, RD_PATH_MAX_LEN, "*", RD_PATH_MAX_LEN);
		hFind = FindFirstFile(dirPath, &fd);

		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				_GetExtension(ext, RD_PATH_MAX_LEN, fd.cFileName, RD_PATH_MAX_LEN);
				if(!(((type != RdFlashImageTypeFontInfo)?(RdStringCompare(".bmp",RD_PATH_MAX_LEN, ext, RD_PATH_MAX_LEN)):1) && ((type == RdFlashImageTypeFontInfo)?(RdStringCompare(".dat",RD_PATH_MAX_LEN, ext, RD_PATH_MAX_LEN)):1)))
				{
					RdFsMakePath(filePath, RD_PATH_MAX_LEN, directoryPath, RD_PATH_MAX_LEN, fd.cFileName, RD_PATH_MAX_LEN);
					_GetTitle(fileName, RD_PATH_MAX_LEN, fd.cFileName, RD_PATH_MAX_LEN);
					ret = _RdDeviceDataSendFileToDevice(rdi, type, filePath, fileName);
					if (ret != 0)
					{
						return ret;
					}
				}
				else
				{
					continue;					//NOT proper file
				}
			} while (FindNextFile(hFind, &fd) != 0);

		}
	}
	else
	{
		RD_ERR("could not open directory %s : ErrorInvalidDirectoryPath ", directoryPath);
		return RdErrorInvalidDirectoryPath;
	}
#else
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(directoryPath)) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)			//all the files and directories within directory
		{
			_GetExtension(ext, RD_PATH_MAX_LEN, ent->d_name, RD_PATH_MAX_LEN);
			if(!(((type != RdFlashImageTypeFontInfo)?(RdStringCompare(".bmp",RD_PATH_MAX_LEN, ext, RD_PATH_MAX_LEN)):1) && ((type == RdFlashImageTypeFontInfo)?(RdStringCompare(".dat",RD_PATH_MAX_LEN, ext, RD_PATH_MAX_LEN)):1)))
			{
				RdFsMakePath(filePath, RD_PATH_MAX_LEN, directoryPath, RD_PATH_MAX_LEN, ent->d_name, RD_PATH_MAX_LEN);
				_GetTitle(fileName, RD_PATH_MAX_LEN, ent->d_name, RD_PATH_MAX_LEN);
				ret = _RdDeviceDataSendFileToDevice(rdi, type, filePath, fileName);
				if (ret != 0)
				{
					return ret;
				}
			}
			else
			{
				continue;					//NOT proper file
			}
		}
		closedir(dir);
	}
	else
	{
		RD_ERR("could not open directory %s : ErrorInvalidDirectoryPath ", directoryPath);
		return RdErrorInvalidDirectoryPath;
	}
#endif
	return ret;
}

RdStatus _ProcessImageLists(RdInterface* rdi, const char*  downloadFilePath)
{
	RdStatus ret = 0;
	char directoryPath[RD_PATH_MAX_LEN];
#if _RD_OS_WINDOWS
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	char dirPath[RD_PATH_MAX_LEN];
	if (GetFileAttributes(downloadFilePath) & FILE_ATTRIBUTE_DIRECTORY)
	{
		RdFsMakePath(dirPath, RD_PATH_MAX_LEN, downloadFilePath, RD_PATH_MAX_LEN, "*", RD_PATH_MAX_LEN);

		hFind = FindFirstFile(dirPath, &fd);

		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (RdStringCompare(fd.cFileName, RD_PATH_MAX_LEN, ".", RD_PATH_MAX_LEN) != 0 && RdStringCompare(fd.cFileName, RD_PATH_MAX_LEN, "..", RD_PATH_MAX_LEN) != 0)
				{
					RdFsMakePath(directoryPath, RD_PATH_MAX_LEN, downloadFilePath, RD_PATH_MAX_LEN, fd.cFileName, RD_PATH_MAX_LEN);
					ret = _ProcessFolder(rdi, RdFlashImageTypeImageList, directoryPath);
					if (ret != 0)
					{
						return ret;
					}
				}
			} while (FindNextFile(hFind, &fd) != 0);

		}
	}
	else
	{
		RD_ERR("could not open directory %s : ErrorInvalidDirectoryPath ", directoryPath);
		return RdErrorInvalidDirectoryPath;
	}
#else
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir (downloadFilePath)) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			if (ent->d_name[0] != '.')
			{
				RdFsMakePath(directoryPath, RD_PATH_MAX_LEN, downloadFilePath, RD_PATH_MAX_LEN, ent->d_name, RD_PATH_MAX_LEN);
				ret = _ProcessFolder(rdi, RdFlashImageTypeImageList, directoryPath);
				if (ret != 0)
				{
					return ret;
				}
			}
		}
		closedir(dir);
	}
	else
	{
		RD_ERR("could not open directory %s : ErrorInvalidDirectoryPath ", downloadFilePath);
		return RdErrorInvalidDirectoryPath;
	}
#endif
	return ret;
}

RdStatus _CheckFileAndDownload(RdInterface* rdi, RdId type, const char* downloadFilePath, const char* selectedFileName)
{
	RdStatus ret = 0;
	char ext[RD_PATH_MAX_LEN];
	char fileName[RD_PATH_MAX_LEN];
	char dirPath[RD_PATH_MAX_LEN];
	char filePath[RD_PATH_MAX_LEN];
#if _RD_OS_WINDOWS
	WIN32_FIND_DATA fd;
	HANDLE hFind;

	if (GetFileAttributes(downloadFilePath) & FILE_ATTRIBUTE_DIRECTORY)
	{
		RdFsMakePath(dirPath, RD_PATH_MAX_LEN, downloadFilePath, RD_PATH_MAX_LEN, "*", RD_PATH_MAX_LEN);

		hFind = FindFirstFile(dirPath, &fd);

		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				_GetExtension(ext, RD_PATH_MAX_LEN, fd.cFileName, RD_PATH_MAX_LEN);
				RdFsMakePath(filePath, RD_PATH_MAX_LEN, downloadFilePath, RD_PATH_MAX_LEN, fd.cFileName, RD_PATH_MAX_LEN);
				if (type == RdFlashImageTypeImageList)				        //IMAGELIST FOLDER
				{
					if (RdStringCompare(fd.cFileName, RD_PATH_MAX_LEN, selectedFileName, RD_PATH_MAX_LEN) == 0)
					{
						ret = _CheckForAvailableSizeOnDevice(rdi, filePath);
						if (ret != 0)
						{
							return ret;
						}
						ret = _ProcessFolder(rdi, RdFlashImageTypeImageList, filePath);
						if (ret != 0)
						{
							return ret;
						}
 					    else
					    {
						    return  RdFlagTrue;
					    }


					}
				}
				else if (!((RdStringCompare(".bmp", RD_PATH_MAX_LEN, ext, RD_PATH_MAX_LEN)) && (RdStringCompare(".dat", RD_PATH_MAX_LEN, ext, RD_PATH_MAX_LEN))))
				{
					_GetTitle(fileName, RD_PATH_MAX_LEN, fd.cFileName, RD_PATH_MAX_LEN);
					if (RdStringCompare(fileName, RD_PATH_MAX_LEN, selectedFileName, RD_PATH_MAX_LEN) == 0)
					{
						ret = _CheckForAvailableSizeOnDevice(rdi, filePath);
						if (ret != 0)
						{
							return ret;
						}
						ret = _RdDeviceDataSendFileToDevice(rdi, type, filePath, fileName);
						if (ret != 0)
						{
							return ret;
						}

 					    else
					    {
						    return  RdFlagTrue;
					    }

					}
				}
				else
				{
					continue;	//NOT proper file
				}
			} while (FindNextFile(hFind, &fd) != 0);
		}
	}
	else
	{
		RD_ERR("could not open directory %s : ErrorInvalidDirectoryPath ", downloadFilePath);
		return RdErrorInvalidDirectoryPath;
	}
#else
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(downloadFilePath)) != NULL)   //OPEN FILE
	{
		while ((ent = readdir(dir)) != NULL) //CHECK FILES
		{
			_GetExtension(ext, RD_PATH_MAX_LEN, ent->d_name, RD_PATH_MAX_LEN);
			RdFsMakePath(filePath, RD_PATH_MAX_LEN, downloadFilePath, RD_PATH_MAX_LEN, ent->d_name, RD_PATH_MAX_LEN);
			if (type == RdFlashImageTypeImageList)				        //IMAGELIST FOLDER
			{
				if (RdStringCompare(ent->d_name, RD_PATH_MAX_LEN, selectedFileName, RD_PATH_MAX_LEN) == 0)
				{
					ret = _CheckForAvailableSizeOnDevice(rdi, filePath);
					if (ret != 0)
					{
						return ret;
					}
					ret = _ProcessFolder(rdi, RdFlashImageTypeImageList, filePath);
					if (ret != 0)
					{
						return ret;
					}
					else
					{
						return  RdFlagTrue;
					}
				}
			}
			else if (!((RdStringCompare(".bmp", RD_PATH_MAX_LEN, ext, RD_PATH_MAX_LEN)) && (RdStringCompare(".dat", RD_PATH_MAX_LEN, ext, RD_PATH_MAX_LEN))))
			{
				_GetTitle(fileName, RD_PATH_MAX_LEN, ent->d_name, RD_PATH_MAX_LEN);
				if (RdStringCompare(fileName, RD_PATH_MAX_LEN, selectedFileName, RD_PATH_MAX_LEN) == 0)
				{
					ret = _CheckForAvailableSizeOnDevice(rdi, filePath);
					if (ret != 0)
					{
						return ret;
					}
					ret = _RdDeviceDataSendFileToDevice(rdi, type, filePath, fileName);
					if (ret != 0)
					{
						return ret;
					}
					else
					{
						return  RdFlagTrue;
					}
				}
			}
			else
			{
				continue;	//NOT proper file
			}
		}
		closedir(dir);
	}
	else
	{
		RD_ERR("could not open directory %s : ErrorInvalidDirectoryPath ", downloadFilePath);
		return RdErrorInvalidDirectoryPath;
	}
#endif
	return ret;
}

RdStatus RdDeviceDataDownloadFile(RdInterface* rdi, const char* dirPath, const char* fileName)
{
	RdStatus ret = 0;
	char directoryPath[RD_PATH_MAX_LEN];
	char fileNameFontInfo[RD_PATH_MAX_LEN];
	//IMAGES
	RdFsMakePath(directoryPath, RD_PATH_MAX_LEN, dirPath, RD_PATH_MAX_LEN, "images", RD_PATH_MAX_LEN);
	if (ret != 0)
	{
		return ret;
	}
	ret = _CheckFileAndDownload(rdi, RdFlashImageTypeImage, directoryPath, fileName);
	if(ret == RdFlagTrue)
	{
		return 0;
	}
	else if(ret != 0)
	{
		return ret;
	}

	//FONTS
	RdFsMakePath(directoryPath, RD_PATH_MAX_LEN, dirPath, RD_PATH_MAX_LEN, "fonts", RD_PATH_MAX_LEN);

	ret = _CheckFileAndDownload(rdi, RdFlashImageTypeFontBitmap, directoryPath, fileName);
	if(ret == RdFlagTrue)
	{
		//Font bmp file downloaded so now download fontinfo
		//FONTS INFO
		RdStringCopy(fileNameFontInfo, RD_PATH_MAX_LEN, fileName, RD_PATH_MAX_LEN);
		RdStringCat(fileNameFontInfo, RD_PATH_MAX_LEN, "-info", RD_PATH_MAX_LEN);
		ret = _CheckFileAndDownload(rdi, RdFlashImageTypeFontInfo, directoryPath, fileNameFontInfo);
		if(ret == RdFlagTrue)
		{
			return 0;
		}
		else if(ret != 0)
		{
			return ret;
		}
	}
	else if(ret != 0)
	{
		return ret;
	}

	RdFsMakePath(directoryPath, RD_PATH_MAX_LEN, dirPath, RD_PATH_MAX_LEN, "imagelists", RD_PATH_MAX_LEN);		//IMAGELIST

	ret = _CheckFileAndDownload(rdi, RdFlashImageTypeImageList, directoryPath, fileName);
	if(ret == RdFlagTrue)
	{
		return 0;
	}
	else if(ret != 0)
	{
		return ret;
	}
	RD_ERR("File %s not exist: RdErrorFileNotExist ", fileName);
	return RdErrorFileNotExist;
}

RdStatus RdDeviceDataDownloadAll(RdInterface* rdi, const char* dirPath)
{
	RdStatus ret = 0;
	char directoryPath[RD_PATH_MAX_LEN];
	RdUInt32 size;
	RD_DBG("IN ");

	size = _GetTotalSizeOfDownloadData(rdi, dirPath);
	ret = _CheckFreeSpaceOnDevice(rdi, size);
	if (ret != 0)
	{
		return ret;
	}
	RdFsMakePath(directoryPath, RD_PATH_MAX_LEN, dirPath, RD_PATH_MAX_LEN, "images", RD_PATH_MAX_LEN);	 //IMAGES

	ret = _ProcessFolder(rdi, RdFlashImageTypeImage, directoryPath);
	if (ret != 0)
	{
		return ret;
	}

	RdFsMakePath(directoryPath, RD_PATH_MAX_LEN, dirPath, RD_PATH_MAX_LEN, "fonts", RD_PATH_MAX_LEN);	 //FONTS

	ret = _ProcessFolder(rdi, RdFlashImageTypeFontBitmap, directoryPath);
	if (ret != 0)
	{
		return ret;
	}
	ret = _ProcessFolder(rdi, RdFlashImageTypeFontInfo, directoryPath);
	if (ret != 0)
	{
		return ret;
	}

	RdFsMakePath(directoryPath, RD_PATH_MAX_LEN, dirPath, RD_PATH_MAX_LEN, "imagelists", RD_PATH_MAX_LEN); 	 //IMAGELIST

	ret = _ProcessImageLists(rdi, directoryPath);
	if (ret != 0)
	{
		return ret;
	}

	RD_DBG("OUT ");
	return ret;
}

RdStatus RdDeviceDeleteAll(RdInterface* rdi)
{
	RdStatus ret = 0;
	RD_DBG("IN ");

	ret = RdFlashWriteEnable(rdi, RdFlagTrue);
	if (ret == 0)
	{
		ret = RdFlashDeleteAll(rdi);
		if (ret != 0)
		{
			return ret;
		}
	}
	ret = RdFlashWriteEnable(rdi, RdFlagFalse);
	RD_DBG("OUT ");
	return ret;
}
#endif

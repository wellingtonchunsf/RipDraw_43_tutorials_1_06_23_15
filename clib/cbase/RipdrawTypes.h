//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux
//

#ifndef __RIPDRAW_TYPES_H__
#define __RIPDRAW_TYPES_H__

#include "_cbase.h"

typedef unsigned short RdId;
typedef unsigned short RdStatus;
typedef unsigned char RdByte;
typedef unsigned short RdUWord;
typedef unsigned int RdUInt32;

#define RD_INTERFACE_TYPE_UART 0x01
#define RD_INTERFACE_TYPE_SPI 0x02

typedef enum _RdCommand
{
	RdCommandIdUnknwn = 0,

	// Layer 
	RdCommandSetLayerEnable = 0x3131,
	RdCommandSetLayerOriginAndSize = 0x3132,
	RdCommandSetLayerBackColor = 0x3133,
	RdCommandSetLayerTransparency = 0x3134,
	RdCommandLayerClear = 0x3135,
	RdCommandLayerMove = 0x3136,
	RdCommandLayerWriteRawPixels = 0x3137,
	RdCommandComposeLayersToPage = 0x3138,
	RdCommandPageToScreen = 0x3139,
	RdCommandPartialComposeLayersToScreen = 0x03141,

	// Image 
	RdCommandImageLoad = 0x3231,
	RdCommandImageRelease = 0x3232,
	RdCommandImageWrite = 0x3233,
	RdCommandImageDelete = 0x3234,
	RdCommandImageMove = 0x3235,
	RdCommandImageListLoad = 0x3236,
	RdCommandImageListRelease = 0x3237,
	RdCommandImageListWrite = 0x3238,
	RdCommandImageListReplace = 0x3239,
	RdCommandImageListDelete = 0x3240,
	RdCommandAnimationPlay = 0x3241,
	RdCommandAnimationStop = 0x3242,
	RdCommandAnimationContinue = 0x3243,
	RdCommandAnimationDelete = 0x3244,
	RdCommandImageReplace = 0x3245,

	// Text 
	RdCommandFontLoad = 0x3331,
	RdCommandFontRelease = 0x3332,
	RdCommandSetFontPadding = 0x3333,
	RdCommandStringWrite = 0x3334,
	RdCommandStringReplace = 0x3335,
	RdCommandStringDelete = 0x3336,
	RdCommandCharacterWrite = 0x3337,
	RdCommandCharacterReplace = 0x3338,
	RdCommandCharacterDelete = 0x3339,
	RdCommandTextWindowCreate = 0x3341,
	RdCommandTextWindowSetInsertionPoint = 0x3342,
	RdCommandTextWindowInsertText = 0x03343,
	RdCommandTextWindowDelete = 0x3344,

	// Graph 
	RdCommandLineGraphCreateWindow = 0x3431,
	RdCommandLineGraphInsertPoints = 0x3432,
	RdCommandLineGraphMove = 0x3433,
	RdCommandLineGraphDeleteWindow = 0x3434,
	RdCommandBarGraphCreateWindow = 0x3435,
	RdCommandBarGraphInsertStacks = 0x3436,
	RdCommandBarGraphRemoveStacks = 0x3437,
	RdCommandBarGraphDeleteWindow = 0x3438,

	// Touch 
	RdCommandTouchMapRectangle = 0x3531,
	RdCommandTouchMapCircle = 0x3532,
	RdCommandTouchMapDelete = 0x3533,
	RdCommandTouchMapClear = 0x3534,

	// Information 
	RdCommandSystemInfo = 0x3631,

	// Configuration 
	RdCommandGetMaxBackLightBrightness = 0x3731,
	RdCommandGetBackLightBrightness = 0x3732,
	RdCommandSetBackLightBrightness = 0x3733,

	// Event 
	RdCommandReset = 0x3831,
	RdCommandEventMessage = 0x3832,
	RdCommandSetEventType = 0x3833,
	// Other 
	RdCommandTestEcho = 0x3931,

	// Flash 
	RdCommandFlashWriteEnable = 0x4031,
	RdCommandFlashImage = 0x4032,
	RdCommandFlashData = 0x4033,
	RdCommandFlashDelete = 0x4034,
	RdCommandFlashDeleteAll = 0x4035
} RdCommand;

typedef enum _RdError
{
	//Success
	RdErrorSuccess = 0x0000,

	// basic
	RdErrorUnknown = 0x8000,
	RdErrorInsufficientResource = 0x8001,
	RdErrorNotSupported = 0x8002,
	RdErrorInvalidState = 0x8003,

	// command protocol
	RdErrorCommandInvalidLength = 0x8004,
	RdErrorInvalidSeqNo = 0x8005,
	RdErrorCommandInvalidChecksum = 0x8006,
	RdErrorCommandNotFound = 0x8007,
	RdErrorReadCheckFailed = 0x8008,

	// size and position
	RdErrorInvalidPosition = 0x8009,
	RdErrorInvalidSize = 0x8010,
	RdErrorInvalidDataLength = 0x8601,

	// Layer
	RdErrorLayerInvalidId = 0x8011,
	RdErrorLayerComposeFailed = 0x8012,

	// Page
	RdErrorPageInvalidId = 0x8021,
	RdErrorImageInvalidLabel = 0x8022,
	RdErrorImageInvalidId = 0x8023,
	RdErrorFontInvalidId = 0x8024,
	RdErrorTextWindowInvalidId = 0x8025,
	RdErrorGraphWindowInvalidId = 0x8026,
	RdErrorImageWriteInvalidId = 0x8027,
	RdErrorStringWriteInvalidId = 0x8028,
	RdErrorCharWriteInvalidId = 0x8029,
	RdErrorAnimationInvalidId = 0x8030,
	RdErrorTouchMapInvalidId = 0x8031,
	RdErrorImageListInvalidId = 0x8032,
	RdErrorImageListWriteInvalidId = 0x8033,

	// protocol interface
	RdErrorScriptFileEndOfFile = 0x8101,
	RdErrorScriptFileInvalidCommand = 0x8102,
	RdErrorScriptFileInvalidParam = 0x8103,
	RdErrorScriptFileProcessFailed = 0x8104,
	RdErrorScriptFileVarMapAlreadyExist = 0x810A,
	RdErrorScriptFileInvalidSeqNo = 0x810B,

	// web interface and other
	RdErrorImageReadFailed = 0x8201,
	RdErrorImageWriteFailed = 0x8202,
	RdErrorImageSettingsFailed = 0x8203,
	RdErrorImageResizeFailed = 0x8204,
	RdErrorZipCreateFileFailed = 0x8205,
	RdErrorFreeTypeSettingsFailed = 0x8206,
	RdErrorDpiOrDestFileSettingsFailed = 0x8207,

	// flash
	RdErrorInFlashFileReadFailed = 0x8301,
	RdErrorInFlashDataTransfer = 0x8302,
	RdErrorInFlashMountFailed = 0x8303,
	RdErrorInsufficientFlashMemory = 0x8304,
	// other
	RdErrorPageInitFailed = 0x8401,
	RdErrorIdOverFlow = 0x8402,
	RdErrorConfigLoadFailed = 0x8403,
	RdErrorConfigSaveFailed = 0x8404,

	//i2c
	RdErrorI2cInitFailed = 0x8501,
	RdErrorI2cReadFailed = 0x8502,

	// gui
	RdErrorUiRegisterClassFailed = 0x8801,
	RdErrorUiCreateWindowFailed = 0x8802,
	RdErrorUiSetWindowLongPtrFailed = 0x8803,
	RdErrorlineglowByteFailed = 0x8804,

	//buffer
	RdErrorBufferNull = 0x8901,
	RdErrorInsufficientMemory = 0x8902,
	RdErrorInvalidResponse = 0x8903,
	RdErrorMemoryAllocationFailed = 0x8904,
	RdErrorResponseTimeOut = 0x8905,

	//interface
	RdErrorInterfaceNull = 0x8906,
	RdErrorPortOpenFailed = 0x8907,
	RdErrorPortInitFailed = 0x8908,
	RdErrorExtintAllocFailed = 0x8909,
	RdErrorIntWriteFailed = 0x8910,
	RdErrorIntReadFailed = 0x8911,
	RdErrorFtWriteFailed = 0x8912,
	RdErrorFtReadFailed = 0x8913,
	RdErrorFtOpenFailed = 0x8914,
	RdErrorFtInitFailed = 0x8915,
	RdErrorSeqNoMissmatch = 0x8916,
	RdErrorChecksumMissmatch = 0x8917,
	RdErrorDeviceCommandFailed = 0x8918,

	//Extlib
	RdErrorInvalidDirectoryPath = 0x9001,
	RdErrorFileOpenFailed = 0x9002,
	RdErrorFileNotExist = 0x9003,
	RdErrorFileDownloadFailed = 0x9004,

	//Log
	RdErrorLogFileOpenFailed = 0x9100,
	RdErrorInvalidDebugLevel = 0x9101,
	RdErrorInvalidLogType = 0x9102,
	RdErrorInvalidInterfaceSpeed = 0x9103,
	RdErrorInvalidInterfaceType = 0x9104,
    RdPortNotProvided = 0x9105,
	RdLogFileNotProvided = 0x9106,
	RdDebugLevelNotProvided = 0x9107,
	RdLogTypeNotProvided = 0x9108,
	RdInterfaceSpeedNotProvided = 0x9109,
	RdInterfaceTypeNotProvided = 0x9110,
	RdInterfaceDetailsNotProvided = 0x9111,

	//Version
	RdErrorclibVersionNotCompatible = 0x9200,

	//Controls
	RdErrorInvalidButtonImageLabel = 0x9201,
} RdError;

typedef enum _RdEventType
{
	RdEventTypeAnimation = 0,
	RdEventTypeTouch = 1
} RdEventType;

typedef enum _RdFlashImageType
{
	RdFlashImageTypeImage = 0,
	RdFlashImageTypeImageList = 1,
	RdFlashImageTypeFontBitmap = 2,
	RdFlashImageTypeFontInfo = 3,
} RdFlashImageType;

typedef enum _RdFlashDataType
{
	RdFlashDataTypeChunk = 0,
	RdFlashDataTypeEnd = 1,
} RdFlashDataType;

typedef enum _RdSystemInfoType
{
	RdSystemInfoTypeVersionDevApp = 0,
	RdSystemInfoTypeVersionReserve1 = 1,
	RdSystemInfoTypeVersionTypeOS = 2,
	RdSystemInfoTypeTotalRam = 3,
	RdSystemInfoTypeTotalRom = 4,
	RdSystemInfoTypeFreeRom = 5,
	RdSystemInfoTypeDisplayMatrix = 6,
	RdSystemInfoDevdataChecksum = 7,
	RdSystemInfoRootfsChecksum = 8,
	RdSystemInfoMinClibVersion = 9,
	RdSystemInfoMaxLayersAllowed = 10,
} RdSystemInfoType;

typedef enum _RdHDirection
{
	RdHDirectionLeft = 0,
	RdHDirectionRight = 1,
} RdHDirection;

typedef enum _RdDirection
{
	RdDirectionHorizontal = 0,
	RdDirectionVertical = 1,
} RdDirection;

typedef enum _RdBufferDirection
{
	RdBufferDirectionHost,
	RdBufferDirectionDevice,
} RdBufferDirection;

typedef enum _RdFlag
{
	RdFlagFalse = 0,
	RdFlagTrue = 1,
} RdFlag;

typedef enum _RdTouchMapType
{
	RdTouchMapTypeCircle = 1,
	RdTouchMapTypeRectangle = 2,
} RdTouchMapType;

typedef enum _RdResourceType
{
	RdResourceTypeImage = 1,
	RdResourceTypeFont = 2,
	RdResourceTypeImageList = 3,
} RdResourceType;

typedef struct _RdInterface RdInterface;
typedef int(*_INT_WRITE)(RdInterface*, RdByte*, int);
typedef int(*_INT_READ)(RdInterface*, RdByte*, int);
typedef int(*_INT_CLOSE)(RdInterface*);

#pragma pack(push, 1)
typedef struct _RdPosition
{
	RdUWord x;
	RdUWord y;
} RdPosition;

typedef struct _RdSize
{
	RdUWord width;
	RdUWord height;
} RdSize;

#ifdef _RA_COLOR_BIG_ENDIAN		// ARGB
typedef union _RdColor
{
	struct _rgba
	{
		RdByte blue;
		RdByte green;
		RdByte red;
		RdByte alpha;
	} rgba;
	unsigned int color;
} RdColor;

typedef struct _RdColorRgb
{
	RdByte blue;
	RdByte green;
	RdByte red;
} RdColorRgb;
#else						// RGBA
typedef union _RdColor
{
	struct _rgba
	{
		RdByte red;
		RdByte green;
		RdByte blue;
		RdByte alpha;
	}rgba;
	unsigned int color;
} RdColor;

typedef struct _RdColorRgb
{
	RdByte red;
	RdByte green;
	RdByte blue;
} RdColorRgb;
#endif

typedef struct _RdResourceLoadInfo
{
	char * resLabel;
	RdId resId;
	RdResourceType resLoadType;
	RdUWord imageListIndexStart;
	RdUWord imageListIndexStep;
	RdUWord imageListIndexCount;
} RdResourceLoadInfo;

typedef struct _RdEvent
{
	RdByte eventType;
	RdByte* data;
	RdUWord hasMoreData;
} RdEvent;

typedef struct _RdInterfaceBuffer
{
	int capacity;
	int size;
	int position;
	RdByte* ptr;
} RdInterfaceBuffer;

struct _RdInterface
{
	void* extint;

	int isOpen;
	int seqNo;
	int lastCmdId;
	RdUWord lastResponseStatus;

	int extIntType;
	int extIntSpeed;
	char extIntName[255];

	RdInterfaceBuffer writebuffer;
	RdInterfaceBuffer readbuffer;

	_INT_WRITE write;
	_INT_READ read;
	_INT_CLOSE close;

	int ResponseRetryCount;
	int ResponseRetryDelay;
	int writeDelay;
	int cmdDelay;
};

#pragma pack(pop)

#endif

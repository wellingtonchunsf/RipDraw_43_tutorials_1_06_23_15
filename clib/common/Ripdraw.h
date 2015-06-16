//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux
//

#ifndef _RIPDRAW_H_
#define _RIPDRAW_H_

#include "RipdrawCommon.h"

// Layer commands 


#ifdef  __cplusplus
extern "C" {
#endif


RdStatus RdInitRequest(RdInterface* rdi, RdCommand cmdId);
RdStatus RdProcessRequest(RdInterface* rdi);
RdStatus RdProcessResponse(RdInterface* rdi);


// Set Layer Enable 
RdStatus RdSetLayerEnable(RdInterface* rdi, RdId layerId, RdFlag enable);
// Set Layer Origin And Size
// It creates new area and clear layer by filling background color 
RdStatus RdSetLayerOriginAndSize(RdInterface* rdi, RdId layerId, RdPosition position, RdSize size);
// Set Layer Back Color 
RdStatus RdSetLayerBackColor(RdInterface* rdi, RdId layerId, RdColor backColor);
// Set Layer Transparency 
RdStatus RdSetLayerTransparency(RdInterface* rdi, RdId layerId, RdByte transparencyPercentage);
// Layer Clear
// It fills the background color 
RdStatus RdLayerClear(RdInterface* rdi, RdId layerId);
// Move Layer 
RdStatus RdLayerMove(RdInterface* rdi, RdId layerId, RdUWord moveLeft, RdUWord moveTop, RdUWord moveRight, RdUWord moveBottom);
// Write Raw Pixels 
RdStatus RdLayerWriteRawPixels(RdInterface* rdi, RdId layerId, RdPosition position, RdSize pixelSize, const RdColor* pixels);
// Compose all Layers 
RdStatus RdComposeLayersToPage(RdInterface* rdi, RdId pageId);
// shows page to screen 
RdStatus RdPageToScreen(RdInterface* rdi, RdId pageId);
// partial compose 
RdStatus RdPartialComposeLayersToScreen(RdInterface* rdi, RdId pageId);

// Image commands 

// Load image of given name
// Returns image id of newly loaded image 
RdStatus RdImageLoad(RdInterface* rdi, const char* imageLabel, RdId* imageId);
// Image Release 
RdStatus RdImageRelease(RdInterface* rdi, RdId imageId);
// Image write 
RdStatus RdImageWrite(RdInterface* rdi, RdId layerId, RdId imageId, RdPosition position, RdId* imageWriteId);
// Image Delete 
RdStatus RdImageDelete(RdInterface* rdi, RdId imageWriteId);
// Image Move 
RdStatus RdImageMove(RdInterface* rdi, RdId imageWriteId, RdPosition position);
// Image Replace
RdStatus RdImageReplace(RdInterface* rdi, RdId imageWriteId, RdId imageReplaceId);

// Returns image id of newly loaded image list 
RdStatus RdImageListLoad(RdInterface* rdi, const char* prefix, RdUWord indexStart, RdUWord indexStep, RdUWord indexCount, RdId* imageListId);
// Image list Release 
RdStatus RdImageListRelease(RdInterface* rdi, RdId imageListId);
// Image write from list 
RdStatus RdImageListWrite(RdInterface* rdi, RdId layerId, RdPosition position,
	RdId imageListId, RdUWord imageIndex, RdId* imageListWriteId);
// Image Replace 
RdStatus RdImageListReplace(RdInterface* rdi, RdId imageListWriteId, RdUWord imageIndex);
// Image List Delete 
RdStatus RdImageListDelete(RdInterface* rdi, RdId imageListWriteId);
// Animation play 
RdStatus RdAnimationPlay(RdInterface* rdi, RdId layerId, RdPosition position, RdId imageListId, RdUWord frequency, RdId* animationPlayId);
// Animation stop 
RdStatus RdAnimationStop(RdInterface* rdi, RdId animationPlayId, RdUWord stopIndex);
// Animation continue 
RdStatus RdAnimationContinue(RdInterface* rdi, RdId animationPlayId);
// Animation Delete 
RdStatus RdAnimationDelete(RdInterface* rdi, RdId animationPlayId);

// Text commands 

// Load image of given name 
// Returns image id of newly loaded image 
RdStatus RdFontLoad(RdInterface* rdi, const char* fontLabel, RdUWord* fontId);
// Font Release 
RdStatus RdFontRelease(RdInterface* rdi, RdId fontId);
// Font padding
RdStatus RdSetFontPadding(RdInterface* rdi, RdId fontId, const RdByte padding);
// String write 
RdStatus RdStringWrite(RdInterface* rdi, RdId layerId, RdPosition position, RdId fontId, RdColor color, RdHDirection hDirection, const char* data, RdId* stringWriteId);
// String Replace 
RdStatus RdStringReplace(RdInterface* rdi, RdId stringWriteId, const char* data);
// String delete 
RdStatus RdStringDelete(RdInterface* rdi, RdId stringWriteId);
// Character write 
RdStatus RdCharacterWrite(RdInterface* rdi, RdId layerId, RdPosition position, RdId contId, RdColor color, RdByte c, RdId* characterWriteId);
// Character replace 
RdStatus RdCharacterReplace(RdInterface* rdi, RdId characterWriteId, RdByte c);
// Character delete 
RdStatus RdCharacterDelete(RdInterface* rdi, RdId characterWriteId);
// Create text window 
RdStatus RdTextWindowCreate(RdInterface* rdi, RdId layerId, RdPosition position, RdSize size, RdId fontId, RdColor fontColor, RdHDirection scrollDirection, RdUWord* textWindowId);
// Insert point for text window 
RdStatus RdTextWindowSetInsertionPoint(RdInterface* rdi, RdId textWindowId, RdPosition position);
// Insert text in text window 
RdStatus RdTextWindowInsertText(RdInterface* rdi, RdId textWindowId, const char* stringData);
// text window delete 
RdStatus RdTextWindowDelete(RdInterface* rdi, RdId textWindowId);

// Graph commands 


// create line graph window 
// Returns graph id of newly created graph 
RdStatus RdLineGraphCreateWindow(RdInterface* rdi, RdId layerId, RdPosition position, RdSize size, RdByte lineWidth, RdByte lineGlowWidth, RdFlag autoCompose, RdId* graphId);
// set the line graph start point 
RdStatus RdLineGraphInsertPoints(RdInterface* rdi, RdId graphId, RdColor pointColor, RdUWord pointLength, const RdPosition* points);
// set line graph shift point 
RdStatus RdLineGraphMove(RdInterface* rdi, RdId graphId, RdUWord left, RdUWord top, RdUWord right, RdUWord bottom);
// delete Line Graph window
RdStatus RdLineGraphDeleteWindow(RdInterface* rdi, RdId graphId);
// create bar graph window returns graph id 
RdStatus RdBarGraphCreateWindow(RdInterface* rdi, RdId layerId, RdPosition position, RdSize size, RdByte stackSize, RdDirection stackDirection, RdFlag autoCompose, RdId* graphId);
// insert stacks 
RdStatus RdBarGraphInsertStacks(RdInterface* rdi, RdId graphId, RdByte noOfStack, RdId imageId);
// remove stacks 
RdStatus RdBarGraphRemoveStacks(RdInterface* rdi, RdId graphId, RdByte noOfStack);
// delete Bar Graph window
RdStatus RdBarGraphDeleteWindow(RdInterface* rdi, RdId graphId);

// Touch map commands 

// create MapRectangle
// Returns MapRectangle id 
RdStatus RdTouchMapRectangle(RdInterface* rdi, RdPosition position, RdSize size, const char* label, RdId* touchId);
// create MapCircle
// Returns MapCircle id 
RdStatus RdTouchMapCircle(RdInterface* rdi, RdPosition position, RdUWord outerCircleRadius, RdUWord innerCircleRadius, const char* label, RdId* touchId);
// TouchMap delete shape 
RdStatus RdTouchMapDelete(RdInterface* rdi, RdId touchId);
// TouchMap Clear all 
RdStatus RdTouchMapClear(RdInterface* rdi);

// Information commands 
RdStatus RdSystemInfo(RdInterface* rdi, RdSystemInfoType type, char** output);

// Other commands 

// Reset all 
RdStatus RdReset(RdInterface* rdi);
// Test Echo
RdStatus RdTestEcho(RdInterface* rdi, const char* label, char** output);
// free data 
RdStatus RdFreeData(void* data);
//event data
RdStatus RdGetEventData(RdInterface* rdi, RdEvent** event, RdUWord* count);
//event type
RdStatus RdSetEventType(RdInterface* rdi, RdUWord eventType);
// Flash commands 

// Set FlashWrite Enable 
RdStatus RdFlashWriteEnable(RdInterface* rdi, RdFlag enable);
// Start file transfer
// Returns Transfer id 
RdStatus RdFlashImage(RdInterface* rdi, RdUWord type, const char* filename, RdUInt32 length, RdId* transferId);
// File transfer chunk 
RdStatus RdFlashData(RdInterface* rdi, RdId transferId, RdUWord type, const char* data, RdUWord len);
// File delete 
RdStatus RdFlashDelete(RdInterface* rdi, RdUWord type, const char* filename);
// File delete all 
RdStatus RdFlashDeleteAll(RdInterface* rdi);

// Configuration 

// Get Max BackLight Brightness 
RdStatus RdGetMaxBackLightBrightness(RdInterface* rdi, RdUWord* maxBacklightBrightness);
// Get BackLight Brightness 
RdStatus RdGetBackLightBrightness(RdInterface* rdi, RdUWord* backlightBrightness);
// Set BackLight Brightness 
RdStatus RdSetBackLightBrightness(RdInterface* rdi, RdUWord backlightBrightness);

// Flush driver response frame
RdStatus RdFlushSetDefaults(RdInterface* rd_interface, int len);
// Set Response RetryCount and RetryDelay value
RdStatus RdSetRetryCntAndDelay(RdInterface* rd_interface, int ResponseRetryCount, int ResponseRetryDelay);
// Set Delay between write and read & between command
RdStatus RdSetDelay(RdInterface* rd_interface, int writeDelay, int cmdDelay);

#ifdef  __cplusplus
}
#endif


#endif


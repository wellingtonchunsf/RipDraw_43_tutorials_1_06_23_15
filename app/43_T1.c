/* 43_T1.c  Version 3.0T 6/23/15 RipDraw Tutorial Application that demonstrates RipDraw image, text and background commands
 * 
 * makefile modified for current RipDraw C directory structure
 * removed RdInitialize() for RipdrawLinuxSerialInit()  
 * change #include ripdraw.h to #include Ripdraw.h  for case sensitivity for linux file system/tools
 *
 * Tested with RipDraw C Library 
 * Version:  3.00 Date of Release May 01, 2015
 *
 * Sample RipDraw Tutorial application program to put up simple images onto layer 2
 *
 * General steps 
 *   - initialize 		RipdrawLinuxSerialInit() 
 *   - reset 			RdReset()			Display will go blank here
 *   - enable layer 		RdSetLayerEnable()
 *   - set background color  	RdSetLayerBackColor()
 *   - load images from font	RdFontLoad()
 *   - write string to display  RdStringWrite()
 *   - replace string 		RdStringReplace()
 *   - load images from flash	RdImageLoad()
 *   - write images to layer	RdImageWrite()
 *   - move image		RdImageMove()
 *   - close off interface 	RdClose()
 */

#include <stdio.h>
#include "Ripdraw.h"

int main(int argc, char **argv)
{

	#define STATUS_OK 0
	#define RD_TRUE 1
	#define Y_MIDDLE 272/2

	int i;

	int ret;			/* return value from all RipDraw library calls, zero is STATUS_OK */
	RdInterface* rd_interface;	/* Handle to the RipDraw display */
	RdId layer;			/* Layers, 1 to 2 */
	RdId font_layer;		/* Layers, 1 to 2 */
	RdId page;			/* Page 1, only one page,  other display may have more */

	RdId	font_id;		/* font_id */
	RdId	string_id;		/* string id */
    
	/* start position and storage of position */
	/* image id and write id storage for each image */
	#define X_GREEN		98
	#define Y_GREEN		Y_MIDDLE
	RdUWord x_green;
	RdUWord y_green;
	RdId image_id_green = 0;
	RdId write_id_green = 0;

	/* start position and storage of position */
	/* image id and write id storage for each image */
	#define X_YELLOW	192	
	#define Y_YELLOW	Y_MIDDLE
	RdUWord x_yellow;
	RdUWord y_yellow;
	RdId image_id_yellow = 0;
	RdId write_id_yellow = 0;

	/* start position and storage of position */
	/* image id and write id storage for each image */
	#define X_RED		288	
	#define Y_RED		Y_MIDDLE
	RdUWord x_red;
	RdUWord y_red;
	RdId image_id_red = 0;
	RdId write_id_red = 0;

	/* start position and storage of position */
	/* image id and write id storage for each image */
	#define X_BLUE		384	
	#define Y_BLUE		Y_MIDDLE	
	RdId image_id_blue = 0;
	RdId write_id_blue = 0;
	RdUWord x_blue;
	RdUWord y_blue;


	/*
	 * Initialize the RipDraw library
	 * Connect the system port to a RipDraw display interface handle
	 *
	 * The RipDraw display interface handle is used direct all RipDraw commands to the specific RipDraw display
	 *
	 * For the Beagle board, the USB port shows up as "/dev/ttyACMO"
	 */

	/* malloc space for rd_interface object */
	rd_interface = malloc(sizeof(RdInterface));
	if (rd_interface == NULL) return; /* if handle is NULL, the malloc failed */

	memset(rd_interface,0,sizeof(RdInterface)); 

    	ret = RipdrawLinuxSerialInit(rd_interface, "/dev/ttyACM0");
    	if (ret != RdErrorSuccess)
    	{
        	printf("Failed with error code : %x", ret);
        	return ret;
    	}

	/* Issue reset to RipDraw display
	* if you are single stepping, the screen will go blank
	*/
	ret = RdReset(rd_interface);
	if (ret !=  STATUS_OK ) return ret;

	/* 
	 * Enable layers  on RipDraw display
	 * There are two layers beginning with 1
	 * Layer numbers are numbered the same as "floor numbers", 1 is the lowest, 2 is the highest
	 * The user sees layers from the "top" down to the lowest layer, hence the higher layer can cover a lower layer
	 *
	 * This example is both using layer 1 and 2 , so we only enable layer 1 and 2
	 * The same command is used to disable the layer 
	 */
	layer =1;
	ret = RdSetLayerEnable(rd_interface, layer, RD_TRUE);
	if (ret != STATUS_OK) return ret;
	
	layer =2;
	ret = RdSetLayerEnable(rd_interface, layer, RD_TRUE);
	if (ret != STATUS_OK) return ret;

	/*
	 * Set background color for layer 1 and 2 
	 * RdCreateColor() is function that packs the red, green, blue and alpha values into a structure
	 * color values  RGB= 0xFF is white,  RGB = 0x00 is black 
	 *
	 */
	#define	RED_BACK	0x00
	#define	GREEN_BACK	0x00
	#define	BLUE_BACK	0xff	
	#define ALPHA		0xff	

	layer =1;
	ret =RdSetLayerBackColor(rd_interface, layer, RdCreateColor(RED_BACK, GREEN_BACK, BLUE_BACK,ALPHA));

	layer =2;
	ret =RdSetLayerBackColor(rd_interface, layer, RdCreateColor(RED_BACK, GREEN_BACK, BLUE_BACK,ALPHA));

	/* 
	 * Load font for display 
	 * For each font set that are in the RipDraw flash, load from flash into memory using the filename 
	 * RdFrontLoad returns an font_id, font_id is the hand to the font, font only needs to be loaded once
	 * This takes a bit of time
	 */
	ret = RdFontLoad(rd_interface, "ConsolaMono_20", &font_id);
	if (ret != STATUS_OK) return ret;

	#define X_FONT 100	
	#define Y_FONT	220 
	#define	RD_HDIRECTION_LEFT 0

	/*
	 * Write text to display using the font_id to identify the font to use
	 * RdStringWrite() returns a string_id
	 * Any manipulations to the string will now use string_id for that string
	 */
	font_layer = 2;
    	ret = RdStringWrite(rd_interface, font_layer, RdCreatePosition(X_FONT,Y_FONT), font_id, RdCreateColor(0xFF, 0xFF, 0xFF, 0xFF), RD_HDIRECTION_LEFT, "RipDraw 4.3  Tutorial 1", &string_id);
	if (ret != STATUS_OK) return ret;

	/*
	 * For each of the images that are in the RipDraw flash, load from flash into memory using the filename 
	 * RdImageLoad returns an image_id, image_id is the handled to image,  image only needs to be loaded once
	 *
	 * Images MUST be already loaded into the RipDraw flash, typically by the emulator
	 *
	 */
	ret = RdImageLoad(rd_interface, "green-on", &image_id_green);
	if (ret != STATUS_OK) return ret;

	ret = RdImageLoad(rd_interface, "yellow-on", &image_id_yellow);
	if (ret != STATUS_OK) return ret;

	ret = RdImageLoad(rd_interface, "red-on", &image_id_red);
	if (ret != STATUS_OK) return ret;

	ret = RdImageLoad(rd_interface, "peacock-blue-on", &image_id_blue);
	if (ret != STATUS_OK) return ret;

	
	/*
	 * For each of the images that are now in memory, write the image using the image_id to a specific position in a specific layer 
	 * RdImageWrite returns an write_id, write_id the handled to written image
	 * The write_id is used to manipulate this written image on the layer
	 * You can Rd_ImageWrite() an image multiple times to various or same layers, each time a unique write_id is returned so that each written image
	 * can be individually manipulated
	 *
	 * RdCreatePosition() is a function to pack X,Y into data structure
	 * The x,y positon is the position for the upper left hand corner of the image
	 * 
	 * If you are single stepping, the image will show up on the display at each RdImageWrite()
	 */

	x_green = X_GREEN;
	y_green = Y_GREEN;
	ret = RdImageWrite(rd_interface, layer, image_id_green, RdCreatePosition(x_green,y_green), &write_id_green);
	if (ret != STATUS_OK) return ret;

	x_yellow = X_YELLOW;
	y_yellow = Y_YELLOW;
	ret = RdImageWrite(rd_interface, layer, image_id_yellow, RdCreatePosition(x_yellow, y_yellow), &write_id_yellow);
	if (ret != STATUS_OK) return ret;

	x_red = X_RED;
	y_red = Y_RED;
	ret = RdImageWrite(rd_interface, layer, image_id_red, RdCreatePosition(x_red, y_red), &write_id_red);
	if (ret != STATUS_OK) return ret;

	x_blue = X_BLUE;
	y_blue = Y_BLUE;
	ret = RdImageWrite(rd_interface, layer, image_id_blue, RdCreatePosition(x_blue, y_blue), &write_id_blue);
	if (ret != STATUS_OK) return ret;

	/* 
	 * Demonstrate how to move images around layer 
	 *
	 */
	//#define	PAUSE_TIME		500	/* number of usleep tics to pause for user to see */	
	#define	PAUSE_TIME		5	/* number of usleep tics to pause for user to see */	
	#define VERTICAL_LOOPCOUNT	35	/* number of vertical moves */
	#define VERTICAL_DELTA		2	/* vertical displacement */

	/*
	 * Change string with RdStringReplace()
	 */
	ret = RdStringReplace(rd_interface, string_id, "Moving Vertical");
	if (ret !=  STATUS_OK ) return ret;		

	/* note x,y 0,0 is upper left hand corner of display */
	for (i = 0; i < VERTICAL_LOOPCOUNT; i++)
	{
		y_green = y_green 	+ VERTICAL_DELTA; 	/* move down */
		y_yellow = y_yellow 	- VERTICAL_DELTA;	/* move up   */
		y_red = y_red 		+ VERTICAL_DELTA;	/* move down */
		y_blue = y_blue 	- VERTICAL_DELTA;	/* move up  */

		ret = RdImageMove(rd_interface, write_id_green ,RdCreatePosition(x_green,y_green));
		if (ret !=  STATUS_OK ) return ret;		

		ret = RdImageMove(rd_interface, write_id_yellow ,RdCreatePosition(x_yellow,y_yellow));
		if (ret !=  STATUS_OK ) return ret;		

		ret = RdImageMove(rd_interface, write_id_red ,RdCreatePosition(x_red,y_red));
		if (ret !=  STATUS_OK ) return ret;		

		ret = RdImageMove(rd_interface, write_id_blue ,RdCreatePosition(x_blue,y_blue));
		if (ret !=  STATUS_OK ) return ret;		
		
		usleep(PAUSE_TIME);
	}

	/*
	 * Change string with RdStringReplace()
	 */
	ret = RdStringReplace(rd_interface, string_id, "Moving Horizontal");
	if (ret !=  STATUS_OK ) return ret;		

	#define HORIZONTAL_LOOPCOUNT	15	/* number of horizontal moves */
	#define HORIZONTAL_DELTA	2	/* vertical displacement */
	for (i = 0; i < HORIZONTAL_LOOPCOUNT; i++)
	{
		x_green = x_green 	+ HORIZONTAL_DELTA; 	/* move right */
		x_yellow = x_yellow 	- HORIZONTAL_DELTA;	/* move left  */
		x_red = x_red 		+ HORIZONTAL_DELTA;	/* move right */
		x_blue = x_blue 	- HORIZONTAL_DELTA;	/* move left  */

		ret = RdImageMove(rd_interface, write_id_green ,RdCreatePosition(x_green,y_green));
		if (ret !=  STATUS_OK ) return ret;		

		ret = RdImageMove(rd_interface, write_id_yellow ,RdCreatePosition(x_yellow,y_yellow));
		if (ret !=  STATUS_OK ) return ret;		

		ret = RdImageMove(rd_interface, write_id_red ,RdCreatePosition(x_red,y_red));
		if (ret !=  STATUS_OK ) return ret;		

		ret = RdImageMove(rd_interface, write_id_blue ,RdCreatePosition(x_blue,y_blue));
		if (ret !=  STATUS_OK ) return ret;		
		
		usleep(PAUSE_TIME);
	}

	/*
	 * Change string with RdStringReplace()
	 */
	ret = RdStringReplace(rd_interface, string_id, "Moving Vertical Again");
	if (ret !=  STATUS_OK ) return ret;		

	for (i = 0; i < VERTICAL_LOOPCOUNT; i++)
	{
		y_green = y_green 	- VERTICAL_DELTA; 	/* move up    */
		y_yellow = y_yellow 	+ VERTICAL_DELTA;	/* move down  */
		y_red = y_red 		- VERTICAL_DELTA;	/* move up    */
		y_blue = y_blue 	+ VERTICAL_DELTA;	/* move down  */

		ret = RdImageMove(rd_interface, write_id_green ,RdCreatePosition(x_green,y_green));
		if (ret !=  STATUS_OK ) return ret;		

		ret = RdImageMove(rd_interface, write_id_yellow ,RdCreatePosition(x_yellow,y_yellow));
		if (ret !=  STATUS_OK ) return ret;		

		ret = RdImageMove(rd_interface, write_id_red ,RdCreatePosition(x_red,y_red));
		if (ret !=  STATUS_OK ) return ret;		

		ret = RdImageMove(rd_interface, write_id_blue ,RdCreatePosition(x_blue,y_blue));
		if (ret !=  STATUS_OK ) return ret;		
		
		usleep(PAUSE_TIME);
	}

	/*
	 * Change string with RdStringReplace()
	 */
	ret = RdStringReplace(rd_interface, string_id, "Moving Horizontal Again");
	if (ret !=  STATUS_OK ) return ret;		

	for (i = 0; i < HORIZONTAL_LOOPCOUNT; i++)
	{
		x_green = x_green 	- HORIZONTAL_DELTA; 	/* move left   */
		x_yellow = x_yellow 	+ HORIZONTAL_DELTA;	/* move right  */
		x_red = x_red 		- HORIZONTAL_DELTA;	/* move left   */
		x_blue = x_blue 	+ HORIZONTAL_DELTA;	/* move right  */

		ret = RdImageMove(rd_interface, write_id_green ,RdCreatePosition(x_green,y_green));
		if (ret !=  STATUS_OK ) return ret;		

		ret = RdImageMove(rd_interface, write_id_yellow ,RdCreatePosition(x_yellow,y_yellow));
		if (ret !=  STATUS_OK ) return ret;		

		ret = RdImageMove(rd_interface, write_id_red ,RdCreatePosition(x_red,y_red));
		if (ret !=  STATUS_OK ) return ret;		

		ret = RdImageMove(rd_interface, write_id_blue ,RdCreatePosition(x_blue,y_blue));
		if (ret !=  STATUS_OK ) return ret;		
		
		usleep(PAUSE_TIME);
	}

	/*
	 * Change string with RdStringReplace()
	 */
	ret = RdStringReplace(rd_interface, string_id, "Back To Original Positions");
	if (ret !=  STATUS_OK ) return ret;		

	/*  
	 *
	 *  Currently RdComposeLayersToPage() and RDPageToScreen() are NOT implemented in the 1RU and images will appear after
	 *  an RdImageWrite(),  This code is commented out and left in code as an example in case it is implemented at a later date.
	 *
	 *
	 * Compose all layers to a page and everything should show up
	 *
	 * All images on the each layer is composed into one image on a page 
	 * The image if is composed to page 1, will be displayed
	page = 1;
	ret = RdComposeLayersToPage(rd_interface, page);
	if (ret != STATUS_OK) return ret;
	*/

	/* 
	 * Page to Screen
	 *
	 * RdPageToScreen() connects page to the screen
	 *
	 * At startup, page 1 is already connected to the screen, so when you RdComposeLayersToPage() to page 1, the display will show the composed images
	 * If you RdComposeLayersToPage() to page 2, the display NOT will show the composed images until you RdPageToScreen() to page 2
	 * You can also use RdPageToScreen() to quickly switch between pages on the display
	ret = RdPageToScreen(rd_interface,page);
	if (ret != STATUS_OK) return ret;
	 */ 

	/* 
	 * close off the interface to clean things up for the OS
	 */
	ret = RdClose(rd_interface);
	return 0;
}

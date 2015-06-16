43_T1.c  Version 3.0T 6/23/15 Ripdraw Tutorial Application that demonstrates Ripdraw image, text and background commands

Run on Beagle Bone Black Host computer using USB port to RipDraw

Tested with Ripdraw C Library 
Version:  3.00 Date of Release May 01, 2015

 makefile modified for current RipDraw C directory structure
 removed RdInitialize() for RipdrawLinuxSerialInit() 
 changed log.o to Log.o to handle case sensitivity for linux file system/tools
 changed RipDrawCommon.o to RipdrawCommon.o to handle case sensitivity for linux file system/tools
 added lines to makefile to switch to BBB native tools

 
  Sample Ripdraw Tutorial application program to put up simple images onto layer 2
 
 General steps 
    - initialize 		RipdrawLinuxSerialInit()
    - reset 			RdReset()			Display will go blank here
    - enable layer 		RdSetLayerEnable()
    - set background color  	RdSetLayerBackColor()
    - load images from font	RdFontLoad()
    - write string to display   RdStringWrite()
    - replace string 		RdStringReplace()
    - load images from flash	RdImageLoad()
    - write images to layer	RdImageWrite()
    - move image		RdImageMove()
    - close off interface 	RdClose()


The code was generated using the gcc version 4.8.1, Yagarto tool chain and Eclipses IDE.

The demo program is distributed in the following directory structure

application directory
   app				source files
   clib				RipDraw C Libraries
   fonts			font files
   imagelists			imagelists files
   images			image files
   object 			object file directory

A makefile is included in the distribution.
An executable is included, but his hardwired to /dev/tty/ACM0 and will work without changes if your USB is the same.

The image files must be preloaded into the Ripdraw display via the emulator before you run this program.


To switch Makefile to run use BBB native compiler tools implement these changes to the makefile 
# gcc binaries to use for compile and link
CC = "C:\gcc-linaro\bin\arm-linux-gnueabihf-gcc.exe"
LD = "C:\gcc-linaro\bin\arm-linux-gnueabihf-gcc.exe"

# to switch from windows development to BBB native compiler tools, commment out the arm-linux gcc.exe and use the following
# CC = "/usr/bin/cc"
# LD = "/usr/bin/cc"

# rm is part of yagarto-tools
# to switch from windows development to BBB native compiler tools, comment out the "SHELL = cmd"
SHELL = cmd
REMOVE = rm -f


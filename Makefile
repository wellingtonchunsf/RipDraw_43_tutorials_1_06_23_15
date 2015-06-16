# 
# makefile Version 3.0T 6/1/15 Tutorial Makefile;  Makefile for Beagle Bone Cross Compile on Windows
# 
# makefile modified for current RipDraw C directory structure
# changed log.o to Log.o to handle case sensitivity for linux file system/tools
# changed RipDrawCommon.o to RipdrawCommon.o to handle case sensitivity for linux file system/tools
#

# Set PROJECT to main file
PROJECT=43_T1

# Set RIPDRAW to directory for and ripdraw core 
RIPDRAW_CINT = 	./clib/cint
RIPDRAW_COMMON =./clib/common
RIPDRAW_BASE = 	./clib/cbase
RIPDRAW_EXTLIB = ./clib/extlib

# Directory for C-Source
# Set vpath to where the c files are for application and ripdraw core
# ./source is directory for application code
# $(RIPDRAW) is directory for ripdraw core 
vpath %.c $(CURDIR)/app $(RIPDRAW_CINT) $(RIPDRAW_BASE) $(RIPDRAW_COMMON) $(RIPDRAW_EXTLIB) 

# Directory for includes
# Set variable for where include files are 
# ./include for application code include files
#CINCLUDE = $(CURDIR)/include 

# Directory for object files
OBJDIR = $(CURDIR)/object

# Other dependencies
DEPS = \
 Makefile 
#$(RIPDRAW)/include/*.h 
# $(CURDIR)/include/*.h 	NOTE no .h files for fiveb, uncomment if you add .h file

# Compiler object files 
# add name of any application files, other than main file (PROJECT)
COBJ = \
 $(OBJDIR)/$(PROJECT).o \
 $(OBJDIR)/RipdrawCommon.o \
 $(OBJDIR)/StringUtils.o \
 $(OBJDIR)/Log.o \
 $(OBJDIR)/ProtoBuffer.o \
 $(OBJDIR)/Ripdraw.o \
 $(OBJDIR)/RipdrawLinuxSerial.o \
 $(OBJDIR)/BaseApi.o \
 $(OBJDIR)/DownloadApi.o 


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

# Build Compiler options by concatenating into one string
CFLAGS = -marm
CFLAGS += -O0 
CFLAGS += -g 
#CFLAGS += -I$(CINCLUDE)
CFLAGS += -I$(RIPDRAW_COMMON)

# for a better output
MSG_EMPTYLINE = . 
MSG_COMPILING = ---COMPILE--- 
MSG_LINKING = ---LINK--- 
MSG_SUCCESS = ---SUCCESS--- 

# Our favourite
all: $(PROJECT)

# Linker call
$(PROJECT): $(COBJ)
	@echo $(MSG_EMPTYLINE)
	@echo $(MSG_LINKING)
	$(LD) -o $@ $^ $(CFLAGS)
	@echo $(MSG_EMPTYLINE)
	@echo $(MSG_SUCCESS) $(PROJECT)

# Compiler call
$(COBJ): $(OBJDIR)/%.o: %.c $(DEPS)
	@echo $(MSG_EMPTYLINE)
	@echo $(MSG_COMPILING) $<
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	$(REMOVE) $(OBJDIR)/*.o
	$(REMOVE) $(PROJECT)


//	Copyright 2015 Densitron Technologies PLC. All rights reserved. 
//
//	Version:  3.00 Date of Release May 01, 2015
//
//	Supports Windows and Linux

#include "../common/Ripdraw.h"

typedef struct _RdInterfaceLinuxSerial
{
	int handle;
} RdInterfaceLinuxSerial;

// write data to serial
static int  RipdrawLinuxSerialWrite(RdInterface* rdi, RdByte* dataPtr, int dataLen)
{
	RdInterfaceLinuxSerial* extint = (RdInterfaceLinuxSerial*)rdi->extint;
	int totalWrite = 0;
	int bytesWrite;
	_RD_CHECK_INTERFACE();

	do
	{
		bytesWrite = write(extint->handle, dataPtr, dataLen - totalWrite);
		if (bytesWrite < 0)
		{
			return bytesWrite;
		}
		totalWrite += bytesWrite;
		dataPtr += bytesWrite;
	} while (totalWrite < dataLen);
	return 0;
}

// read data from serial 
static int  RipdrawLinuxSerialRead(RdInterface* rdi, RdByte* dataPtr, int dataLen)
{
	RdInterfaceLinuxSerial* extint = (RdInterfaceLinuxSerial*)rdi->extint;
	int totalRead = 0;
	int bytesRead;
	_RD_CHECK_INTERFACE();

	do
	{
		bytesRead = read(extint->handle, dataPtr, dataLen - totalRead);
		if (bytesRead < 0)
		{
			return bytesRead;
		}
		totalRead += bytesRead;
		dataPtr += bytesRead;
	} while (totalRead < dataLen);
	return 0;
}

// close serial port 
static int  RipdrawLinuxSerialClose(RdInterface* rdi)
{
	RdInterfaceLinuxSerial* extint = (RdInterfaceLinuxSerial*)rdi->extint;
	_RD_CHECK_INTERFACE();

	rdi->isOpen = 0;
	close(extint->handle);
	free(extint);

	return 0;
}

// open the serial port 
int RipdrawLinuxSerialInit(RdInterface* rdi, char* portName)
{
	speed_t speed = B115200;
	int handle;
	struct termios settings;

	if (rdi == NULL)
	{
		RD_ERR("interface should not NULL\n");
		return RdErrorInterfaceNull;
	}

	handle = open(portName, O_RDWR | O_NOCTTY);
	if (handle < 0)
	{
		RD_ERR("port open failed\n");
		return RdErrorPortOpenFailed;
	}

	if (tcgetattr(handle, &settings) < 0)

	{
		RD_ERR("port initialization failed\n");
		return RdErrorPortInitFailed;
	}

	// set input mode to raw, no echo
	// set output mode to raw 
	cfmakeraw(&settings);

	// blocking mode 
	settings.c_cc[VMIN] = 1;
	settings.c_cc[VTIME] = 10;

	settings.c_line = N_TTY;

	// Set the baud rate for both input and output. 
	if ((cfsetispeed(&settings, speed) < 0) || (cfsetospeed(&settings, speed) < 0))
	{
		RD_ERR("port initialization failed\n");
     	return RdErrorPortInitFailed;
	}

	// set no parity, stop bits, data bits 
	settings.c_cflag &= ~PARENB;
	settings.c_cflag &= ~(CSTOPB | CRTSCTS);
	settings.c_cflag &= ~CSIZE;
	settings.c_cflag |= CS8;

	if (tcsetattr(handle, TCSANOW, &settings) < 0)
	{
		RD_ERR("port initialization failed\n");
		return RdErrorPortInitFailed;
	}

	tcflush(handle, TCIOFLUSH);

	rdi->extint = malloc(sizeof(RdInterfaceLinuxSerial));
	if (!rdi->extint)
	{
		close(handle);
		RD_ERR("external interface data not allocated\n");
		return RdErrorExtintAllocFailed;
	}

	rdi->isOpen = 1;
	rdi->extIntType = RD_INTERFACE_TYPE_UART;
	((RdInterfaceLinuxSerial*)rdi->extint)->handle = handle;

	rdi->write = RipdrawLinuxSerialWrite;
	rdi->read = RipdrawLinuxSerialRead;
	rdi->close = RipdrawLinuxSerialClose;

	RdSetRetryCntAndDelay(rdi, RD_INTERFACE_RESPONSE_RETRY_COUNT_DEFAULT, RD_INTERFACE_RESPONSE_RETRY_DELAY_DEFAULT);
	return 0;
}

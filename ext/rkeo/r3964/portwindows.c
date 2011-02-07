/***************************************************************************
 *   Copyright (C) 2006 by Alexander Goryachev                             *
 *   automaticus@gmail.com                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __PORTWINDOWS_H__
#define __PORTWINDOWS_H__

#include "r3964int.h"
#include "../util/util.h"

#if defined WINDOZE
	#include <ctype.h>
#include <stdio.h>

int setTimeout(const THandle *handle, int mSec)
{
	COMMTIMEOUTS timeouts = {0};

	if(!mSec) mSec = 1;

	if (!GetCommTimeouts(handle->desc, &timeouts))
	{
			wLog(handle->errorMessage, "setTimeout: Can't get communication timeouts.\nError: %s", wstrerror());
			return -1;
	}

	timeouts.ReadTotalTimeoutConstant = mSec;

	if (!SetCommTimeouts(handle->desc, &timeouts))
	{
			wLog(handle->errorMessage, "setTimeout: Can't set communication timeouts.\nError: %s", wstrerror());
			return -1;
	}
	return 0;
}

int readChar(const THandle *handle, BYTE *symbol, unsigned timeout)
{
	DWORD retval = 0;

	if(setTimeout(handle, timeout) == -1) return -1;
		if (!ReadFile(handle->desc, symbol, 1, &retval, NULL))
		{
			//error occurred. Report to user.
			wLog(handle->errorMessage, "readChar: Can't read from the port.\nError: %s", wstrerror());
			return -1;
		}
#ifdef DEBUG
if (retval)printf("\n[readChar = %x]",*symbol);
#endif //DEBUG

	return retval;
}

int sendChar(const THandle *handle, BYTE symbol)
{
	DWORD retval = 0;

#ifdef DEBUG
printf("\n[sendChar = %x]",symbol);
#endif //DEBUG

		if (!WriteFile(handle->desc, &symbol, 1, &retval, NULL))
		{
			wLog(handle->errorMessage, "sendChar: Can't write to the port.\nError: %s", wstrerror());
			return -1;
		}

	return 1;
}


EXPORT THandle *createConnection(const char *port, const int baudRate, char parity, int stopBits, bool useBcc,
		int cdt,int adt, int tBlock, int setupAttempts, int transmittingAttempts, char *errorMessage)
{
	THandle *handle;
	DCB dcb = {0};
	errno = 0;
	handle = (THandle*) malloc(sizeof(THandle));
	if(!handle)
	{
	    wLog(errorMessage, "createConnection: Can't allocate the memory.\nError: %s", strerror(errno));
	    return 0;
	}
	memset(handle, 0, sizeof(THandle));

	errno = 0;
	handle->port = (char*) malloc(strlen(port)+1);
	if(!handle->port)
	{
	    wLog(errorMessage, "createConnection: Can't allocate the memory.\nError: %s", strerror(errno));
	    free(handle);
	    return 0;
	}

	strcpy(handle->port, port);
	handle->parity = parity;
	handle->baudRate = baudRate;
	handle->useBcc = useBcc;
	handle->errorMessage = errorMessage;

	switch(stopBits)
	{
		case 1 : handle->stopBits = ONESTOPBIT; break;
		case 2 : handle->stopBits = TWOSTOPBITS; break;
		default: wLog(errorMessage, "createConnection: Not acceptable number of stop bits: <%i>",stopBits);
			destroyConnection(handle);
			return 0;
	}

	handle->desc = CreateFileA(handle->port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (handle->desc == INVALID_HANDLE_VALUE)
	{
		//some error occurred. Inform user.
		if (GetLastError()) // == ERROR_FILE_NOT_FOUND)
		{
			wLog(errorMessage, "createConnection: Can't open the port: <%s>\nError: %s", handle->port, wstrerror());
			handle->desc = 0;
			destroyConnection(handle);
			return 0;
		}
	}

	dcb.DCBlength = sizeof(dcb);
	if (!GetCommState(handle->desc, &dcb))
	{
		//error getting state
			wLog(errorMessage, "createConnection: Cannot get state of the port.\nError: %s", wstrerror());
			destroyConnection(handle);
			return 0;
	}

	dcb.fOutxCtsFlow = false;
	dcb.fOutxDsrFlow = false;
	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	dcb.fDsrSensitivity = false;
	dcb.fInX = false;
	dcb.fOutX = false;
	dcb.fNull = false;
	dcb.fAbortOnError = false;
	dcb.fOutxCtsFlow = false;
	dcb.fOutxDsrFlow = false;
	dcb.fRtsControl = true;
	dcb.fTXContinueOnXoff = true;
	dcb.StopBits = handle->stopBits;

	dcb.ByteSize = 8;
	dcb.fBinary = true;
	dcb.fParity = true;

	switch (handle->baudRate)
	{
		case 300 : dcb.BaudRate = CBR_300; break;
		case 600 : dcb.BaudRate = CBR_600; break;
		case 1200 : dcb.BaudRate = CBR_1200; break;
		case 2400 : dcb.BaudRate = CBR_2400; break;
		case 4800 : dcb.BaudRate = CBR_4800; break;
		case 9600 : dcb.BaudRate = CBR_9600; break;
		case 19200 : dcb.BaudRate = CBR_19200; break;
		case 38400 : dcb.BaudRate = CBR_38400; break;
		case 57600 : dcb.BaudRate = CBR_57600; break;
#ifdef CBR_76800
		case 76800 : dcb.BaudRate = CBR_76800; break;
#endif

		case 115200 : dcb.BaudRate = CBR_115200; break;
		default: wLog(errorMessage, "createConnection: Unsupported baudRate: <%i>", handle->baudRate);
			destroyConnection(handle);
			return 0;
	}



	switch (tolower(handle->parity))
	{
		case 'e' : dcb.Parity = EVENPARITY;	break;
		case 'o' : dcb.Parity = ODDPARITY; break;
		case 'n' : dcb.Parity = NOPARITY; break;
		default: wLog(errorMessage, "createConnection: Illegal parity mode: <'%c'>", handle->parity);
			destroyConnection(handle);
			return 0;
	}

	if (!SetCommState(handle->desc, &dcb))
	{
			wLog(errorMessage, "createConnection: Cannot set state of the port.\nError: %s", wstrerror());
			destroyConnection(handle);
			return 0;
	}

	if (sendChar(handle, NAK) != 1)
	{
		destroyConnection(handle);
		return 0;
	}

	if(setTimings(handle, cdt, adt, tBlock, setupAttempts, transmittingAttempts) == -1) return 0;

	return handle;
}

EXPORT int destroyConnection(THandle *handle)
{
	int res;
	if(handle->desc && !CloseHandle(handle->desc))
	{
		wLog(handle->errorMessage, "destroyConnection: Can't close the port.\nError: %s", wstrerror());
		res = -1;
	}
	else res = 0;

	free(handle->port);
	free(handle);
	return res;
}

#endif
#endif//__PORTWINDOWS_H__

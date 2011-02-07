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

#ifndef __PORTLINUX_H__
#define __PORTLINUX_H__

#include "r3964int.h"
#include "../util/util.h"

#if !defined WINDOZE
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <termios.h>

int readChar(const THandle *handle, BYTE *symbol, unsigned timeout)
{
  int retval;
  unsigned tmr = getTickCount();

  do
  {
    errno = 0;
    retval = read(handle->desc, symbol, 1);
    if (retval == -1 && errno != EAGAIN)
    {
      wLog(handle->errorMessage, "readChar: Can't read from the port.\nError: %s", strerror(errno));
      return -1;
    }
    if(retval == 1) break;
  } while((getTickCount()-tmr) < timeout || errno == EAGAIN);

#ifdef DEBUG
  if (retval)printf("\n[readChar = %x]",*symbol);
#endif //DEBUG

  return retval;
}

int sendChar(const THandle *handle, BYTE symbol)
{
  int retval;
#ifdef DEBUG
  printf("\n[sendChar = %x]", symbol);
#endif //DEBUG

  do
  {
    errno = 0;
    retval = write(handle->desc, &symbol, 1);
    if (retval == -1 && errno != EAGAIN)
    {
      wLog(handle->errorMessage, "sendChar: Cannot write to the port.\nError: %s", strerror(errno));
      return -1;
    }
  } while(errno == EAGAIN);

  if (retval != 1)
  {
    wLog(handle->errorMessage, "sendChar: Some unknown error occured");
    return -1;
  }
  return 1;
}

EXPORT THandle *createConnection(const char *port, const int baudRate, char parity, int stopBits, bool useBcc,
                                 int cdt,int adt, int tBlock, int setupAttempts, int transmittingAttempts, char *errorMessage)
{
  struct termios theTermio;
  int bdr;
  THandle *handle;

  errno = 0;
  handle = (THandle*) malloc(sizeof(THandle));
  if (!handle)
  {
    wLog(errorMessage, "createConnection: Can't allocate the memory.\nError: %s", strerror(errno));
    return 0;
  }
  memset(handle, 0, sizeof(THandle));

  errno = 0;
  handle->port = (char*) malloc(strlen(port) + 1);
  if (!handle->port)
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

  if(stopBits == 1 || stopBits == 2)handle->stopBits = stopBits;
  else
  {
    wLog(errorMessage, "createConnection: Not acceptable number of stop bits: <%i>",stopBits);
    free(handle->port);
    free(handle);
    return 0;
  }

  switch (handle->baudRate)
  {
  case 110: bdr = B110; break;
  case 150: bdr = B150; break;
  case 300: bdr = B300; break;
  case 600: bdr = B600; break;
  case 1200: bdr = B1200; break;
  case 2400: bdr = B2400; break;
  case 4800: bdr = B4800; break;
  case 9600: bdr = B9600; break;
  case 19200: bdr = B19200; break;
  case 38400: bdr = B38400; break;
  case 57600: bdr = B57600; break;
#ifdef B76800
  case 76800: bdr = B76800; break;
#endif
  case 115200: bdr = B115200; break;
  default: wLog(errorMessage, "createConnection: Unsupported baudRate: <%i>", handle->baudRate);
    free(handle->port);
    free(handle);
    return 0;
  }

  errno = 0;
  if ((handle->desc = open(handle->port, O_RDWR | O_NOCTTY | O_NDELAY)) == -1)
  {
    wLog(errorMessage, "createConnection: Can't open the port: <%s>\nError: %s", handle->port, strerror(errno));
    free(handle->port);
    free(handle);
    return 0;
  }

  errno = 0;
  if(tcgetattr(handle->desc, &theTermio) == -1)
  {
    wLog(errorMessage, "createConnection: Can't get state of the port.\nError: %s", strerror(errno));
    destroyConnection(handle);
    return 0;
  }

  /* set baudRate: */
  errno = 0;
  if(cfsetispeed(&theTermio, bdr) == -1)
  {
    wLog(errorMessage, "createConnection: Can't set input speed.\nError: %s", strerror(errno));
    destroyConnection(handle);
    return 0;
  }

  errno = 0;
  if(cfsetospeed(&theTermio, bdr) == -1)
  {
    wLog(errorMessage, "createConnection: Can't set output speed.\nError: %s", strerror(errno));
    destroyConnection(handle);
    return 0;
  }

  theTermio.c_cflag |= (CLOCAL | CREAD);

  switch (tolower(handle->parity))
  {
  case 'n' :
    theTermio.c_cflag &= ~PARENB;//disable parity bit
    theTermio.c_iflag &= ~ICRNL;
    break;

  case 'e' :
    theTermio.c_cflag |= PARENB;//enable parity bit
    theTermio.c_cflag &= ~PARODD;
    theTermio.c_iflag |= INPCK;
    theTermio.c_iflag &= ~ISTRIP;
    theTermio.c_iflag &= ~ICRNL;
    //			theTermio.c_iflag &= ~INLCR;
    //			theTermio.c_iflag |= IGNCR;
    break;

  case 'o' :
    theTermio.c_cflag |= PARENB;
    theTermio.c_cflag |= PARODD;//use odd parity instead of even
    theTermio.c_iflag &= ~ICRNL;
    theTermio.c_iflag |= INPCK;
    theTermio.c_iflag &= ~ISTRIP;
    break;

  default: wLog(errorMessage, "createConnection: Illegal parity mode: <'%c'>", handle->parity);
    destroyConnection(handle);
    return 0;
  }

  if(handle->stopBits == 2) theTermio.c_cflag |= CSTOPB;
  else theTermio.c_cflag &= ~CSTOPB;

  theTermio.c_cflag &= ~CSIZE;
  theTermio.c_cflag |= CS8;

  theTermio.c_cflag &= ~CRTSCTS;//disable hardware flow control

  theTermio.c_iflag &= ~(IXON | IXOFF | IXANY);//disable software flow control

  theTermio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//raw input
  theTermio.c_oflag &= ~OPOST;//raw output

	theTermio.c_cc[VMIN] = 0;
	theTermio.c_cc[VTIME] = 0;

  //tcflush(handle->desc, TCIFLUSH);

  errno = 0;
  if(tcsetattr(handle->desc, TCSANOW, &theTermio) == -1)
  {
    wLog(errorMessage, "createConnection: Can't set state of the port.\nError: %s", strerror(errno));
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
  int res = close(handle->desc);
  if(res == -1) wLog(handle->errorMessage, "destroyConnection: Can't close the port.\nError: %s", strerror(errno));

  free(handle->port);
  free(handle);
  return res;
}

#endif
#endif//__PORTLINUX_H__

/* //////////////////// FULLY WORKING, BUT NOT ENOUGH TIME PRECISION
int setTimeout(const THandle *handle, int mSec)
{
	static struct termios theTermio;
 
	errno = 0;
	if(tcgetattr(handle->desc, &theTermio) == -1)
	{
		wLog(handle->errorMessage, "setTimeout: Can't get communication timeouts.\nError: %s", strerror(errno));
		return -1;
	}
 
	if(!mSec) fcntl(handle->desc, F_SETFL, FNDELAY); //non blocking mode;
	else fcntl(handle->desc, F_SETFL, 0);
 
	theTermio.c_cc[VTIME] = mSec/100;
 
	errno = 0;
	if(tcsetattr(handle->desc, TCSANOW, &theTermio) == -1)
	{
		wLog(handle->errorMessage, "setTimeout: Can't set communication timeouts.\nError: %s", strerror(errno));
		return -1;
	}
	return 0;
}
 
 
// Use the macro readChar in r3964.h instead of this!
//
//  Reads one symbol from the port.
//  int descriptor   - opened descriptor of the port;
//  BYTE *symbol   - return symbol;
//  unsigned timeout - time in msec, while function
//			will try to read the symbol.
//  Return value - "1" on succes
//		 - "0" on timeout
//		 - "-1" on error
//
int readChar_(const THandle *handle, BYTE *symbol, unsigned timeout,
              const char *file, int line)
{
	int retval;
	if(setTimeout(handle, timeout) == -1) return -1;
 
		do
		{
			errno = 0;
			retval = read(handle->desc, symbol, 1);
			if (retval == -1 && errno != EAGAIN)
			{
				wLog(handle->errorMessage, "readChar: Can't read from the port.\nError: %s", strerror(errno));
				return -1;
			}
		} while(errno == EAGAIN);
 
#ifdef DEBUG
if (retval)printf("\n[readChar = %x]",*symbol);
#endif //DEBUG
 
	return retval;
}
 */

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

/*! \file r3964.h
 Include this file if you want R3964 protocol.*/

#ifndef __R3964_H__
#define __R3964_H__

#ifdef __GNUC__
	#include <stdbool.h>
#else
	#ifndef bool
		#define bool int
	#endif
	#ifndef true
		#define true 1
	#endif
	#ifndef false
		#define false 0
	#endif

	#ifndef __bool_true_false_are_defined
		#define __bool_true_false_are_defined 1
	#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
	#define WINDOZE
#endif
*/
#ifdef DLL_EXPORT
	#define EXPORT __declspec(dllexport)
#elif defined DLL_IMPORT
	#define EXPORT __declspec(dllimport)
#else
	#define EXPORT
#endif

#if !defined WINDOZE
    #include <stdint.h>
    typedef int portDesc;
    typedef uint8_t BYTE;
    typedef uint16_t WORD;
    typedef uint32_t DWORD;
#else
    #include <windows.h>
    typedef HANDLE portDesc;
#endif

typedef struct THandle
{
    char *port;					///<port name
    char parity;				///<parity - "E", "O", "N" (Even, Odd, None)
    int baudRate;				///<baud rate: 300/600/1200/2400/4800/9600/19200/38400/57600/115200
    bool useBcc;				///<use Block Character Check
	int stopBits;				///<number of stop bits
	int cdt;					///<character delay time
	int adt;					///<acknowledgment delay time
	int tBlock;					///<waiting while partner prepares the data
	int setupAttempts;			///<number of setup attempts
	int transmittingAttempts;	///<number of transmitting attempts
	portDesc desc;				///<opened port's handle
	char *errorMessage;			///<buffer for textual error message
}THandle;

/*!Creates and initializes connection to the partner;
  \param port - port name under your OS. Something like "COM1" or "/dev/ttyS0";
  \param baudRate - port speed. One of: 300/600/1200/2400/4800/9600/19200/38400/57600/115200
 			other baud rates are not supported currently;
  \param parity - 'e' - even, 'o' - odd, 'n' - none;
  \param stopBits - "1" or "2".
  \param useBcc - use block check character.<br>
  YOU CAN SET ANY VARIABLES BELOW TO "0", IT'S MEAN - "DEFAULT VALUE"
  \param cdt - character delay time. From 20ms to 655350ms in 10 ms increments. Default - 220ms;
  \param adt - acknowledgment delay time. From 20ms to 655350ms in 10 ms increments.
 			Default - 2000ms for R3964(with BCC) and 550ms for 3964(without BCC);
  \param tBlock - Timeout between request and answer telegram (time, while the partner prepares
 			data). Usually from 500 to 20000ms, depends from baud rate;
  \param setupAttempts - number of setup attempts. Default - 6;
  \param transmittingAttempts - number of transmitting attempts. Default - 6;
  \param *errorMessage - buffer for textual error message;
  \return pointer to the descriptor structure on success, or "0" on fail.*/
EXPORT THandle *createConnection(const char *port, const int baudRate, char parity, int stopBits, bool useBcc,
		int cdt,int adt, int tBlock, int setupAttempts, int transmittingAttempts, char *errorMessage);


/*!Sends (R)3964 telegram.
  \param *handle - descriptor returned by createConnection;
  \param telegramSize - length of the telegram in bytes;
  \param telegram - buffer for fetched telegram (at least 128bytes);
  \param W - retries counter (you have to set it to "0" always);
  \return length of the telegram in bytes on
  success, or "-1" in case of fail;*/
EXPORT int send3964(const THandle *handle, int telegramSize, const BYTE *telegram, int W);

/*!Receives (R)3964 telegram;
  \param *handle - descriptor returned by createConnection;
  \param telegram - buffer for fetched telegram (at least 128bytes);
  \param maxLen - maximum length of telegram in bytes;
  \param W - retries counter (you have to set it to "0" always);
  \return length of the received telegram in bytes on
  success, or "-1" in case of fail.*/
EXPORT int fetch3964(const THandle *handle, BYTE *telegram, int maxLen, int W);

/*!Closes port and frees allocated memory
  \param *handle - descriptor returned by createConnection;
  \returns "0" - on success, "-1" - on fail to close the port.*/
EXPORT int destroyConnection(THandle *handle);

#ifdef __cplusplus
 }
#endif
#endif // #ifndef __R3964_H__

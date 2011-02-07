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

/* \file r3964int.h
 Internals for r3964 protocol*/

#ifndef __R3964INT_H__
#define __R3964INT_H__

#include <sys/types.h>
//#include "../util/util.h"
#include "../r3964.h"

#ifdef __cplusplus
extern "C" {
#endif

// PRIMITIVES ////////////////////////////////////

/*! For internal use only.
  \param *handle - descriptor returned by createConnection;<br>
  YOU CAN SET ANY VARIABLES BELOW TO "0", IT'S MEAN - "DEFAULT VALUE"
  \param cdt - character delay time. From 20ms to 655350ms in 10 ms increments. Default - 220ms;
  \param adt - acknowledgment delay time. From 20ms to 655350ms in 10 ms increments.
 			Default - 2000ms for R3964(with BCC) and 550ms for 3964(without BCC);
  \param tBlock - Timeout between request and answer telegram (time, while the partner prepares
 			data). Usually from 500 to 20000ms, depends from baud rate;
  \param setupAttempts - number of setup attempts. Default - 6;
  \param transmittingAttempts - number of transmitting attempts. Default - 6;
  \return "0" on success, or "-1" in case of fail;*/
int setTimings(THandle *handle, int cdt,int adt, int tBlock, int setupAttempts, int transmittingAttempts);

/*! For internal use only. Reads one symbol from the port;
  \param *handle - descriptor returned by createConnection;
  \param symbol - return symbol;
  \param timeout - maximum timeout in milliseconds to wait for a symbol;
  \return "1" - success; "-1" - error.*/
int readChar(const THandle *handle, BYTE *symbol, unsigned timeout);


/*! For internal use only. Sends one symbol to the port;
  \param *handle - descriptor returned by createConnection;
  \param symbol - symbol to send;
  \return "1" - success; "-1" - error.*/
int sendChar(const THandle *handle, BYTE symbol);

/*
 * Common ascii handshake characters:
 */

#define STX 0x02
#define ETX 0x03
#define DLE 0x10
#define NAK 0x15

#ifdef __cplusplus
 }
#endif
#endif //__RK512INT_H__

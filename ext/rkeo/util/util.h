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

#ifndef __UTIL_H__
#define __UTIL_H__

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

 //to call after functions that return LastError
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
/*! \return Last error for WinAPI functions.*/
	char *wstrerror(void);
#endif

/*! For internal use only.
  \return Time of the day in milliseconds under Linux;*/
unsigned getTickCount(void);

/*! Wait for a time period (crossplatform).
 \param dl - time in milliseconds;*/
void delay(unsigned dl);

/*! Logger function.
 \param *target - buffer where to write error message;
 \param *fmt - printf's format string.
 \param "..." - parameters for format string.*/
void wLog(char *target, char *fmt, ...);

/*! \return Array with date and time.*/
char *curTime(void);

/*! Converts character to an array of bits.
 \param c - input character;
 \return zero terminated string with binary number.*/
char *bin2Str(unsigned char c);

#ifdef __cplusplus
 }
#endif
#endif //__UTIL_H__

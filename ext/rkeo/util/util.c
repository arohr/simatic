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

#include <sys/timeb.h>

#include <time.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include "util.h"

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
    #include <windows.h>
#else
    #include <sys/time.h>
#endif

void wLog(char *target, char *fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);

	if(!target){va_end(argptr);return;}

	/*-----------------08.11.2006 12:30-----------------
	 * Removed carriage return and curTime()
	 * --------------------------------------------------*/
	//sprintf(target, "\n%s ", curTime());
	target[0] = '\0';

//#warning Can cose buffer overflow!!!
    vsprintf(target+strlen(target), fmt, argptr);
    va_end(argptr);
    strcat(target, ";");
}

#if !defined(__WIN32__) && !defined(_WIN32) && !defined(WIN32)

 unsigned getTickCount(void)
{
	struct timeval now;

	gettimeofday(&now, 0);

	return (now.tv_usec / 1000) + (now.tv_sec * 1000);
}
#endif

void delay(unsigned dl)
{
#if !defined(__WIN32__) && !defined(_WIN32) && !defined(WIN32)
	unsigned timeStart;

	timeStart = getTickCount();
	while(getTickCount() - timeStart < dl);
#else
	#include <windows.h>
	Sleep(dl);
#endif
}

char *curTime(void)
{
	static char *Month[12] =
		{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
"Nov", "Dec" };
	static char date[16];
	struct tm *tblock;
	time_t timer;

	timer = time(NULL);
	tblock = localtime(&timer);
	++tblock->tm_mon;
	sprintf(date, "%s %02u %02u:%02u:%02u", Month[tblock->tm_mon - 1],
					tblock->tm_mday, tblock->tm_hour, tblock->tm_min, tblock->tm_sec);
	return date;
}

char *bin2Str(unsigned char c)
{
	static char str[9];

	int i = 0;

	for(; i <= 7; i++)
	{
		str[i] = (0x80 >> i & c) ? '1' : '0';
	}
	str[++i] = 0;
	return str;
}


//to call after functions that return LastError
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
char *wstrerror(void)
{
		static char lastError[1024];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(),
		              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lastError, 1024, NULL);
					  return lastError;

}
#endif

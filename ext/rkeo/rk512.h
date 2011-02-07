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

/*! \file rk512.h
 Include this file if you want rk512 protocol.*/

#ifndef __RK512_H__
#define __RK512_H__

#include "r3964.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!Send Rk512 telegram. Bytes version;
  \param *handle - descriptor returned by createConnection;
  \param dbnr - number of the DB;
  \param dbbnr - number of the first DBB. It have to be divisible by 2; USE BYTE ADDRESSATION!!!
  \param sizeInBytes - amount of bytes. It have to be divisible by 2 (0,2,4,6...);
  \param *data - array of data;
  \return value - "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int sendRk512Dbb(const THandle *handle, int dbnr, int dbbnr, int sizeInBytes, const BYTE *data);

/*!Send Rk512 telegram. Words version;
  \param *handle - descriptor returned by createConnection;
  \param dbnr - number of the DB;
  \param dbwnr - number of the first DBW. It have to be divisible by 2. USE BYTE ADDRESSATION!!!
  \param sizeInWords - amount of words;
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int sendRk512Dbw(const THandle *handle, int dbnr, int dbwnr, int sizeInWords, const WORD *data);

/*!Send Rk512 telegram. Double words version;
  \param *handle - descriptor returned by createConnection;
  \param dbnr - number of the DB;
  \param dbdnr - the number of the first DBD. It have to be divisible by 2. USE BYTE ADDRESSATION!!!
  \param sizeInDWords - amount of double words;
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int sendRk512Dbd(const THandle *handle, int dbnr, int dbdnr, int sizeInDWords, const DWORD *data);

/*!Send Rk512 telegram. Float point version;
  \param *handle - descriptor returned by createConnection;
  \param dbnr - number of the DB;
  \param dbdnr - the number of the first DBD. It have to be divisible by 2. USE BYTE ADDRESSATION!!!
  \param sizeInFloats - amount of float point variables;
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int sendRk512DbReal(const THandle *handle, int dbnr, int dbdnr, int sizeInFloats, const float *data);

/*!Receive Rk512 telegram. Bytes version;
  \param *handle - descriptor returned by createConnection;
  \param dbnr - number of the DB;
  \param dbbnr - number of the first DBB. It have to be divisible by 2. USE BYTE ADDRESSATION!!!
  \param sizeInBytes - amount of bytes. It have to be divisible by 2 (0,2,4,6...);
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512Dbb(const THandle *handle, int dbnr, int dbbnr, int sizeInBytes, BYTE *data);

/*!Receive Rk512 telegram. Words version;
  \param *handle - descriptor returned by createConnection;
  \param dbnr - number of the DB;
  \param dbwnr - number of the first DBW. It have to be divisible by 2. USE BYTE ADDRESSATION!!!
  \param sizeInWords - amount of words.
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512Dbw(const THandle *handle, int dbnr, int dbwnr, int sizeInWords, WORD *data);

/*!Receive Rk512 telegram. Double words version;
  \param *handle - descriptor returned by createConnection;
  \param dbnr - number of the DB;
  \param dbdnr - number of the first DBB. It have to be divisible by 2. USE BYTE ADDRESSATION!!!
  \param sizeInDoubleWords - amount of double words.
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512Dbd(const THandle *handle, int dbnr, int dbdnr, int sizeInDoubleWords, DWORD *data);

/*!Receives Rk512 telegram. Float point version;
  \param *handle - descriptor returned by createConnection;
  \param dbnr - number of the DB;
  \param dbdnr - number of the first DBD. It have to be divisible by 2. USE BYTE ADDRESSATION!!!
  \param sizeInFloats - amount of float point variables;
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512DbReal(const THandle *handle, int dbnr, int dbdnr, int sizeInFloats, float *data);

/*!Read flag bytes from the PLC;
  \param *handle - descriptor returned by createConnection;
  \param firstMb - the number of the first byte of flags; USE BYTE ADDRESSATION!!!
  \param sizeInBytes - amount of bytes.
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512Mb(const THandle *handle, int firstMb, int sizeInBytes, BYTE *data);

/*!Read flag words from the PLC;
  \param *handle - descriptor returned by createConnection;
  \param firstMw - the number of the first word of flags; USE BYTE ADDRESSATION!!!
  \param sizeInWords - amount of words.
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512Mw(const THandle *handle, int firstMw, int sizeInWords, WORD *data);

/*!Read flag double words from the PLC;
  \param *handle - descriptor returned by createConnection;
  \param firstMd - number of the first double word of flags; USE BYTE ADDRESSATION!!!
  \param sizeInDWords - amount of double words.
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512Md(const THandle *handle, int firstMd, int sizeInDWords, DWORD *data);

/*!Read flags in float point form from the PLC;
  \param *handle - descriptor returned by createConnection;
  \param firstMd - number of the first double word of flags; USE BYTE ADDRESSATION!!!
  \param sizeInDWords - amount of double words.
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512MdReal(const THandle *handle, int firstMd, int sizeInDWords, float *data);

/*!Read input bytes from the PLC;
  \param *handle - descriptor returned by createConnection;
  \param firstIb - the number of the first byte of inputs;
  \param sizeInBytes - amount of bytes.
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512Ib(const THandle *handle, int firstIb, int sizeInBytes, BYTE *data);

/*!Read input words from the PLC;
  \param *handle - descriptor returned by createConnection;
  \param firstIw - the number of the first word of inputs;
  \param sizeInWords - amount of words.
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512Iw(const THandle *handle, int firstIw, int sizeInWords, WORD *data);

/*!Read input double words from the PLC;
  \param *handle - descriptor returned by createConnection;
  \param firstId - the number of the first double word of flags; USE BYTE ADDRESSATION!!!
  \param sizeInDWords - amount of double words.
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512Id(const THandle *handle, int firstId, int sizeInDWords, DWORD *data);

/*!Read flags in float point form from the PLC;
  \param *handle - descriptor returned by createConnection;
  \param firstId - the number of the first double word of inputs; USE BYTE ADDRESSATION!!!
  \param sizeInDWords - amount of double words.
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512IdReal(const THandle *handle, int firstId, int sizeInDWords, float *data);


/*!Read output bytes from the PLC;
  \param *handle - descriptor returned by createConnection;
  \param firstQb - the number of the first byte of outputs;
  \param sizeInBytes - amount of bytes.
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512Qb(const THandle *handle, int firstQb, int sizeInBytes, BYTE *data);

 /*!Read output words from the PLC;
  \param *handle - descriptor returned by createConnection;
  \param firstQw - the number of the first word of outputs;
  \param sizeInWords - amount of words.
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512Qw(const THandle *handle, int firstQw, int sizeInWords, WORD *data);

/*!Read output double words from the PLC;
  \param *handle - descriptor returned by createConnection;
  \param firstQd - the number of the first double word of outputs; USE BYTE ADDRESSATION!!!
  \param sizeInDWords - amount of double words.
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512Qd(const THandle *handle, int firstQd, int sizeInDWords, DWORD *data);

/*!Read inputs in float point form from the PLC;
  \param *handle - descriptor returned by createConnection;
  \param firstQd - the number of the first double word of output; USE BYTE ADDRESSATION!!!
  \param sizeInDWords - amount of double words.
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512QdReal(const THandle *handle, int firstQd, int sizeInDWords, float *data);

/*!Read timers from the PLC;
  \param *handle - descriptor returned by createConnection;
  \param firstTimer - the number of the first byte of flags; USE BYTE ADDRESSATION!!!
  \param numOfTimers - amount of timers.
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512Timers(const THandle *handle, int firstTimer, int numOfTimers, WORD *data);

/*!Read counters from the PLC;
  \param *handle - descriptor returned by createConnection;
  \param firstCounter - the number of the first counter;
  \param numOfCounters - amount of counters.
  \param *data - array of data;
  \return value "0" - success; "-1" error to send. >0 - error code from the partner.
*/
EXPORT int fetchRk512Counters(const THandle *handle, int firstCounter, int numOfCounters, WORD *data);



#ifdef __cplusplus
 }
#endif
#endif //__RK512_H__

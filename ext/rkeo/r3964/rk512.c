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

#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#if defined(__GNUC__)
 #include <unistd.h>
#endif
#include <errno.h>

#include "../util/util.h"
#include "../r3964.h"

#if !defined WINDOZE
	#include <termios.h>
#endif



int formTelegram (char type, bool isSend, bool isContinue, int dbnr, int dbbnr,
                  int sizeInBytes, const BYTE *data, BYTE *mess, char *errorMessage);


int bytesToWords(WORD *dest, const BYTE *from, int numOfBytes, char *errorMessage)
{
    int words = 0, bytes = 0;

    if(numOfBytes%2)
    {
        wLog(errorMessage, "bytesToWords: You've specified NOT EVEN amount of bytes: <%i>. Can't convert it to words",numOfBytes);
        return -1;
    }

    numOfBytes/=2; //bytes to words

    for (;words < numOfBytes;words++)
    {
        dest[words] = (from[bytes] << 8) | from[bytes + 1];
        bytes += 2;
    }
    return 0;
}

int bytesToDWords(DWORD *dest, const BYTE *from, int numOfBytes, char *errorMessage)
{
    int dWords = 0, bytes = 0;

    if(numOfBytes%4)
    {
        wLog(errorMessage, "bytesToDWords: You have to supply divisible by 4 number of bytes (numOfBytes): <%i>. Can't convert it to double words",numOfBytes);
        return -1;
    }

    numOfBytes/=4; //bytes to DWords

    for (;dWords < numOfBytes;dWords++)
    {
        dest[dWords] = (from[bytes] << 24) | (from[bytes+1] << 16) | (from[bytes+2] << 8) | from[bytes+3];
        bytes += 4;
    }
    return 0;
}

int bytesToFloats(float *dest, const BYTE *from, int numOfBytes, char *errorMessage)
{
    int floats = 0, bytes = 0;
    union
    {
        float Float;
        DWORD DWORD;
    } convert;

    if(numOfBytes%4)
    {
        wLog(errorMessage, "bytesToFloats: You have to supply divisible by 4 number of bytes (numOfBytes): <%i>. Can't convert it to floats",numOfBytes);
        return -1;
    }

    numOfBytes /= 4;

    for (;floats < numOfBytes;floats++)
    {
        convert.DWORD = (from[bytes] << 24) | (from[bytes+1] << 16) | (from[bytes+2] << 8) | from[bytes+3];
        bytes+=4;
        dest[floats] = convert.Float;
    }
    return 0;
}


void wordsToBytes(BYTE *dest, const WORD *from, int numOfWords)
{
    int words = 0, bytes = 0;
    for (;words < numOfWords;words++)
    {
        dest[bytes] = (from[words] >> 8); ++bytes;
        dest[bytes] = (from[words] & 0xff); ++bytes;
    }
}

void dWordsToBytes(BYTE *dest, const DWORD *from, int numOfDWords)
{
    int dWords = 0, bytes = 0;
    for (;dWords < numOfDWords;dWords++)
    {
        dest[bytes] = (BYTE) (from[dWords] >> 24); ++bytes;
        dest[bytes] = (BYTE) (from[dWords] >> 16); ++bytes;
        dest[bytes] = (BYTE) (from[dWords] >> 8); ++bytes;
        dest[bytes] = (BYTE) (from[dWords] & 0xff); ++bytes;
    }
}

void floatsToBytes(BYTE *dest, const float *from, int numOfFloats)
{
    int floats = 0, bytes = 0;
    union
    {
        float Float;
        DWORD DWORD;
    } convert;

    for (;floats < numOfFloats;floats++)
    {
        convert.Float = from[floats];
        dest[bytes] = (BYTE) (convert.DWORD >> 24); ++bytes;
        dest[bytes] = (BYTE) (convert.DWORD >> 16); ++bytes;
        dest[bytes] = (BYTE) (convert.DWORD >> 8); ++bytes;
        dest[bytes] = (BYTE) (convert.DWORD & 0xff); ++bytes;
    }
}


/*! For internal use only. Send Rk512 telegram.
 \param *handle - descriptor returned by createConnection;
 \param dbnr = number of the DB;
 \param dbwnr = number of the first DBW; USE BYTE ADDRESSATION!!!
 \param sizeInBytes = amount of bytes;
 \param *data = array of data;
 \return "0" - success; "-1" error to send. >0 - error code from the partner.*/
int sendRk512(const THandle *handle, int dbnr, int dbwnr, int sizeInBytes, const BYTE *data)
{
    int telegramSize;
    int rest;
    bool firstTelegram = true;
    BYTE retTlg[4];
    BYTE telegram[128 + 10]; // data+header

    if (!sizeInBytes) return 0;

    if(sizeInBytes%2)
    {
        wLog(handle->errorMessage, "sendRk512: You've specified NOT EVEN amount of bytes: <%i>. Sorry - protocol restriction",sizeInBytes);
        return -1;
    }

    if (!handle)
    {
        wLog(handle->errorMessage, "sendRk512: Connection is not initialized");
        return -1;
    }

    rest = sizeInBytes;
    while (rest)
    {
#ifdef DEBUG
        printf("\n[rest = %i]", rest);
#endif //DEBUG

        if (firstTelegram)
        {
            if ((telegramSize = formTelegram ('D', true, false, dbnr, dbwnr,
                                              rest, data, telegram, handle->errorMessage)) == -1) return -1;
        }
        else
        {
            if ((telegramSize = formTelegram ('D', true, true, 0, 0,
                                              rest, data, telegram, handle->errorMessage)) == -1) return -1;
        }
        if (rest <= 128)
            rest = 0;
        else
        {
            rest -= 128;
            data += 128; //move the pointer to the next chunk of data
        }

        if (send3964(handle, telegramSize, telegram, 0) == -1) return -1;

        // read the answer telegram //////////////////////
        if (fetch3964(handle, retTlg, 4, 0) == -1) return -1;
        if (!rest || retTlg[3]) return retTlg[3];
        //----------------------------------------------------------------------
        firstTelegram = false;
    }
    return -1; //control never goes here
}

/*! For internal use only. Receive Rk512 telegram.
 \param *handle - descriptor returned by createConnection;
 \param type - data to be transmitted consists of (when sending only ’D’ is possible):
  			’D’ (44H) =data block; ’X’ (58H)= extended data block;
  			’E’ (45H) =input bytes; ’A’ (41H) = output bytes;
  			’M’ (4DH) =memory bytes; T’ (54H) = time cells;
  			’C’ (5AH) =counter cells;
 \param dbnr = number of the DB;
 \param dbwnr = number of the first DBW; USE BYTE ADDRESSATION!!!
 \param sizeInBytes = amount of bytes;
 \param *data = array of data;
 \return "0" - success; "-1" error to send. >0 - error code from the partner.*/
int fetchRk512(const THandle *handle, char type, int dbnr, int dbwnr, int sizeInBytes, BYTE *data)
{
    int telegramSize;
    int rest;
    bool firstTelegram = true;
    BYTE telegram[128 + 10]; // data + header

    if (!sizeInBytes) return 0;

    if(sizeInBytes%2 && (tolower(type) == 'd' || tolower(type) == 'x'))
    {
        wLog(handle->errorMessage, "fetchRk512: You've specified NOT EVEN amount of bytes: <%i>. Sorry - protocol restriction",sizeInBytes);
        return -1;
    }

    if (!handle)
    {
        wLog(handle->errorMessage, "fetchRk512: Connection is not initialized");
        return -1;
    }

    rest = sizeInBytes;

    while (sizeInBytes)
    {

#ifdef DEBUG
        printf("\n[rest = %i]", rest);
#endif //DEBUG

        if (firstTelegram)
        {
            if ((telegramSize = formTelegram (type, false, false, dbnr, dbwnr,
                                              rest, 0, telegram, handle->errorMessage)) == -1) return -1;
        }
        else
        {
            if ((telegramSize = formTelegram (type, false, true, 0, 0,
                                              rest, 0, telegram, handle->errorMessage)) == -1) return -1;
        }

		if (send3964(handle, telegramSize, telegram, 0) == -1) return -1;

        if (sizeInBytes <= 128)
        {
            rest = sizeInBytes;
            sizeInBytes = 0;
        }
        else
        {
            sizeInBytes -= 128;
            rest = 128;
        }

        if (fetch3964(handle, telegram, (rest*2) + 4, 0) == -1) return -1;
        if (sizeInBytes && telegram[3]) return telegram[3];
        else
        {
            if(toupper(type) != 'T' && toupper(type) != 'Z') memmove(data, telegram+4,rest);
            else memmove(data, telegram+4,rest*2);
        }

        if (sizeInBytes) data += rest;
        firstTelegram = false;
    }
    return 0;
}


/*! For internal use only. Assembles rk512 telegram;
 \param type - data to be transmitted consists of (when sending only ’D’ is possible):
  			’D’ (44H) =data block; ’X’ (58H)= extended data block;
  			’E’ (45H) =input bytes; ’A’ (41H) = output bytes;
  			’M’ (4DH) =memory bytes; T’ (54H) = time cells;
  			’C’ (5AH) =counter cells;
 \param isSend - "true" for send telegrams;
 \param isContinue - "true" for continuation telegrams;
 \param dbnr = number of DB; number of periferal byte; etc...;
 \param dbbnr = number of DW; number of periferal bit; etc...;
 \param sizeInBytes = amount of DW; amount of periferal bytes; etc...;
 \param *data = array of data. Used only first 128bytes for first
  			telegram, and 256 for every continuation;
 \param *mess = array for telegram. It have to be large enough to
  			accept output telegram with header, the best size is
  			260bytes (256 continuation telegram + 4bytes for
  			continuation header;
 \param *errorMessage - buffer for textual error message
 \return = the real size of the telegram with header.*/
int formTelegram (char type, bool isSend, bool isContinue, int dbnr, int dbbnr,
                  int sizeInBytes, const BYTE *data, BYTE *mess, char *errorMessage)
{
    // |1st send| |send| |data| |dbnr| |dwnr| |null| |dwnum| |CF | |CPU|
    BYTE stub[] =
        { 0x00, 0x00, 'A', 'D', 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF };
    int sizeOfHeader = 10;
    char dataType;

    if (isSend && !isContinue && type != toupper('D'))
    {
        wLog(errorMessage, "formTelegram: When sending only ’D’ is possible");
        return -1;
    }

    switch (toupper(type))
    {
        case 'D' : dataType = 'D'; break;
        //case 'X' : dataType = 'X'; break;
        case 'E' : dataType = 'E'; break;
        case 'A' : dataType = 'A'; break;
        case 'M' : dataType = 'M'; break;
        case 'T' : dataType = 'T'; break;
        case 'Z' : dataType = 'Z'; break;
        default: wLog(errorMessage, "formTelegram: The \"type\" must be one of \"DXEAMTC\": <%c>", type);
            return -1;
    }

    if (isSend && isContinue) //continue sending
    {
        sizeOfHeader = 4;
        mess[0] = 0xFF; //continuation
        mess[1] = 0x00; //00
        mess[2] = 0x41; //send
        mess[3] = dataType; //data type
    }

    if (!isSend && isContinue) //continue fetching
    {
        sizeOfHeader = 4;
        mess[0] = 0xFF; //continuation
        mess[1] = 0x00; //00
        mess[2] = 0x45; //send
        mess[3] = dataType; //data type
        return sizeOfHeader;
    }

    if (!isContinue)
    {
        sizeOfHeader = 10;
        memmove(mess, stub, sizeOfHeader);

        if(dataType == 'D' || dataType == 'X')
        {
            mess[6] = ((sizeInBytes/2) & 0xff00) >> 8;
            mess[7] = (sizeInBytes/2) & 0xff;
            dbbnr /= 2; //bytes to words
        }
        else
        {
            mess[6] = (sizeInBytes & 0xff00) >> 8;
            mess[7] = sizeInBytes & 0xff;
        }

        mess[3] = dataType;
        mess[4] = dbnr;
        mess[5] = dbbnr;

    }

    if (!isSend && !isContinue)
    {
        mess[2] = 'E';
        return sizeOfHeader;      	 // size of header
    }

    if (sizeInBytes > 128) sizeInBytes = 128;

    memmove(mess+sizeOfHeader,data,sizeInBytes);
    return sizeInBytes + sizeOfHeader;	/* header + data */
}

EXPORT int sendRk512Dbb(const THandle *handle, int dbnr, int dbbnr, int sizeInBytes, const BYTE *data)
{
    if(dbbnr%2)
    {
        wLog(handle->errorMessage, "sendRk512Dbb: You have to specify divisible by 2 beginnig address (dbbnr): <%i>. - Protocol restriction for DB's",dbbnr);
        return -1;
    }

    return sendRk512(handle, dbnr, dbbnr, sizeInBytes, data);
}

EXPORT int sendRk512Dbw(const THandle *handle, int dbnr, int dbwnr, int sizeInWords, const WORD *data)
{
    BYTE *tmp;

    if(dbwnr%2)
    {
        wLog(handle->errorMessage, "sendRk512Dbw: You have to specify divisible by 2 beginnig address (dbwnr): <%i>. - Protocol restriction for DB's",dbwnr);
        return -1;
    }

    tmp = (BYTE*) alloca(sizeInWords*2);
    if(!tmp)
    {
        wLog(handle->errorMessage, "sendRk512Dbw: Can't allocate the memory");
        return -1;
    }
    wordsToBytes(tmp, data, sizeInWords);
    return sendRk512(handle, dbnr, dbwnr, sizeInWords*2, tmp);
}

EXPORT int sendRk512Dbd(const THandle *handle, int dbnr, int dbdnr, int sizeInDWords, const DWORD *data)
{
    BYTE *tmp;

    if(dbdnr%2)
    {
        wLog(handle->errorMessage, "sendRk512Dbd: You have to specify divisible by 2 beginnig address (dbdnr): <%i>. - Protocol restriction for DB's",dbdnr);
        return -1;
    }

    tmp = (BYTE*) alloca(sizeInDWords*4);
    if(!tmp)
    {
        wLog(handle->errorMessage, "sendRk512Dbd: Can't allocate the memory");
        return -1;
    }
    dWordsToBytes(tmp, data, sizeInDWords);
    return sendRk512(handle, dbnr, dbdnr, sizeInDWords*4, tmp);
}

EXPORT int sendRk512DbReal(const THandle *handle, int dbnr, int dbdnr, int sizeInFloats, const float *data)
{
    BYTE *tmp;

    if(dbdnr%2)
    {
        wLog(handle->errorMessage, "sendRk512DbReal: You have to specify divisible by 2 beginnig address (dbdnr): <%i>. - Protocol restriction for DB's",dbdnr);
        return -1;
    }

    tmp = (BYTE*) alloca(sizeInFloats*4);
    if(!tmp)
    {
        wLog(handle->errorMessage, "sendRk512DbReal: Can't allocate the memory");
        return -1;
    }
    floatsToBytes(tmp, data, sizeInFloats);
    return sendRk512(handle, dbnr, dbdnr, sizeInFloats*4, tmp);
}

EXPORT int fetchRk512Dbb(const THandle *handle, int dbnr, int dbbnr, int sizeInBytes, BYTE *data)
{
    if(dbbnr%2)
    {
        wLog(handle->errorMessage, "fetchRk512Dbb: You have to specify divisible by 2 beginnig address (dbbnr): <%i>. - Protocol restriction for DB's",dbbnr);
        return -1;
    }
    return fetchRk512(handle, 'D', dbnr, dbbnr, sizeInBytes, data);
}

EXPORT int fetchRk512Dbw(const THandle *handle, int dbnr, int dbwnr, int sizeInWords, WORD *data)
{
    int retcode;
    BYTE *tmp;

    if(dbwnr%2)
    {
        wLog(handle->errorMessage, "fetchRk512Dbw: You have to specify divisible by 2 beginnig address (dbwnr): <%i>. - Protocol restriction for DB's",dbwnr);
        return -1;
    }

    tmp = (BYTE*) alloca(sizeInWords*2);
    if(!tmp)
    {
        wLog(handle->errorMessage, "fetchRk512Dbw: Can't allocate the memory");
        return -1;
    }

    retcode = fetchRk512(handle, 'D', dbnr, dbwnr, sizeInWords*2, tmp);
    if(retcode) return retcode;

    bytesToWords(data,tmp,sizeInWords*2, handle->errorMessage);
    return 0;
}

EXPORT int fetchRk512Dbd(const THandle *handle, int dbnr, int dbdnr, int sizeInDoubleWords, DWORD *data)
{
    int retcode;
    BYTE *tmp;

    if(dbdnr%2)
    {
        wLog(handle->errorMessage, "fetchRk512Dbd: You have to specify divisible by 2 beginnig address (dbdnr): <%i>. - Protocol restriction for DB's",dbdnr);
        return -1;
    }

    tmp = (BYTE*) alloca(sizeInDoubleWords*4);
    if(!tmp)
    {
        wLog(handle->errorMessage, "fetchRk512Dbd: Can't allocate the memory");
        return -1;
    }

    retcode = fetchRk512(handle, 'D', dbnr, dbdnr, sizeInDoubleWords*4, tmp);
    if(retcode) return retcode;

    bytesToDWords(data,tmp,sizeInDoubleWords*4, handle->errorMessage);
    return 0;
}

EXPORT int fetchRk512DbReal(const THandle *handle, int dbnr, int dbdnr, int sizeInFloats, float *data)
{
    int retcode;
    BYTE *tmp;

    if(dbdnr%2)
    {
        wLog(handle->errorMessage, "fetchRk512DbReal: You have to specify divisible by 2 beginnig address (dbdnr): <%i>. - Protocol restriction for DB's",dbdnr);
        return -1;
    }

    tmp = (BYTE*) alloca(sizeInFloats*4);
    if(!tmp)
    {
        wLog(handle->errorMessage, "fetchRk512Real: Can't allocate the memory");
        return -1;
    }

    retcode = fetchRk512(handle, 'D', dbnr, dbdnr, sizeInFloats*4, tmp);
    if(retcode) return retcode;

    bytesToFloats(data,tmp,sizeInFloats*4, handle->errorMessage);
    return 0;
}

EXPORT int fetchRk512Mb(const THandle *handle, int firstMb, int sizeInBytes, BYTE *data)
{
    return fetchRk512(handle, 'M', 0, firstMb, sizeInBytes, data);
}

EXPORT int fetchRk512Mw(const THandle *handle, int firstMw, int sizeInWords, WORD *data)
{
    int retcode;

    BYTE *tmp = (BYTE*) alloca(sizeInWords*2);
    if(!tmp)
    {
        wLog(handle->errorMessage, "fetchRk512Mw: Can't allocate the memory");
        return -1;
    }

    retcode = fetchRk512(handle, 'M', 0, firstMw, sizeInWords*2, tmp);
    if(retcode) return retcode;

    bytesToWords(data,tmp,sizeInWords*2, handle->errorMessage);
    return 0;
}

EXPORT int fetchRk512Md(const THandle *handle, int firstMd, int sizeInDWords, DWORD *data)
{
    int retcode;

    BYTE *tmp = (BYTE*) alloca(sizeInDWords*4);
    if(!tmp)
    {
        wLog(handle->errorMessage, "fetchRk512Md: Can't allocate the memory");
        return -1;
    }

    retcode = fetchRk512(handle, 'M', 0, firstMd, sizeInDWords*4, tmp);
    if(retcode) return retcode;

    bytesToDWords(data,tmp,sizeInDWords*4, handle->errorMessage);
    return 0;
}

EXPORT int fetchRk512MdReal(const THandle *handle, int firstMd, int sizeInDWords, float *data)
{
    int retcode;

    BYTE *tmp = (BYTE*) alloca(sizeInDWords*4);
    if(!tmp)
    {
        wLog(handle->errorMessage, "fetchRk512MdReal: Can't allocate the memory");
        return -1;
    }

    retcode = fetchRk512(handle, 'M', 0, firstMd, sizeInDWords*4, tmp);
    if(retcode) return retcode;

    bytesToFloats(data,tmp,sizeInDWords*4, handle->errorMessage);
    return 0;
}


EXPORT int fetchRk512Ib(const THandle *handle, int firstIb, int sizeInBytes, BYTE *data)
{
    return fetchRk512(handle, 'E', 0, firstIb, sizeInBytes, data);
}

EXPORT int fetchRk512Iw(const THandle *handle, int firstIw, int sizeInWords, WORD *data)
{
    int retcode;

    BYTE *tmp = (BYTE*) alloca(sizeInWords*2);
    if(!tmp)
    {
        wLog(handle->errorMessage, "fetchRk512Iw: Can't allocate the memory");
        return -1;
    }

    retcode = fetchRk512(handle, 'E', 0, firstIw, sizeInWords*2, tmp);
    if(retcode) return retcode;

    bytesToWords(data,tmp,sizeInWords*2, handle->errorMessage);
    return 0;
}

EXPORT int fetchRk512Id(const THandle *handle, int firstId, int sizeInDWords, DWORD *data)
{
    int retcode;

    BYTE *tmp = (BYTE*) alloca(sizeInDWords*4);
    if(!tmp)
    {
        wLog(handle->errorMessage, "fetchRk512Id: Can't allocate the memory");
        return -1;
    }

    retcode = fetchRk512(handle, 'E', 0, firstId, sizeInDWords*4, tmp);
    if(retcode) return retcode;

    bytesToDWords(data,tmp,sizeInDWords*4, handle->errorMessage);
    return 0;
}

EXPORT int fetchRk512IdReal(const THandle *handle, int firstId, int sizeInDWords, float *data)
{
    int retcode;

    BYTE *tmp = (BYTE*) alloca(sizeInDWords*4);
    if(!tmp)
    {
        wLog(handle->errorMessage, "fetchRk512IdReal: Can't allocate the memory");
        return -1;
    }

    retcode = fetchRk512(handle, 'E', 0, firstId, sizeInDWords*4, tmp);
    if(retcode) return retcode;

    bytesToFloats(data,tmp,sizeInDWords*4, handle->errorMessage);
    return 0;
}

EXPORT int fetchRk512Qb(const THandle *handle, int firstQb, int sizeInBytes, BYTE *data)
{
    return fetchRk512(handle, 'A', 0, firstQb, sizeInBytes, data);
}

EXPORT int fetchRk512Qw(const THandle *handle, int firstQw, int sizeInWords, WORD *data)
{
    int retcode;

    BYTE *tmp = (BYTE*) alloca(sizeInWords*2);
    if(!tmp)
    {
        wLog(handle->errorMessage, "fetchRk512Qw: Can't allocate the memory");
        return -1;
    }

    retcode = fetchRk512(handle, 'A', 0, firstQw, sizeInWords*2, tmp);
    if(retcode) return retcode;

    bytesToWords(data,tmp,sizeInWords*2, handle->errorMessage);
    return 0;
}

EXPORT int fetchRk512Qd(const THandle *handle, int firstQd, int sizeInDWords, DWORD *data)
{
    int retcode;

    BYTE *tmp = (BYTE*) alloca(sizeInDWords*4);
    if(!tmp)
    {
        wLog(handle->errorMessage, "fetchRk512Qd: Can't allocate the memory");
        return -1;
    }

    retcode = fetchRk512(handle, 'A', 0, firstQd, sizeInDWords*4, tmp);
    if(retcode) return retcode;

    bytesToDWords(data,tmp,sizeInDWords*4, handle->errorMessage);
    return 0;
}

EXPORT int fetchRk512QdReal(const THandle *handle, int firstQd, int sizeInDWords, float *data)
{
    int retcode;

    BYTE *tmp = (BYTE*) alloca(sizeInDWords*4);
    if(!tmp)
    {
        wLog(handle->errorMessage, "fetchRk512QdReal: Can't allocate the memory");
        return -1;
    }

    retcode = fetchRk512(handle, 'A', 0, firstQd, sizeInDWords*4, tmp);
    if(retcode) return retcode;

    bytesToFloats(data,tmp,sizeInDWords*4, handle->errorMessage);
    return 0;
}


EXPORT int fetchRk512Timers(const THandle *handle, int firstTimer, int numOfTimers, WORD *data)
{
    int retcode;
    BYTE *tmp = (BYTE*) alloca(numOfTimers*2);
    if(!tmp)
    {
        wLog(handle->errorMessage, "fetchRk512Timers: Can't allocate the memory");
        return -1;
    }
    retcode = fetchRk512(handle, 'T', 0, firstTimer, numOfTimers, tmp);
    if(retcode) return retcode;

    bytesToWords(data,tmp,numOfTimers*2, handle->errorMessage);
    return 0;
}

EXPORT int fetchRk512Counters(const THandle *handle, int firstCounter, int numOfCounters, WORD *data)
{
    int retcode;
    BYTE *tmp = (BYTE*) alloca(numOfCounters*2);
    if(!tmp)
    {
        wLog(handle->errorMessage, "fetchRk512Counters: Can't allocate the memory");
        return -1;
    }
    retcode = fetchRk512(handle, 'Z', 0, firstCounter, numOfCounters, tmp);
    if(retcode) return retcode;

    bytesToWords(data,tmp,numOfCounters*2, handle->errorMessage);
    return 0;
}

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

#include "r3964int.h"
#include "../util/util.h"
#include <stdio.h>
#include <errno.h>

EXPORT int send3964(const THandle *handle, int telegramSize, const BYTE *telegram, int W)
{
    int x = 0;		//setup attempt count
    int iterator = 0;
    BYTE symbol;// buffer for one symbol

#ifdef DEBUG
    printf("\n[send recursion N%i]",W);
#endif //DEBUG

	if(W >= handle->transmittingAttempts)
    {
        sendChar(handle, NAK);
        errno = 0;
        wLog(handle->errorMessage, "send3964: Can't handshake with partner, maximum transmitting attempts reached");
		return -1;
    }

    // handshake //////////////////////////////////////
    while(true)
    {
        if(x >= handle->setupAttempts)
        {
            sendChar(handle, NAK);
            errno = 0;
            wLog(handle->errorMessage, "send3964: Can't handshake with partner, maximum tries reached");
            //destroyConnection(handle);
            return -1;
        }
        //		if (sendChar(handle, NAK, handle->cdt) != 1) return -1;
        if (sendChar(handle, STX) != 1) return -1;

waitForDle:
        if(readChar(handle, &symbol, handle->adt) == 1)
        {
            if(symbol == DLE) break;
            if(symbol == STX) goto waitForDle;//yes, i know, it's ugly, but... "C'est la vie"
        }
        ++x;
    }
    //----------------------------------------------------------------------

    iterator = 0;
    for(iterator = 0;iterator < telegramSize;iterator++)
    {
        // send the telegram body ////////////////////////
        if (sendChar(handle, telegram[iterator]) != 1) return -1;
        if (readChar(handle, &symbol, 0) == -1) return -1;
        if (symbol == NAK) return send3964(handle, telegramSize, telegram, ++W); // next retry
        //duplicate codes equal to DLE
        if ( telegram[iterator] == DLE) if (sendChar(handle, telegram[iterator]) != 1) return -1;
        if (readChar(handle, &symbol, 0) == -1)return -1;
        if (symbol == NAK) return send3964(handle, telegramSize, telegram, ++W);// next retry
        //----------------------------------------------------------------------
    }

    // send DLE-ETX at the end of the telegram ///////
    if (sendChar(handle, DLE) != 1) return -1;
    symbol = 0;
    if (readChar(handle, &symbol, 0) == -1)return -1;
    else if (symbol == NAK) return send3964(handle, telegramSize, telegram, ++W);
    if (sendChar(handle, ETX) != 1) return -1;
    //----------------------------------------------------------------------


    if (handle->useBcc)
    {

        BYTE BCC = 0;
        for(iterator = 0;iterator < telegramSize;iterator++)
        {
            BCC ^= telegram[iterator];
            if(telegram[iterator] == DLE) BCC ^= telegram[iterator];//DLE duplication occured
        }
        BCC ^= DLE;
        BCC ^= ETX;

        if (sendChar(handle, BCC) != 1) return -1;
        //        if (readChar(handle, &symbol, 0) == -1) return -1;
        //        if (symbol == NAK) return send3964(handle, telegramSize, telegram, ++W); // next retry
    }

    // receive the acknowledgement (DLE) /////////////
    symbol = 0;
    if (readChar(handle, &symbol, handle->adt) < 0) return -1;
    if (symbol != DLE) return send3964(handle, telegramSize, telegram, ++W);

    //----------------------------------------------------------------------
    return telegramSize;
}

EXPORT int fetch3964(const THandle *handle, BYTE *telegram, int maxLen, int W)
{
    BYTE symbol;// buffer for one symbol
    int iterator = 0;
    int telegramSize = 0;
    bool nakNote = false;
    int retval = 0;

#ifdef DEBUG
    printf("\n[fetch recursion N%i]",W);
#endif //DEBUG

    if(W >= handle->transmittingAttempts)
    {
        sendChar(handle, NAK);
        return -1;
    }

    // waiting for STX ///////////////////////////////
    while(true)
    {
        symbol = 0;
        if (readChar(handle, &symbol, handle->tBlock) != 1) return -1;
        if (symbol == NAK) continue;
        if (symbol == STX)
        {
            if (sendChar(handle, DLE) != 1) return -1; break;
        }
        else
        {
            nakNote = true; break;
        }
    }

    while(true)
    {
        retval = readChar(handle, &symbol, handle->cdt);
        if (retval == -1) return -1;
        if (!retval)//timeout
        {
            if (sendChar(handle, NAK) != 1) return -1;
            return fetch3964(handle, telegram, maxLen, ++W);
        }

        if (symbol == DLE)
        {
            retval = readChar(handle, &symbol, handle->cdt);
            if (retval == -1) return -1;
            if (!retval)//timeout
            {
                if (sendChar(handle, NAK) != 1) return -1;
                return fetch3964(handle, telegram, maxLen, ++W);
            }
            if (symbol == DLE)
            {
                if(!nakNote)telegram[telegramSize++] = symbol; continue;
            }//duplicate DLE occured
            if (symbol == ETX)
            {
                --telegramSize;
                break;
            }//end of the telegram
            nakNote = true;//incorrect symbol
        }

        if(!nakNote && telegramSize > maxLen)
        {
            wLog(handle->errorMessage, "fetch3964: Too long input telegram (>%ibytes)",maxLen);
            return -1;
        }
        if(!nakNote)telegram[telegramSize++] = symbol;
    }

    if (handle->useBcc)
    {
        BYTE calculatedBcc = 0, receivedBcc = 0;

        retval = readChar(handle, &receivedBcc, handle->cdt);
        if (retval == -1) return -1;
        if (!retval)//timeout
        {
            if (sendChar(handle, NAK) != 1) return -1;
            return fetch3964(handle, telegram, maxLen, ++W);
        }
        for(iterator = 0;iterator <= telegramSize;iterator++)
        {
            calculatedBcc ^= telegram[iterator];
            if(telegram[iterator] == DLE) calculatedBcc ^= telegram[iterator];//DLE duplication occured
        }
        calculatedBcc ^= DLE;
        calculatedBcc ^= ETX;
        if(calculatedBcc != receivedBcc)
        {
            wLog(handle->errorMessage, "fetch3964: Block checksum failed");
            if (sendChar(handle, NAK) != 1) return -1;
            return fetch3964(handle, telegram, maxLen, ++W);
        }

        if(nakNote)
        {
            if (sendChar(handle, NAK) != 1) return -1;
            return fetch3964(handle, telegram, maxLen, ++W);
        }


        if (sendChar(handle, DLE) != 1) return -1;
        readChar(handle, &receivedBcc, handle->cdt);

    }
    return telegramSize;
}

int setTimings(THandle *handle, int cdt,int adt, int tBlock, int setupAttempts, int transmittingAttempts)
{

    if(cdt) //character delay time
    {
        if(cdt > 655350)
        {
            wLog(handle->errorMessage, "setTimings: Too high character delay time: <%i>", cdt);
            destroyConnection(handle);
            return -1;
        }

        if(cdt%10)
        {
            wLog(handle->errorMessage, "setTimings: Character delay time is not multiply of 10: <%i>", cdt);
            destroyConnection(handle);
            return -1;
        }

        if((handle->baudRate <= 300 && cdt < 60) || (handle->baudRate <= 600 && cdt < 40)
                || (handle->baudRate <= 1200 && cdt < 30) || cdt < 20)
        {
            wLog(handle->errorMessage, "setTimings: Too short character delay time for this speed: speed <%i> | cdt <%i>",handle->baudRate, cdt);
            destroyConnection(handle);
            return -1;
        }
        handle->cdt = cdt;//character delay time
    }
    else handle->cdt = 220;

    if(adt) //acknowledgment delay time
    {
        if(adt > 655350)
        {
            wLog(handle->errorMessage, "setTimings: Too high acknowledgment delay time: <%i>", adt);
            destroyConnection(handle);
            return -1;
        }

        if(adt%10)
        {
            wLog(handle->errorMessage, "setTimings: Acknowledgment delay time is not multiply of 10: <%i>", adt);
            destroyConnection(handle);
            return -1;
        }

        if((handle->baudRate <= 300 && adt < 60) || (handle->baudRate <= 600 && adt < 40)
                || (handle->baudRate <= 1200 && adt < 30) || adt < 20)
        {
            wLog(handle->errorMessage, "setTimings: Too short acknowledgment delay time for this speed: speed <%i> | adt <%i>",handle->baudRate, adt);
            destroyConnection(handle);
            return -1;
        }
        handle->adt = adt;
    }
    else
    {
        if(handle->useBcc) handle->adt = 2000; //character delay time
        else handle->adt = 550;
    }

    if(tBlock)//waiting while partner prepares the data
    {
        if(tBlock < 500)
        {
            wLog(handle->errorMessage, "setTimings: Too low Tblock time (you have specify at least 500ms): <%i>", tBlock);
            destroyConnection(handle);
            return -1;
        }
        handle->tBlock = tBlock;
    }
    else
    {
        switch(handle->baudRate)
        {
            case 110 : handle->tBlock = 20000; break;
            case 150 : handle->tBlock = 15000; break;
            case 300 : handle->tBlock = 10000; break;
            case 600 : handle->tBlock = 7000; break;
            default: handle->tBlock = 5000;
        }
    }

    if(setupAttempts)
    {
        if(setupAttempts > 255)
        {
            wLog(handle->errorMessage, "setTimings: Too many setup attempts: <%i>", setupAttempts);
            destroyConnection(handle);
            return -1;
        }
        handle->setupAttempts = setupAttempts;
    }
    else handle->setupAttempts = 6;

    if(transmittingAttempts)
    {
        if(transmittingAttempts > 255)
        {
            wLog(handle->errorMessage, "setTimings: Too many transmitting attempts: <%i>", transmittingAttempts);
            destroyConnection(handle);
            return -1;
        }
        handle->transmittingAttempts = transmittingAttempts;
    }
    else handle->transmittingAttempts = 6;
    return 0;
}

/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/***********************************************

	fs.h

   Structures and Defines for ACP-QIO Operation
   File System Writes.

***********************************************/

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <dataport/port_types.h>

#include "Params.hh"


#define kNetRdSz        8192
#define kNetBufSz       16384
#define kMsgIOBufSz     500000

#define k100mSec        100

#define k10mSec         10

#define kMsgIOBufSz     500000

#define kDBCnt          25
#define kBufSz          512
#define kOutBufSz       65536

#define kSerIOBufSz     1000000
 
#define kLineLength     3661
#define kImageSz        6725257

#define kEndSg          2
#define kTagSg          3
#define kHdrSg          9
#define kNavSg          0xB
#define kLinSg          0xC

#define kFileTimeOut    600	/* Set Time Out to ~1 Minute */

#define FOREVER         for(;;)

typedef struct 
{
   int  lastLineNum,
        lineNum,
        bytes,
        errCount,
        imagesDumped,
        nulledLines,
        shortLines,
        longLines,
        totalMins,
        imagesRcvD;
   char errArray[kDBCnt][kBufSz];
} ErrBlk;


/**********************
   Function Prototypes
**********************/

int           ReadBuf(char *program_name, ui08 *pBuf, int len);
int           open_net(char *ip_address, int iPort);
void          UpdateFile(void);

/************************
   External Declarations
************************/

extern Params _params;

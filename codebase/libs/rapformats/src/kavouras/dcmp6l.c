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
/*************************************************************************************
 * DCMP6L.C : Kavouras Supplied routines to decode "low resolution images"
 *
 *  Commented and Modified for general use by F. Hage May 1996. NCAR/RAP
 */
 
#include <stdio.h>
#include <memory.h>
#include <toolsa/os_config.h>
#include <rapformats/kavouras_io.h>

#define NUM_6L_COLS   320
#define NUM_6L_ROWS   240
#define IMAGE_SIZE    (NUM_6L_COLS * NUM_6L_ROWS)
 
#define    SHIFT_BITS      6
#define    FRAME_SEP      33
#define    TRUNCATE      35
#define    LNG_EXTEND    36
#define    COLOR_BITS      63
#define    MAX_X           639
#define    MAXREAD          512

static unsigned char color,Pcolor;  /* color, Previous color */
static unsigned char buffer[MAXREAD];
static unsigned char image[IMAGE_SIZE];

static short    x, y;     /* Current x,y position in image */
static short   Px, Py;   /* Previous x,y position */

static int RdBufLo( FILE *f, int *index);
static void OutPixLo(int  count);

/*************************************************************************
 *  Dcmp6L():
 *
 *  Read a Kavouras Low resolution image file and decompress it into
 *  a static buffer. Returns a pointer to the static buffer.
 *  The image buffer is overwritten each call, so copy data out
 *  if you need to retain data across calls.
 *
 *  Returns NULL on error, pointer to the image data on success
 *
 */

unsigned char *Dcmp6L(FILE   *f)
{
    short    count;
    unsigned char    cmd;
    int        numread,
            index;

    if ((fseek(f,0,0)) >= 0) {   /* Seek to the beginning of the file */
        numread = RdBufLo(f,&index);
        x = 0;
        y = 0;
        Px = x;
        Py = y;
        while (index < numread) {
            cmd = buffer[index++];
            if (index == numread) numread = RdBufLo(f,&index);
            color = cmd & COLOR_BITS;
            count = cmd >> SHIFT_BITS;
            if (count) {
                OutPixLo(count);
            } else {
                color = Pcolor;
                switch (cmd) {
                    case    LNG_EXTEND:
                           count = buffer[index++];
                           if (index == numread) numread = RdBufLo(f,&index);
                           OutPixLo(count);
                    break;

                    case    TRUNCATE:
                              if (x) {
                                 count = MAX_X - x;
                                 OutPixLo(count);
                              }
                    break;
 
                    case    FRAME_SEP:
                              count = MAX_X - x;
                              OutPixLo(count);
                              index = numread;
                    break;

                    default: count = cmd;
                             color = Pcolor;
                             OutPixLo(count);
                     break;
                }
            }
        }
        return(image);
    }
    return(NULL);
}

/*************************************************************************
 *
 */

int RdBufLo( FILE    *f, int    *index)
{
    *index = 0;
    return(fread(buffer,sizeof(char),MAXREAD,f));
}

/*************************************************************************
 *
 */
 
void OutPixLo(int    count)
{
    unsigned char *ptr;

#ifdef ORIGINAL_KAVOURAS
    x += 2 * count;
    kgbline(Px,Py,x,y,color);
    kgbline(Px,Py+1,x,y+1,color);
#else 
    /* compute the proper location in the image */
    ptr = image + (y * NUM_6L_COLS) + x;
    memset(ptr,color,count);                     /* Set pixel values in the image */
    x += count;
#endif

    if (x >= MAX_X) {
        x = 0;
        y++;
    }
    Pcolor = color;
    Px = x;
    Py = y;
}

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
 * DCMP6H.C : Kavouras Supplied routines to decode "high resolution images"
 *
 *  Commented and Modified for general use by F. Hage Feb 1995. NCAR/RAP
 */

#include <stdio.h>
#include <memory.h>
#include <toolsa/os_config.h>
#include <rapformats/kavouras_io.h>

#define    NUM_6H_COLS  640
#define    SHIFT_BITS   6
#define    FRAME_SEP    33
#define    FIELD_SEP    34
#define    TRUNCATE     35
#define    LNG_EXTEND   36
#define    COLOR_BITS   63
#define    MAX_X        639
#define    MAXREAD      512
#define    IMAGE_SIZE   (640 * 480)

 
static short   x, y;    /* Current x,y position in image */
static short   Px,Py;   /* Previous x,y position */

static int     linecount;

static unsigned char    color, Pcolor;   /* color, Previous color */
static unsigned char    buffer[MAXREAD];
static unsigned char    image[IMAGE_SIZE];

static int RdBufHi( FILE *f, int *index);
static void OutPixHi(int    count, char    endofline);

/*************************************************************************
 *  Dcmp6H():
 *
 *  Read a Kavouras High resolution image file and decompress it into
 *  a static buffer. Returns a pointer to the static buffer.
 *  The image buffer is overwritten each call, so copy data out
 *  if you need to retain data across calls.
 * 
 *  Returns NULL on error, pointer to the image data on success
 *
 */
unsigned char *Dcmp6H(FILE   *f)
{
    int     numread,index;
    short    count;
    unsigned char    cmd;

    if ((fseek(f,0,0)) >= 0) {  /* Seek to the beginning of the file */
        numread = RdBufHi(f,&index);
        linecount = 0;
        x = 0;
        y = 0;
        Px = x;
        Py = y;
        while (index < numread) {
            cmd = buffer[index++];
            if (index == numread) numread = RdBufHi(f,&index);
            color = cmd & COLOR_BITS;
            count = cmd >> SHIFT_BITS;
            linecount += count;

            if (count) {
		OutPixHi(count,0);
            } else {
                color = Pcolor;
                switch (cmd) {
                    case    LNG_EXTEND:
		        count = buffer[index++];
                        linecount += count;
                        if (index == numread) numread = RdBufHi(f,&index);
                        if (linecount >= MAX_X) {
                            cmd = buffer[index++];
                            if (index == numread) numread = RdBufHi(f,&index);
                            OutPixHi(count,1);
                        } else {
			    OutPixHi(count,0);
			}
                    break;

                    case    TRUNCATE:
		        if (x) {
                            count = MAX_X - x;
                            OutPixHi(count,0);
                        }
                    break;

                    case    FIELD_SEP: 
		        count = MAX_X - x;
                        OutPixHi(count,0);
                        y = 1;
                        Py = y;
                    break;
		     
                    case    FRAME_SEP:
		        count = MAX_X - x;
                        OutPixHi(count,0);
                        index = numread;
                    break;
		     
                    default:
		        count = cmd;
                        linecount += count;
                        color = Pcolor;
                        if (index == numread) numread = RdBufHi(f,&index);
                        if (linecount >= MAX_X) {
                            cmd = buffer[index++];
                            if (index == numread) numread = RdBufHi(f,&index);
                            OutPixHi(count,1);
                        } else {
                            OutPixHi(count,0);
			}
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
int RdBufHi( FILE *f, int *index)
{
    *index = 0;
    return(fread(buffer,sizeof(char),MAXREAD,f));
}

/*************************************************************************
 *
 */

void OutPixHi(int    count, char    endofline)
{
    unsigned char *ptr;


#ifdef ORIGINAL_KAVOURAS
     x += count;
     kgbline(Px,Py,x,y,color);   /* DRAW a line from Px,Py to x,y ? */
#else
    /* compute the proper location in the image */
    ptr = image + (y * NUM_6H_COLS) + x;
    memset(ptr,color,count);                     /* Set pixel values in the image */
    x += count;
#endif
     
    if ((x >= MAX_X) || (endofline)) {
        x = 0;
        y += 2;
        linecount = 0;
    }
    Pcolor = color;
    Px = x;
    Py = y;
}

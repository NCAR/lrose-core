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
#ifdef __cplusplus                                                        
 extern "C" {                                                         
#endif
/******************************************************************
	Image files header structure

	File: image.h

*****************************************************************/


/* the image header */
struct image_header {
    char name[8];		/* a name identifying the image class */
    unsigned short dim;		/* dimension of the image (1-3) */
    unsigned short xz;		/* size in dim 1 */
    unsigned short yz;		/* size in dim 2 */
    unsigned short zz;		/* size in dim 3 */

    unsigned short depth;	/* pixel depth (field width) in bits */
    /* 1,8,16,32 */
    unsigned short nf;		/* number of fields */
    unsigned short fl;		/* float (1) or int (0) pixel */
    unsigned short count;	/* a counter for identifying processed
				   version */
    unsigned long size;		/* data size (excluding the header) in 
				   bytes */
    unsigned long cmp_code;	/* compresion code. 0 - raster */
    char ctname[12];		/* color table name, hint */
    unsigned long o_size;	/* overlay size, In long boundary */
    char note[144];		/* e.g. time, auther, image name ... */

};				/* 196 bytes */

typedef struct image_header Image_header;

/* the overlay data struct is an array of short integers */
/*         [0]: Data length in bytes 
   [1]: Number of parts (multi-polylines or char strings).
   [2-3]: Drawing mask
   [4-]: offset of the i-th part
   For each part starting from the offset:
   char str: first bit=1, line type, color,
   location: y, x
   the string
   lines:    first bit=0, line type, color,
   (y,x), (y,x), ....

   The maximum data length is limited to 32k (or 64k if
   [0],[4-], is casted to unsigned). However this simplifies
   the data structure.
 */


struct sh_color_table {		/* we assume 8 bit frame buffer */
    char name[56];
    int cmap;			/* not used */
    int window;			/* not used */
    int user_cnt;		/* 0: no user, to be removed; -1: to be
				   created; -2: removed; -3: creation error; 
				   -4: Other errors; */
    unsigned int ovplane;
    /* overlay plane available (bits set to 1) */
    int length;
    unsigned int pixel[256];
};

typedef struct sh_color_table Col_table;

struct commu_area {
    int key_ctmng;
    int pid_ctmng;
    int res[6];
};

typedef struct commu_area Com;


/* a shared memory area is created to hold:
   N_TBL color tables;
   N_LOC image headers;
   and an area for communication;
   The mem size is C_SIZE */

#define IMG_LOC 7800
#define N_LOC   15
#define C_TBL   7817
#define N_TBL   12
#define C_SIZE  16384
#define MAX_IMAGE_SIZE 1000*1024

#define KEY_CTMNG 12345789
#ifdef __cplusplus             
}
#endif

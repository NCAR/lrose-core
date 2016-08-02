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
/* font.h */

#ifndef FONT_WAS_INCLUDED
#define FONT_WAS_INCLUDED

#include <dataport/port_types.h>

/* The number of standard Ascii characters. */
#define NUM_ASCII 95

#define MAX_FONTNAME 22

/* Structure for each stroke of a character. */
typedef struct
{
  si16 	pen;    /* pen flag: 0->up  1->down */
  si16 	x;	/* x position */
  si16 	y;	/* y position */
} FONTstroke;

typedef struct
{
  si16        ascii_code;
  si16        xnext;
  si16        ynext;
  si16        n_strokes;
  FONTstroke  strokes[1];   /* There are n_strokes of these */
} FONTchar;

typedef struct FONTstruct_
{
  char	   name[MAX_FONTNAME];
  si32     index[NUM_ASCII];
  int      descender;
  int      height;
  int      num_chars;
  FONTchar *chars[NUM_ASCII];     /* There will be num_chars of these used */
} FONTstruct;


extern FONTstruct *FONTinit ( char *name );
   /* name is the name of a binary font file (*.binf)
      Return NULL on failure */

extern void FONTfree ( FONTstruct *fs );
   /* free the memory associated with this font */

extern int FONTextent( FONTstruct *fs, char *text, int *penups, int *xwidth);
   /* return the number of strokes for this text;
       	int *penups;	number of penups in the text
	int *xwidth;	width of text (arb. units)	
    */

extern int FONTtext( FONTstruct *fs, char *text, FONTstroke **stroka,
	int *stroka_len,  int *height, int *width);
   /* Translate the given text into an array of FONTstrokes.
      Input : fs, text.
      Return: stroka: a pointer to a static array of strokes,
		stroka_len : # elements in stroka
		height : height of text
		width : width of text

      The units of FONTstroke positions are arbitrary; to scale use the height
      and width values.
    */

extern void FONTscale( FONTstroke *stroka, int stroka_len, int height, int width,
			 int scaled_height, int scaled_width, int preserve_aspect );
   /* Scale the stroked text array to fit into a box scaled_height x scaled_width.
      stroka, stroke_len, height, width are output from FONTtext.  
      If preserve_aspect = TRUE, preserve the aspect ratio of the text, in which case
      the box becomes a limit which the scaled text will not exceed.
      If preserve_aspect = FALSE, the text will fit exactly in the box.
      Call this before you call FONToffset.
    */

extern void FONToffset( FONTstroke *stroka, int stroka_len, int offset_x, 
			  int offset_y);
   /* Add a constant offset to a stroked text array. 
      stroka, stroke_len are output from FONTtext.
      Call this after you call FONTscale.
    */

#endif
#ifdef __cplusplus
}
#endif

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
/* font_internal.h */

#include <dataport/port_types.h>
#include <toolsa/font.h>

/* Typedef for long version:  pen, x, y, fields of each character stroke.
 * Note that we use the long version for a character when:
 *  1)  the number of strokes in the character is greater than 127 or
 *  2)  at least one of the x or y values for any of the character's
 *      strokes is outside byte range (-127, 127).
 */

/* Structure for the header of a font. */
#define FNT_FIL_HDR_SIZ 8
typedef struct  {
	char	check[FNT_FIL_HDR_SIZ] ; /* Contains Preview font
					  * ID string (FNT_FIL_HDR).
					  */
	si16	font_code ;	/* The IGES font code */
	si16	name_size ;	/* The size of the font name */
	si16	sf ;		/* superseeded font code */
	si16	scale ;		/* IGES text scale (char height) */
	si16	num_chr ;	/* Number of characters in this font */
	} Font_header ;

/* Structure for header of each character.
 * Note:  
 *   We use short form if "asc_cod" > 0, and long form if "asc_cod" < 0,
 *   in which case the ascii code for the character is the absolute
 *   value of "asc_cod".
 */
/* Standard (short) form: */
typedef struct  {
	si16	asc_cod ; /* The ascii code for the char. */
	si08 	xnext ;   /* x offset to next char origin. */
	si08 	ynext ;   /* y offset to next char origin. */
	si08 	strokes ; /* Number of strokes in char. */
	} Char_header ;

/* Long form: */
typedef struct  {
	si16	asc_cod ; /* The negative ascii code for the char. */
	si16 	xnext ;   /* x offset to next char origin. */
	si16 	ynext ;   /* y offset to next char origin. */
	si16 	strokes ; /* Number of strokes in char. */
	} Char_headerl ;

/* Structure for each stroke of a character. */
typedef struct  {
	si08		pen ;	/* pen flag: 0->up  1->down */
	si08 	x ;	/* x position */
	si08 	y ;	/* y position */
	} Stroke ;

/* Structure for each stroke of a character in long format. */
typedef struct  {
	si16 	pen ; /* pen flag: 0->up  1->down */
	si16 	x ;	/* x position */
	si16 	y ;	/* y position */
	} Strokel ;


/* the structure of a binf file :

   igs_fnt_header	header;
   char			font_name[ header.name_size];
   long			font_index[ NUM_ASCII];

   now, depending if asc_cod > 0 or not:

   Char_header		chead;
   Stroke		stk[ chead.strokes];		

   (or)

   Char_headerl		cheadl;
   Strokel		stkl[ cheadl.strokes];		

 */
#ifdef __cplusplus
}
#endif

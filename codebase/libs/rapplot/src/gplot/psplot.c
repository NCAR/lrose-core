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
/*********************************************************************
 * psutil.c
 *
 * Postscript graphics utility routines - used by gutil.c routines
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * December 1991
 *
 *********************************************************************/

#include <rapplot/gplot.h>

/**************************************************************************
 * PsGrestore() : performs a grestore in postscript
 *
 *************************************************************************/

void PsGrestore(FILE *file)
{

  fprintf(file, "grestore\n");

}
/**************************************************************************
 * PsGsave() : performs a gsave in postscript
 *
 *************************************************************************/

void PsGsave(FILE *file)
{

  fprintf(file, "gsave\n");

}
/**************************************************************************
 * PsRotate() : rotate the coord system
 *
 *************************************************************************/

void PsRotate(FILE *file, double angle)
{

  fprintf(file, "%f rotate\n", angle);
    
}

/**************************************************************************
 * PsScale() : scale the page
 *
 *************************************************************************/

void PsScale(FILE *file, double xscale, double yscale)
{

  fprintf(file, "%g %g scale\n", xscale, yscale);

}

/**************************************************************************
 * PsSetFont() : sets up font on a postscript printer
 *
 *************************************************************************/

void PsSetFont(FILE *file, const char *fontname, double fontsize)
{

  fprintf(file, "/%s findfont %g scalefont setfont\n",
	  fontname, fontsize);

}
/**************************************************************************
 * PsSetGray() : sets up gray on a postscript printer
 *
 *************************************************************************/

void PsSetGray(FILE *file, double graylevel)
{

  fprintf(file, "%.2f setgray\n", graylevel);

}
/**************************************************************************
 * PsSetLineDash()
 *
 * set the line dash style in postscript - the dash and space lengths
 * are in whatever user unit is set
 *
 *************************************************************************/

void PsSetLineDash(FILE *file, double dash_length, double space_length)
{

  fprintf(file, "[%g %g] 0 setdash\n",
	  dash_length, space_length);

}

/**************************************************************************
 * PsSetLineStyle() : set the line stype in postscript
 *
 *************************************************************************/

void PsSetLineStyle(FILE *file, const psgc_t *psgc)
{

  PsSetLineWidth(file, psgc->line_width);

  PsSetLineDash(file,
		psgc->dash_length, psgc->space_length);

}

/**************************************************************************
 * PsSetLineWidth() : set the line width in postscript
 *
 *************************************************************************/

void PsSetLineWidth(FILE *file, double line_width)
{

  fprintf(file, "%g setlinewidth\n", line_width);

}

/**************************************************************************
 * PsShowPage() : send the show page message
 *
 *************************************************************************/

void PsShowPage(FILE *file)
{

  fprintf(file, "showpage\n");
    
}

/**************************************************************************
 * PsTranslate() : translate the coords in device coords
 *
 *************************************************************************/

void PsTranslate(FILE *file, double wx, double wy)
{

  fprintf(file, "%g %g translate\n", wx, wy);
    
}


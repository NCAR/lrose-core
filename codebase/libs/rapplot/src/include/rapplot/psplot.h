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

/******************************************************************
 * psplot.h: header file for postscript utility routines
 ******************************************************************/

#ifndef psplot_h
#define psplot_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

  /*
   * structure for world co-ordinate windows - PSREF;
   */

  typedef struct {
    int xmin, ymin;
    unsigned int width, height;
    double xscale, yscale;
  } psref_t;

  /*
   * structure for PostScript Graphics Context
   */

  typedef struct {
    FILE *file;
    int current_x, current_y;
    char *fontname;
    double fontsize;
    double line_width;
    double dash_length;
    double space_length;
    double graylevel;
    char hexstring[4];
  } psgc_t;

  /*
   * struct for point in postscript - as in XPoint, GPoint
   */

  typedef struct {
    int x, y;
  } PsPoint;

  /*
   * function prototypes
   */

  extern void PsGrestore(FILE *file);

  extern void PsGsave(FILE *file);

  extern void PsPageSetup();

  extern void PsReadColors();

  extern void PsRotate(FILE *file,
		       double angle);

  extern void PsScale(FILE *file,
		      double xscale,
		      double yscale);

  extern void PsSetFont(FILE *file,
			const char *fontname,
			double fontsize);

  extern void PsSetGray(FILE *file,
			double graylevel);

  extern void PsSetLineDash(FILE *file,
			    double dash_length,
			    double space_length);

  extern void PsSetLineStyle(FILE *file,
			     psgc_t *psgc);

  extern void PsSetLineWidth(FILE *file,
			     double line_width);

  extern void PsShowPage(FILE *file);

  extern void PsTranslate(FILE *file,
			  double wx,
			  double wy);


#ifdef __cplusplus
}
#endif

#endif

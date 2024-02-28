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
/**********************************************************************
 * CIDD_COLORSCALES.H:  Data structure defns for CIDD
 */

#ifndef CIDD_COLORSCALES_H
#define CIDD_COLORSCALES_H

typedef struct {   /* data value to color mappings */
  double min,max;  /* data Range to map onto color */
  long pixval;     /* X value to use to draw in this color */
  GC gc;           /* A GC for this color */
  char cname[NAME_LENGTH];   /* color name    */
  char label[LABEL_LENGTH];  /* label to use (use min value if empty) */
} Val_color_t;

typedef struct {
  int nentries;
  long val_pix[MAX_COLOR_CELLS]; /* Pixel values to draw in */
  GC val_gc[MAX_COLOR_CELLS];    /* GCs for each value */
  Val_color_t *vc[MAX_COLOR_CELLS];
} Valcolormap_t;
 
typedef struct {
  char name[NAME_LENGTH];
  long pixval;
  GC gc;
  unsigned short r,g,b; /* Full intensity color values for this color */
} Color_gc_t;

typedef struct { 
  unsigned long handle; /* handle to Xview object */
  Color_gc_t *cgcp;     /* Color Cell info */
  long cval; /* pointer to color value data */
  int c_num; /* color cell number in our allocated arrary */
} Color_change_t;

#endif

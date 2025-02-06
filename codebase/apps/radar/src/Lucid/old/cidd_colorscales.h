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

#include <QBrush>

class Val_color_t {   /* data value to color mappings */
public:
  Val_color_t() {
    min = 1.0e99;
    max = -1.0e99;
    pixval = 0;
    cname[0] = '\0';
    label[0] = '\0';
  }
  double min,max;  /* data Range to map onto color */
  long pixval;     /* X value to use to draw in this color */
  QBrush brush;    /* A brush for this color */
  char cname[NAME_LENGTH];   /* color name    */
  char label[LABEL_LENGTH];  /* label to use (use min value if empty) */
};

class Valcolormap_t {
public:
  Valcolormap_t() {
    nentries = 0;
    memset(val_pix, 0, sizeof(val_pix));
    memset(vc, 0, sizeof(vc));
  }
  int nentries;
  long val_pix[MAX_COLOR_CELLS]; /* Pixel values to draw in */
  QBrush brush[MAX_COLOR_CELLS]; /* brush for each value */
  Val_color_t *vc[MAX_COLOR_CELLS];
};
 
class Color_gc_t {
public:
  Color_gc_t() {
    name[0] = '\0';
    pixval = 0;
    r = g = b = 0;
  }
  char name[NAME_LENGTH];
  long pixval;
  QBrush brush;
  unsigned short r,g,b; /* Full intensity color values for this color */
};

class Color_change_t {
public:
  Color_change_t() {
    cgcp = NULL;
    cval = 0;
    c_num = 0;
  }
  // unsigned long handle; /* handle to Xview object */
  Color_gc_t *cgcp;     /* Color Cell info */
  long cval; /* pointer to color value data */
  int c_num; /* color cell number in our allocated arrary */
};

#endif

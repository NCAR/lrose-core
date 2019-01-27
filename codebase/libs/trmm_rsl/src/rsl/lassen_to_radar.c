/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997
            John H. Merritt
            Space Applications Corporation
            Vienna, Virginia

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


19 Jan 1998
   Michael Whimpey from BMRC, Australia changed code, for the
   different callibrations, between, Pre_mctex, mctex, Gunn_Pt
   periods.
22 Apr 1999
   Michael Whimpey added more callibration periods over berrimah and scsmex.
   please see code where there is m.whimpey 

*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define USE_RSL_VARS
#include <trmm_rsl/rsl.h>

Radar *RSL_lassen_to_radar(char *infile)
{
  fprintf(stderr, "LASSEN is not installed in this version of RSL.\n");
  return NULL;
}

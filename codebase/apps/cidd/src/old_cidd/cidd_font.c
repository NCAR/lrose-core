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
/********************************************************************
 * CIDD_FONT.C Routines that deal with the fonts in the RD program
 *
 *
 * For the Cartesian Radar Display (CIDD)
 * Frank Hage   July 1991 NCAR, Research Applications Program
 */

#include <X11/Xlib.h>

#define RD_FONT 1
#include "cidd.h"

#include <rapplot/xrs.h>

/*****************************************************************
 * LOAD_FONTS:  Get standard set of fonts and font metrics to use
 */

load_fonts(dpy)
    Display *dpy;
{
    int i;
    char    p_name[256];
    char    *f_name;

    gd.num_fonts = XRSgetLong(gd.cidd_db, "cidd.num_fonts", 1);

    if(gd.num_fonts > MAX_FONTS) {
	gd.num_fonts = MAX_FONTS;
	fprintf(stderr,"Cidd: Warning. Too Many Fonts. Limited to %d Fonts\n",MAX_FONTS);
    }

    for(i=0;i < gd.num_fonts; i++) {
        sprintf(p_name,"cidd.font%d",i+1);
        f_name = XRSgetString(gd.cidd_db,
                              p_name, "fixed");
        gd.fontst[i] = (XFontStruct *) XLoadQueryFont(dpy,f_name);
        if(gd.fontst[i] != NULL) {
            gd.ciddfont[i]  = gd.fontst[i]->fid;
        } else {
            fprintf(stderr,"Can't load font: %s\n",f_name);
            fprintf(stderr,"Using 'fixed' instead\n");
            gd.fontst[i]  = (XFontStruct *) XLoadQueryFont(dpy,"fixed");
            gd.ciddfont[i]  = gd.fontst[i]->fid;
        }
    }    

}

/********************************************************************
 * CHOOSE_FONT: Choose the largest font that fits inside bounding
 *      box. Returns offsets to center string
 */
 
Font
choose_font(string,x_size,y_size,xmid,ymid)
    char *string;
    int x_size,y_size;
    int    *xmid,*ymid;    /* RETURNS number of pixels from origin to Mid point of string */
{
    int i;
    int len,direct,ascent,descent;
    XCharStruct overall;
 
    len = strlen(string);
    for(i = gd.num_fonts -1; i >= 0; i-- ) {
        XTextExtents(gd.fontst[i],string,len,&direct,&ascent,&descent,&overall);                                 
        if(overall.width < x_size && (overall.ascent + overall.descent) < y_size) {
            *xmid = -(overall.width / 2);
            *ymid = overall.ascent - ((overall.ascent + overall.descent) /2);
            return gd.ciddfont[i];  
        }
    }
    *xmid = -(overall.width / 2);
    *ymid = overall.ascent - ((overall.ascent + overall.descent) /2);
    return gd.ciddfont[0];  /* go with smallest as last resort */
}

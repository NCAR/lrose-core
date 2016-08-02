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
/****************************************************************************
 * CIDD_CBAR.C : Routines used to draw the colorbars for CIDD
 *
 * F. Hage    Jan 1991    NCAR - RAP
 */

#define CIDD_CBAR

#include "cidd.h"

/************************************************************************
 * DRAW_COLORBAR : Draw and Label a color bar : Orient ==0 - Vert,
 * == 1, Horiz. 
 */

void
draw_colorbar(Display *dpy, Drawable xid, GC gc,  int x1,  int y1,  int width,  int height,
	      int num_entries,  Val_color_t **vc,  int orient,  char *units)
{
    int 	i,j,len;
    int		decimate;
    int 	xmid, ymid;
    int 	bar_wd;
    int 	bar_xstart;
    int 	bar_ystart;
    int		bar_yend;
    int 	bar_xend;
    int		y_pos, x_pos;
    int	     	bar_width;
    int	     	bar_height;
    int         max_label;
	int         ival;
    long        white_cell;
    long        black_cell;
    char    	label[LABEL_LENGTH];
    Font    	font;
    

    if(height < 10 || width < 5) return;
    
    switch(orient) {
    case 0:  /* Vertical orientation */
	bar_height = (int)(((double) height / ((double) num_entries +2.5)) + 0.5);
	bar_width = width - 2;
    break;
    case 1:  /* HorizontaL orientation */
	bar_width = (int)(((double) width / ((double) num_entries +2.5)) + 0.5);
	bar_height = height - 2 ;
    break;
    }
	

    /* Select a font for everything */
    max_label = 0;
    for(i=0; i < num_entries; i++ ) {    /* Find the widest label string */
	if((len = strlen(vc[i]->label)) > 0) {
	    if(len > max_label) max_label = len;
	} else {
	    sprintf(label,"%.3g",vc[i]->min);
	    if(strlen(label) > max_label) max_label = strlen(label);
	}
    }
    for(i=0; i < max_label; i++) {  /* Set a dummy label to all W's */
	label[i] = 'W';  /* a wide character */
    }
    label[i] = '\0';
    font = choose_font(label, bar_width, bar_height, &xmid, &ymid);
    XSetFont(gd.dpy, gc, font);

    decimate = num_entries/ 32; /* don't put more than 32 labels on */
    
  switch(orient) {
  case 0:  /* Vertical orientation */
        decimate = num_entries/ 32; /* don't put more than 32 labels on */
	if(decimate < 1) decimate = 1;

	bar_xstart = x1 + 1 ;
	bar_ystart = y1 + height - bar_height;
	 
	for(i=0; i < num_entries; i++ ) {
	    bar_yend = bar_ystart - bar_height;
	    
	    /* fill in color rectangle */
	    XSetForeground(gd.dpy,gd.def_gc,vc[i]->pixval);
	    XFillRectangle(gd.dpy,xid,gd.def_gc,bar_xstart,bar_yend,bar_width,bar_height);
	    /* Decimate the number of labels */
	    if(decimate == 1 || i % decimate == 0) {
	     
	      if ((len = strlen(vc[i]->label)) > 0) {
		STRcopy(label,vc[i]->label,LABEL_LENGTH);
		
		x_pos = bar_xstart + (bar_width/2) + xmid;
		y_pos = bar_yend  + (bar_height/2) + ymid;
		XDrawImageString(gd.dpy,xid,gc, x_pos, y_pos, label, len);

	      } else {	    /* numeric label */
		
		if(fabs(vc[i]->max - vc[i]->min) == 1.0) {	/* If range == 1; center label */
		    y_pos =  (bar_yend  + (bar_height/2) + ymid);
		    sprintf(label,"%g",vc[i]->min);
		} else {		/* Place label at bottom of bar */
		    y_pos = bar_ystart - 1;
			ival = vc[i]->min;   /* convert to an integer */
		    if(ival == vc[i]->min) {
				sprintf(label,"%d",ival);
			} else {
				sprintf(label,"%.3g",vc[i]->min);
			}
		}
		
		XDrawImageString(gd.dpy,xid,gc,
	  	    (bar_xstart + (bar_width/2) + xmid), y_pos, label, strlen(label));
	      }
	    }
	    bar_ystart = bar_yend;
	} 

	/* place a max val at the top of the scale if appropriate */
	if(strlen(vc[num_entries-1]->label) == 0 && (fabs(vc[num_entries-1]->max - vc[num_entries-1]->min) != 1.0)) {
		y_pos = bar_ystart - 1;
		ival = vc[num_entries-1]->max;   /* convert to an integer */
	    if(ival == vc[num_entries-1]->max) {
			sprintf(label,"%d",ival);
		} else {
			sprintf(label,"%.3g",vc[num_entries-1]->max);
		}
		XDrawImageString(gd.dpy,xid,gc, (bar_xstart + (bar_width/2) + xmid), y_pos, label, strlen(label));
	}
	
	font = choose_font(units, width, bar_height, &xmid, &ymid);
	XSetFont(gd.dpy, gc,font);
	XDrawImageString(gd.dpy, xid, gc,  (
		bar_xstart + (bar_width/2) + xmid), (y1 + (ymid*2) + 1), units, strlen(units));
  break;
	
  case 1:  /* HorizontaL orientation */
        decimate = num_entries/ 16; /* don't put more than 16 labels on */
	if(decimate < 1) decimate = 1;

	bar_xstart = bar_width + x1;
	bar_ystart = y1  + 1 ;
	
	for(i=0; i < num_entries; i++ ) {
	    bar_xend = ((bar_width + ((i +1) * bar_width) + 0.5))+ x1;
	    bar_wd   = bar_xend - bar_xstart;
	    
	    /* fill in color rectangle */
	    XSetForeground(gd.dpy,gd.def_gc,vc[i]->pixval);
	    XFillRectangle(gd.dpy,xid,gd.def_gc,bar_xstart,bar_ystart,bar_wd,bar_height);
	    

	    /* Decimate the number of labels */
	    if(decimate == 1 || i % decimate == 0) {
	    
	      if ((len = strlen(vc[i]->label)) != 0) {
		    STRcopy(label,vc[i]->label,LABEL_LENGTH);
		
		    x_pos = bar_xstart + (bar_width/2) + xmid;
		    y_pos = bar_yend  + (bar_height/2) + ymid;
		    XDrawImageString(gd.dpy,xid,gc, x_pos, y_pos, label, len);
	      } else { /* numeric label */
		
		if(fabs(vc[i]->max - vc[i]->min) == 1.0) {	/* If range == 1; center label */
		    x_pos =  (bar_xstart  + (bar_height/2) + xmid);
		} else {		/* Place label at left edge of bar */
		    x_pos = bar_xstart - 1;
			ival = vc[i]->min;   /* convert to an integer */
		    if(ival == vc[i]->min) {
				sprintf(label,"%d",ival);
			} else {
				sprintf(label,"%.3g",vc[i]->min);
		    }

		    y_pos =  (bar_yend  + (bar_height/2) + ymid);
		
		    XDrawImageString(gd.dpy,xid,gc, x_pos, y_pos, label, strlen(label));
	      }
	    }
	  }
	  bar_xstart = bar_xend;
	}

	/* place a max val at the right side of the scale if appropriate */
	if(strlen(vc[num_entries-1]->label) == 0 && (fabs(vc[num_entries-1]->max - vc[num_entries-1]->min) != 1.0)) {
		x_pos = bar_xstart - 1;
		ival = vc[num_entries-1]->max;   /* convert to an integer */
	    if(ival == vc[num_entries-1]->max) {
			sprintf(label,"%d",ival);
		} else {
			sprintf(label,"%.3g",vc[num_entries-1]->max);
		}
		XDrawImageString(gd.dpy,xid,gc, x_pos, y_pos, label, strlen(label));
	}
	
	font = choose_font(units,bar_wd,bar_height,&xmid,&ymid);
	XSetFont(gd.dpy,gc,font);
	XDrawImageString(gd.dpy,xid,gc, bar_xstart + (bar_wd /2),(y1 + (height /2) + ymid),units,strlen(units));
  break;
	
  }
    return;
}

#ifndef LINT
static char RCS_id[] = "$Id: cidd_cbar.c,v 1.14 2016/03/07 18:28:26 dixon Exp $";
#endif /* not LINT */
 

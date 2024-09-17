// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/****************************************************************************
 * RENDER_CBAR.C : Routines used to draw the colorbars for CIDD
 *
 * F. Hage    Jan 1991    NCAR - EOL
 */

#define RENDER_CBAR

#include "cidd.h"

/************************************************************************
 * DRAW_COLORBAR : Draw and Label a color bar : Orient ==0 - Vert,
 * == 1, Horiz. 
 */

void
draw_colorbar(Display *dpy, QPaintDevice *pdev, QBrush brush,  int x1,  int y1,  int width,  int height,
	      int num_entries,  Val_color_t **vc,  int orient,  const char *units)
{
    int 	i;
    u_int       len;
    int		decimate;
    int 	xmid, ymid;
    int 	bar_wd = 0;
    int 	bar_xstart;
    int 	bar_ystart;
    int		bar_yend = 0;
    int 	bar_xend = 0;
    int		y_pos = 0, x_pos = 0;
    int	     	bar_width = 0;
    int	     	bar_height = 0;
    u_int       max_label;
    int         ival;
    int         reverse_scale;
    char    	label[LABEL_LENGTH];
    Font    	font;
    

    if(height < 10 || width < 5) return;
    
    switch(orient) {
    case 0:  /* Vertical orientation */
	bar_height = (int)(((double) height / ((double) num_entries +2.5)) + 0.5);
	bar_width = width - 2;
    break;
    case 1:  /* Horizontal orientation */
	bar_width = (int)(((double) width / ((double) num_entries +2.5)) + 0.5);
	bar_height = height - 2 ;
    break;
    }
	

    /* Select a font for all labels */
    max_label = 0;
    for(i=0; i < num_entries; i++ ) {    /* Find the widest label string */
	if((len = strlen(vc[i]->label)) > 0) {
	    if(len > max_label) max_label = len;
	} else {
	    snprintf(label,LABEL_LENGTH,"%.3g",vc[i]->min);
	    if(strlen(label) > max_label) max_label = strlen(label);
	}
    }
    for(i=0; i < (int) max_label; i++) {  /* Set a dummy label to all W's */
	label[i] = 'W';  /* a wide character */
    }
    label[i] = '\0';
    font = choose_font(label, bar_width, bar_height-1, &xmid, &ymid);
    XSetFont(dpy, gc, font);

    decimate = num_entries/ 32; /* don't put more than 32 labels on */

    // Determine if the order of the color scale goes low to high
    reverse_scale = vc[0]->min < vc[num_entries -1]->min ? 0 : 1;
    
  switch(orient) {
  case 0:  /* Vertical orientation */
        decimate = num_entries/ 32; /* don't put more than 32 labels on */
	if(decimate < 1) decimate = 1;

	bar_xstart = x1 + 1 ;
	bar_ystart = y1 + height - bar_height;


	if(reverse_scale) {
	  /* place a max val at the bottom of the scale if appropriate */
	  if(strlen(vc[0]->label) == 0 ) {
		y_pos = bar_ystart  + 1 + (ymid *2);
		ival =  (int) vc[num_entries-1]->max;   /* convert to an integer */
	    if(ival == vc[0]->max) {
			snprintf(label,LABEL_LENGTH,"%d",ival);
		} else {
			snprintf(label,LABEL_LENGTH,"%.3g",vc[0]->max);
		}
		if(strncmp(label,"       ",strlen(label)) != 0)  {
		    XDrawImageString(dpy,xid,gc, (bar_xstart + (bar_width/2) + xmid), y_pos, label, strlen(label));
		}
	  }
	}
	 
	for(i=0; i < num_entries; i++ ) {
	    bar_yend = bar_ystart - bar_height;
	    
	    /* fill in color rectangle */
	    XSetForeground(dpy,gd.def_gc,vc[i]->pixval);
	    XFillRectangle(dpy,xid,gd.def_gc,bar_xstart,bar_yend,bar_width,bar_height);
	    /* Decimate the number of labels */
	    if(decimate == 1 || i % decimate == 0) {
	     
	      if ((len = strlen(vc[i]->label)) > 0) {
		    STRcopy(label,vc[i]->label,LABEL_LENGTH);
		
		    x_pos = bar_xstart + (bar_width/2) + xmid;
		    y_pos = bar_yend  + (bar_height/2) + ymid;
		    if(*label == ' ' ) { // Labels that Begin with Space have no bakground blanking
		        XDrawString(dpy,xid,gc, x_pos, y_pos, label, len);
			} else {
		        XDrawImageString(dpy,xid,gc, x_pos, y_pos, label, len);
			}

	      } else {	    /* numeric label */
		
	        if(reverse_scale) {	/* Place label at top of bar */
	                y_pos = bar_yend + 1 + (2 * ymid);
	        } else {	/* Place label at bottom of bar */
	                y_pos = bar_ystart - 1;
	        }
	        ival = (int) vc[i]->min;   /* convert to an integer */
	        if(ival == vc[i]->min) {
			    snprintf(label,LABEL_LENGTH,"%d",ival);
		    } else {
			    snprintf(label,LABEL_LENGTH,"%.3g",vc[i]->min);
		    }
		
		    if(strncmp(label,"       ",strlen(label)) != 0)  {
		        XDrawImageString(dpy,xid,gc,
	  	            (bar_xstart + (bar_width/2) + xmid), y_pos, label, strlen(label));
		    }
	      }
	    }
	    bar_ystart = bar_yend;
	} 

	if(reverse_scale == 0) {
	  /* place a max val at the top of the scale if appropriate */
	  if(strlen(vc[num_entries-1]->label) == 0 ) {
		y_pos = bar_ystart - 1;
		ival =  (int) vc[num_entries-1]->max;   /* convert to an integer */
	    if(ival == vc[num_entries-1]->max) {
			snprintf(label,LABEL_LENGTH,"%d",ival);
		} else {
			snprintf(label,LABEL_LENGTH,"%.3g",vc[num_entries-1]->max);
		}
		XDrawImageString(dpy,xid,gc, (bar_xstart + (bar_width/2) + xmid), y_pos, label, strlen(label));
	  }
	}
	
	font = choose_font(units, width, bar_height, &xmid, &ymid);
	XSetFont(dpy, gc,font);
	XDrawImageString(dpy, xid, gc,  (
		bar_xstart + (bar_width/2) + xmid), (y1 + (ymid*2) + 1), units, strlen(units));
  break;
	
  case 1:  /* Horizontal orientation */
        decimate = num_entries/ 16; /* don't put more than 16 labels on */
	if(decimate < 1) decimate = 1;

	bar_xstart = bar_width + x1;
	bar_ystart = y1  + 1 ;
	
	if(reverse_scale) {
	  /* place a minval at the left side of the scale if appropriate */
	  if(strlen(vc[0]->label) == 0 ) {
		x_pos = bar_xstart + bar_width - (2*xmid);
		ival =  (int) vc[0]->min;   /* convert to an integer */
	    if(ival == vc[0]->min) {
			snprintf(label,LABEL_LENGTH,"%d",ival);
		} else {
			snprintf(label,LABEL_LENGTH,"%.3g",vc[0]->min);
		}
		XDrawImageString(dpy,xid,gc, x_pos, y_pos, label, strlen(label));
	  }
	}

	for(i=0; i < num_entries; i++ ) {
	    bar_xend =  (int) ((bar_width + ((i +1) * bar_width) + 0.5))+ x1;
	    bar_wd   = bar_xend - bar_xstart;
	    
	    /* fill in color rectangle */
	    XSetForeground(dpy,gd.def_gc,vc[i]->pixval);
	    XFillRectangle(dpy,xid,gd.def_gc,bar_xstart,bar_ystart,bar_wd,bar_height);
	    

	    /* Decimate the number of labels */
	    if(decimate == 1 || i % decimate == 0) {
	    
	      if ((len = strlen(vc[i]->label)) != 0) {
		    STRcopy(label,vc[i]->label,LABEL_LENGTH);
		
		    x_pos = bar_xstart + (bar_width/2) + xmid;
		    y_pos = bar_yend  + (bar_height/2) + ymid;
		    if(*label == ' ' ) { // Labels that Begin with Space have no bakground blanking
		        XDrawString(dpy,xid,gc, x_pos, y_pos, label, len);
			} else {
		        XDrawImageString(dpy,xid,gc, x_pos, y_pos, label, len);
			}
	      } else { /* numeric label */
		
		    if(reverse_scale) {	/* Place label at right edge of bar */
		        x_pos = bar_xstart + bar_width - (2*xmid); ;
		    } else {	/* Place label at left edge of bar */
		        x_pos = bar_xstart - 1;
		    }
			ival =  (int) vc[i]->min;   /* convert to an integer */
		    if(ival == vc[i]->min) {
				snprintf(label,LABEL_LENGTH,"%d",ival);
			} else {
				snprintf(label,LABEL_LENGTH,"%.3g",vc[i]->min);
		    }

		    y_pos =  (bar_yend  + (bar_height/2) + ymid);
		
		    XDrawImageString(dpy,xid,gc, x_pos, y_pos, label, strlen(label));
	    }
	  }
	  bar_xstart = bar_xend;
	}

	if(reverse_scale == 0) {
	  /* place a max val at the right side of the scale if appropriate */
	  if(strlen(vc[num_entries-1]->label) == 0 ) {
		x_pos = bar_xstart - 1;
		ival =  (int) vc[num_entries-1]->max;   /* convert to an integer */
	    if(ival == vc[num_entries-1]->max) {
			snprintf(label,LABEL_LENGTH,"%d",ival);
		} else {
			snprintf(label,LABEL_LENGTH,"%.3g",vc[num_entries-1]->max);
		}
		XDrawImageString(dpy,xid,gc, x_pos, y_pos, label, strlen(label));
	  }
	}
	
	font = choose_font(units,bar_wd,bar_height,&xmid,&ymid);
	XSetFont(dpy,gc,font);
	XDrawImageString(dpy,xid,gc, bar_xstart + (bar_wd /2),(y1 + (height /2) + ymid),units,strlen(units));
  break;
	
  }
    return;
}

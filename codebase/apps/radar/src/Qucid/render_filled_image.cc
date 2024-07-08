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
/************************************************************************
 * RENDER_FILLED_IMAGE
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_FILLED_IMAGE

#include "cidd.h"

/**********************************************************************
 * DRAW_FILLED_IMAGE: Render an image using pixel fills
 *
 */

int draw_filled_image( Drawable xid, int x_start[], int y_start[], met_record_t *mr)
{
    unsigned short    miss;           /* missing/unmeasured data value */
    unsigned short    bad;            /* bad/noise data value */
    unsigned short   *im_ptr,*data_ptr;    /* pointers to the image & data arrays */
    unsigned short    *last_row_ptr;
    unsigned short    *im_data;
    int    yi_index;            /* row index for the image */
    int    flag;
    int    x,y;
    int    len,width;
    XImage *img;
    Visual  *visual;

    /********* Retrieve the Horizontal Display Image *********/
    visual = DefaultVisual(gd.dpy,0);

    if(gd.debug2) printf("Drawing Pixel- filled image, field: %s \n",
		      mr->field_label);
     
    /* get memory areas for header and data */
    im_data = (unsigned short *)  calloc(1,gd.h_win.can_dim.width * gd.h_win.can_dim.height);
    if(im_data == NULL) return 0;
    img = XCreateImage(gd.dpy,visual,8,ZPixmap,0,(char *)im_data,
            gd.h_win.can_dim.width,gd.h_win.can_dim.height,8,0);
    
    mr->num_display_pts = 0;
    width = gd.h_win.can_dim.width;
    data_ptr = (unsigned short *) mr->h_data;
    if(data_ptr == NULL) return CIDD_FAILURE;
    miss = (unsigned short) mr->h_fhdr.missing_data_value;
    bad = (unsigned short) mr->h_fhdr.bad_data_value;

    /**** Fill in blank rows before data area ****/
    im_ptr = im_data + (width * (y_start[0] +1));
    len = ((gd.h_win.can_dim.height - y_start[0]) -1) * width;
    if(len > 0) memset((char *)im_ptr,gd.legends.background_color->pixval,len);
    im_ptr -= width;

    yi_index = y_start[0];
    
    /*** Fill in data area rows  ***/
    for(y=1; y <= mr->h_fhdr.ny; y++) {
        /* Determine if this data row needs rendering */
        flag = 0;
        if(yi_index >= y_start[y])  flag = 1;
         
        if(flag) {    /* this data row needs rendering */
            last_row_ptr = im_ptr;

            /**** Fill in blank area to the left of the data area */
            len = x_start[0];
            if(len > 0) {
                memset((char *)im_ptr,gd.legends.background_color->pixval,len);
                im_ptr += len;
            }
    
            /**** Fill in one pixel row of data values */
            for(x = 0; x < mr->h_fhdr.nx; x++) {
                len = x_start[x+1] - x_start[x];
                    if(*data_ptr == miss ) {
                        memset((char *)im_ptr,gd.layers.missing_data_color->pixval,len);
		    } else if(  *data_ptr == bad ) {
                        memset((char *)im_ptr,gd.layers.bad_data_color->pixval,len);
                    } else {
                        memset((char *)im_ptr,mr->h_vcm.val_pix[*data_ptr],len+1);
                        mr->num_display_pts++;
                    }
                    im_ptr += len;
                data_ptr++;
            }
    
            /**** fill in blank area to the right of the data area */
            len = width  - x_start[mr->h_fhdr.nx];  
            if(len > 0) {   
                 memset((char *)im_ptr,gd.legends.background_color->pixval,len);
                im_ptr += len;
            }
    
            /**** copy the pixel row to fill out the data row */
            yi_index--;
            im_ptr-= (2 * width); /* set image pointer back to previous row */
            while(yi_index > y_start[y]) {
                memcpy((char *)im_ptr,(char *)last_row_ptr,width);
                im_ptr-= width;
                yi_index--;
            }
    
        } else {    /* skip this row of data */
            data_ptr += mr->h_fhdr.nx;
        }
    }
    
    /**** Fill in blank rows after data area ****/
    len =  (y_start[mr->h_fhdr.ny] +1)  * width;
    if(len > 0) {
        memset((char *)im_data,gd.legends.background_color->pixval,len);
    }

    if(gd.debug2) printf("NUM PIXEL FILLED PTS: %d of %lld\n",
                         mr->num_display_pts,mr->h_fhdr.nx*mr->h_fhdr.ny);
    
    /**** Now copy to the canvas ****/
    XPutImage(gd.dpy,xid,gd.def_gc,img,0,0,0,0,
        gd.h_win.can_dim.width,gd.h_win.can_dim.height);

    XDestroyImage(img);    /* Deallocates both the structure and the image data */
    return CIDD_SUCCESS;
}

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
/*************************************************************************
 * MOVIE_FRAME_SAVE.C: Routines to Save Movie cwframesloops
 *
 * For the Configurable Interactive Data Display (CIDD)
 * Frank Hage   July 1991 NCAR, Research Applications Program
 */

#define MOVIE_FRAME_SAVE 1
#include "cidd.h"

/*************************************************************************
 * SAVE_H_MOVIE_FRAME: Saves the given pixmap in either a pixmap cache
 *  Or on disk as a compressed image. In HTML Mode, it saves the
 *  Frame as an image file, after rendering products
 */

int save_h_movie_frame( int index, QPaintDevice *pdev, int page)
{
    if(gd.debug1) {
        fprintf(stderr,"\nSaving  horiz movie_frame %d\n",index);
        if( _params.image_dir != NULL) {
	        fprintf(stderr,"DIR: %s html_mode: %d series_save_active: %d\n",
		       _params.image_dir,_params.html_mode,gd.series_save_active);
	    } else {
	        fprintf(stderr,"DIR: NULL html_mode: %d series_save_active: %d\n",
		       _params.html_mode,gd.series_save_active);
	    }
    }
    /* Save the image for HTML or Series save inclusion */
    if(_params.image_dir != NULL && (_params.html_mode || gd.series_save_active)) {

        set_busy_state(1);
	    /*
		 * Copy the finished image to the final drawing area pixmap
         */
#ifdef NOTYET
        XCopyArea(gd.dpy, xid,
                  gd.h_win.can_xid[gd.h_win.cur_cache_im], 
                  gd.def_gc,    0,0,
                  gd.h_win.can_dim.width,
                  gd.h_win.can_dim.height,
                  gd.h_win.can_dim.x_pos,
                  gd.h_win.can_dim.y_pos);
#endif
                                    
        /* Save the final drawing area pixmap  */
		gd.generate_filename = 1;

		dump_cidd_image(PLAN_VIEW,0,0,page);

        set_busy_state(0);
    }

    /* If not the primary field;
     * - Is a background rendered image not to be saved 
     */
    if(page != gd.h_win.page) return 0;

#ifdef NOTYET
    if(gd.movie.frame[index].h_xid != 0 &&
		gd.movie.frame[index].h_xid != xid  ) {
        
        if(gd.debug2) fprintf(stderr,"Caching XID: %ld to frame %d XID: %ld\n",
				 xid,index,gd.movie.frame[index].h_xid);
        
        XCopyArea(gd.dpy,xid,
            gd.movie.frame[index].h_xid,
            gd.def_gc,  0,0,
            gd.h_win.can_dim.width,
            gd.h_win.can_dim.height,
            gd.h_win.can_dim.x_pos,
            gd.h_win.can_dim.y_pos);

    }
#endif
        
    return 0;
}

/*************************************************************************
 * SAVE_V_MOVIE_FRAME:
 */

int save_v_movie_frame( int index, QPaintDevice *pdev)
{
    if(gd.debug1) fprintf(stderr,"Saving XSECT movie_frame %d Save_active: %d\n",index,gd.series_save_active);

#ifdef NOTYET
    if(gd.movie.frame[index].v_xid != 0 &&
		gd.movie.frame[index].v_xid != xid) { 

        if(gd.debug2) fprintf(stderr, "save_v_movie_frame: Copying Vertical Xid: %ld to movie_frame %d XID: %ld\n",
             xid,index,gd.movie.frame[index].v_xid);
        XCopyArea(gd.dpy,xid,
            gd.movie.frame[index].v_xid,
            gd.def_gc,  0,0,
            gd.v_win.can_dim.width,
            gd.v_win.can_dim.height,
            gd.v_win.can_dim.x_pos,
            gd.v_win.can_dim.y_pos);

    }
#endif

    /* Save the image for HTML or Series save inclusion */
    if(_params.image_dir != NULL && gd.series_save_active != 0 ) {

        set_busy_state(1);

		gd.generate_filename = 1;

		dump_cidd_image(XSECT_VIEW,0,0,gd.v_win.page);

        set_busy_state(0);
    }

    return(0);
}

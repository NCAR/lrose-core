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
 * MOVIE_FRAME_RETRIEVE.CC: Routines used to retrieve Movie frames
 *
 * For the Configurable Interactive Data Display (CIDD)
 * Frank Hage   July 1991 NCAR, Research Applications Program
 */

#define MOVIE_FRAME_RETRIEVE 1
#include "cidd.h"

/*************************************************************************
 * RETRIEVE_H_MOVIE_FRAME:
 */

void retrieve_h_movie_frame(int    index, Drawable xid)
{
    if(gd.movie.frame[index].h_xid != 0) {        /* can get from the pixmap */
        if(gd.movie.frame[index].h_xid != xid) {
            if(gd.debug1) fprintf(stderr, "retrieve_h_movie_frame: Copying Horizontal movie_frame %d Xid: %ld to XID: %ld\n",
                index,gd.movie.frame[index].h_xid, xid);
            XCopyArea(gd.dpy,gd.movie.frame[index].h_xid,
                xid,
                gd.def_gc,  0,0,
                gd.h_win.can_dim.width,
                gd.h_win.can_dim.height,
                gd.h_win.can_dim.x_pos,
                gd.h_win.can_dim.y_pos);
        }
    
    } 
    
}

/*************************************************************************
 * RETRIEVE_V_MOVIE_FRAME:
 */

void retrieve_v_movie_frame(int index, Drawable xid)
{
    if(gd.movie.frame[index].v_xid != 0) {        /* can save to the pixmap */
        if(gd.movie.frame[index].v_xid != xid) {
            if(gd.debug1) fprintf(stderr, "retrieve_v_movie_frame: Copying Horizontal movie_frame %d Xid: %ld to XID: %ld\n",
                index,gd.movie.frame[index].v_xid, xid);
            XCopyArea(gd.dpy, gd.movie.frame[index].v_xid,
                xid,
                gd.def_gc,  0,0,
                gd.v_win.can_dim.width,
                gd.v_win.can_dim.height,
                gd.v_win.can_dim.x_pos,
                gd.v_win.can_dim.y_pos);
        }

    
    } 
    
    return;
}

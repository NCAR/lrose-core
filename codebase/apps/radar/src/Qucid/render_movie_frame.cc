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
 * RENDER_MOVIE_FRAME.CC: 
 *
 * For the Configurable Interactive Data Display (CIDD)
 * Frank Hage   July 1991 NCAR, Research Applications Program
 */

#define RENDER_MOVIE_FRAME 1

#include "cidd.h"

/*************************************************************************
 * RENDER_H_MOVIE_FRAME: Render a horizontal display view
 */

int render_h_movie_frame( int index, Drawable xid)
{
    int    c_field;
    int stat = 0;

    c_field = gd.h_win.page;
    if(gd.debug2)
        fprintf(stderr,
                "Rendering Horizontal movie_frame %d - field: %d, to Xid: %ld\n",
                index, c_field,xid);

    switch(gd.movie.mode) {
        case REALTIME_MODE :
        case ARCHIVE_MODE :
            stat = gather_hwin_data(c_field,
                                    gd.movie.frame[index].time_start,
                                    gd.movie.frame[index].time_end);
            if(stat == CIDD_SUCCESS)  {
                render_horiz_display(xid,c_field,
                                     gd.movie.frame[index].time_start,
                                     gd.movie.frame[index].time_end);
            } else {
                return stat;
            }
        break;
         
        default:
            fprintf(stderr,
                    "Invalid movie mode %d in render_h_movie_frame\n",
                    gd.movie.mode);
            break;
    }


    return stat;
}

/*************************************************************************
 * RENDER_V_MOVIE_FRAME:
 */

int render_v_movie_frame( int index, Drawable xid)
{
    int    c_field;
    int    stat = 0;

    c_field = gd.v_win.page;

    if(gd.debug2) fprintf(stderr, "Rendering Vertical movie_frame %d - field %d\n", index, c_field);

    switch(gd.movie.mode) {
        case REALTIME_MODE:
        case ARCHIVE_MODE:
            stat = render_vert_display(xid, c_field,
                                       gd.movie.frame[index].time_start,
                                       gd.movie.frame[index].time_end);
        break;
         
    }

    return stat;
}

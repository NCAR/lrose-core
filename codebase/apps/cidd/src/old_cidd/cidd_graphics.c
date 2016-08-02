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
/*************************************************************************
 * CIDD_GRAPH.C : Routines that are useful for graphics operations 
 *
 *
 * For the Cartesian Radar Display (CIDD)
 * Frank Hage   July 1991 NCAR, Research Applications Program
 */

#define RD_GRAPH 1
#include "cidd.h"


/*************************************************************************
 * COMPUTE_TICK_INTERVAL: Return the tick interval given a range
 *        Range Assumed to be > 1.0 && < 10000.0
 */

double
compute_tick_interval(range)
    double range;
{
    double    arange = fabs(range);

    if(arange < 1.5) return (0.1);
    if(arange < 5.0) return (0.5);
    if(arange < 15.0) return (1.0);
    if(arange < 30.0) return (2.0);
    if(arange < 75.0) return (5.0);
    if(arange < 150.0) return (10.0);
    if(arange < 300.0) return (20.0);
    if(arange < 750.0) return (50.0);
    if(arange < 1500.0) return (100.0);
    if(arange < 3000.0) return (200.0);
    if(arange < 7500.0) return (500.0);
    return(1000.0);
}

double
compute_range(x1,y1,x2,y2)
    double x1,y1,x2,y2;
{
    double xdiff,ydiff;

    xdiff = x1 - x2;
    ydiff = y1 - y2;

    return (sqrt(((xdiff * xdiff) + (ydiff * ydiff))));
}

/*************************************************************************
 * RESET_DATA_VALID_FLAGS: Indicate data arrays are out of data, & thus
 *        are invalid
 *
 */

reset_data_valid_flags(hflag,vflag)
    int    hflag,vflag;
{
    int    i;

    if(hflag) {
        for(i=0; i < gd.num_datafields; i++) {
             gd.mrec[i]->h_data_valid = 0;
             gd.mrec[i]->h_date.unix_time = 0;
        }

        for(i=0; i < gd.extras.num_wind_sets; i++) {
            gd.extras.wind[i].wind_u->h_data_valid = 0;
            gd.extras.wind[i].wind_u->h_date.unix_time = 0;
            gd.extras.wind[i].wind_v->h_data_valid = 0;
            gd.extras.wind[i].wind_v->h_date.unix_time = 0;
            if(gd.extras.wind[i].wind_w != NULL) {
                gd.extras.wind[i].wind_w->h_data_valid = 0;
                gd.extras.wind[i].wind_w->h_date.unix_time = 0;
            }
        }
    }

    if(vflag) {
        for(i=0; i < gd.num_datafields; i++) {
             gd.mrec[i]->v_data_valid = 0;
             gd.mrec[i]->v_date.unix_time = 0;
        }
        for(i=0; i < gd.extras.num_wind_sets; i++) {
            if(gd.extras.wind[i].wind_w != NULL) {
                gd.extras.wind[i].wind_u->v_data_valid = 0;
                gd.extras.wind[i].wind_u->v_date.unix_time = 0;
                gd.extras.wind[i].wind_v->v_data_valid = 0;
                gd.extras.wind[i].wind_v->v_date.unix_time = 0;
                gd.extras.wind[i].wind_w->v_data_valid = 0;
                gd.extras.wind[i].wind_w->v_date.unix_time = 0;
            }
        }
    }
    gd.data_status_changed = 1;
}

/*************************************************************************
 * SET_REDRAW_FLAGS : Set the redraw flags for each field or frame
 *    When set to 1, Frame need rerendering
 */

set_redraw_flags(h_flag,v_flag)
    int    h_flag;
    int    v_flag;
{
    int    i,j;
    if(h_flag) {
        for(i=0; i < MAX_FRAMES; i++) {
            gd.movie.frame[i].redraw_horiz = 1;

        }
        for(i=0; i < gd.num_datafields; i++) {
            gd.h_win.redraw[i] = 1;
        }
    }

    if(v_flag) {
        for(i=0; i < MAX_FRAMES; i++) {
            gd.movie.frame[i].redraw_vert = 1;
        }
        for(i=0; i < gd.num_datafields; i++) {
             gd.v_win.redraw[i] = 1;
        }
    }
}

#ifndef LINT
static char RCS_id[] = "$Id: cidd_graphics.c,v 1.6 2016/03/07 18:28:26 dixon Exp $";
#endif /* not LINT */

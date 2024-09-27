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
 * GRAPHIC_MANAGE.CC:
 *
 * For the Cartesian Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define GRAPHIC_MANAGE    1

#include "cidd.h"

/*************************************************************************
 * MANAGE_H_PIXMAPS: Manage the creation/ destruction of pixmaps for the
 *        horizontal display window 
 */
void manage_h_pixmaps(int  mode)  /* 1= create, 2 = replace, 3 = destroy */
{
#ifdef NOTYET
  int    i;
     
  switch (mode)
  {
    case 1:        /* Create all pixmaps */
    case 2:        /* Replace Pixmaps */
      for(i=0; i < gd.num_datafields; i++) {
        if(gd.h_win.page_xid[i]) {
          XFreePixmap(gd.dpy,gd.h_win.page_xid[i]);
          gd.h_win.page_xid[i] = 0;
        }

        /* Create new field Pixmaps -f field updates automatically */
        if(gd.mrec[i]->auto_render) {
          gd.h_win.page_xid[i] = XCreatePixmap(gd.dpy, 
                                               gd.h_win.vis_xid, 
                                               gd.h_win.can_dim.width,
                                               gd.h_win.can_dim.height,
                                               gd.h_win.can_dim.depth);
          gd.h_win.redraw[i] = 1;
          /* clear drawing area */
          XFillRectangle(gd.dpy,gd.h_win.page_xid[i],gd.legends.background_color->gc,
                         0,0,gd.h_win.can_dim.width,gd.h_win.can_dim.height);
        }

      }
      /* create last stage pixmaps */
      for(i=0; i < _params.num_cache_zooms; i++) {
        if(gd.h_win.can_xid[i]) XFreePixmap(gd.dpy,gd.h_win.can_xid[i]);
        gd.h_win.can_xid[i] = XCreatePixmap(gd.dpy,
                                            gd.h_win.vis_xid,
                                            gd.h_win.can_dim.width,
                                            gd.h_win.can_dim.height,
                                            gd.h_win.can_dim.depth);
        XFillRectangle(gd.dpy,gd.h_win.can_xid[i],gd.legends.background_color->gc,
                       0,0,gd.h_win.can_dim.width,gd.h_win.can_dim.height);
      }

      /* create temporary area pixmaps */ 
      if(gd.h_win.tmp_xid) XFreePixmap(gd.dpy,gd.h_win.tmp_xid);
      gd.h_win.tmp_xid = XCreatePixmap(gd.dpy,
                                       gd.h_win.vis_xid,
                                       gd.h_win.can_dim.width,
                                       gd.h_win.can_dim.height,
                                       gd.h_win.can_dim.depth);
      XFillRectangle(gd.dpy,gd.h_win.tmp_xid,gd.legends.background_color->gc,
                     0,0,gd.h_win.can_dim.width,gd.h_win.can_dim.height);


      /* release old movie frame pixmaps */
      for(i=0;i < MAX_FRAMES; i++ )
      {
        if(gd.movie.frame[i].h_xid) {
          XFreePixmap(gd.dpy,gd.movie.frame[i].h_xid); 
          gd.movie.frame[i].h_xid = 0;
        }
      }
      for(i=0; i < gd.movie.num_frames; i++) {
        gd.movie.frame[i].h_xid =
          XCreatePixmap(gd.dpy, 
                        gd.h_win.vis_xid, 
                        gd.h_win.can_dim.width,
                        gd.h_win.can_dim.height,
                        gd.h_win.can_dim.depth);
        gd.movie.frame[i].redraw_horiz = 1;
        /* clear drawing area */
        XFillRectangle(gd.dpy,gd.movie.frame[i].h_xid,gd.legends.background_color->gc,
                       0,0,gd.h_win.can_dim.width,gd.h_win.can_dim.height);
      }

      break;

    case 3: // Destroy ALL
      for(i=0;i < MAX_FRAMES; i++ ) {
        if(gd.movie.frame[i].h_xid) {
          XFreePixmap(gd.dpy,gd.movie.frame[i].h_xid); 
          gd.movie.frame[i].h_xid = 0;
        }
      }
      for(i=0; i < gd.num_datafields; i++) {
        if(gd.h_win.page_xid[i]) {
          XFreePixmap(gd.dpy,gd.h_win.page_xid[i]);
          gd.h_win.page_xid[i] = 0;
        }
      }
      for(i=0; i < _params.num_cache_zooms; i++) {
        if(gd.h_win.can_xid[i]) {
          XFreePixmap(gd.dpy,gd.h_win.can_xid[i]);
          gd.h_win.can_xid[i] = 0;
        }
      }
      if(gd.h_win.tmp_xid) {
        XFreePixmap(gd.dpy,gd.h_win.tmp_xid);
        gd.h_win.tmp_xid = 0;
      }
      break;
  }
#endif
}

/*************************************************************************
 * MANAGE_V_PIXMAPS: Manage the creation/ destruction of pixmaps for the
 *      vertical display window
 */

void manage_v_pixmaps( int mode)   /* 1= create, 2 = replace, 3 = destroy */
{
#ifdef NOTYET
  int i;

  switch (mode) {
    case 1:     /* Create all pixmaps */
    case 2:     /* Replace Pixmaps */
      for(i=0; i < gd.num_datafields; i++) {
        if(gd.v_win.page_xid[i]) {
          XFreePixmap(gd.dpy,gd.v_win.page_xid[i]);
          gd.v_win.page_xid[i] = 0;
        }
        /* Create new field Pixmaps */
        if(gd.mrec[i]->auto_render) {
          gd.v_win.page_xid[i] = XCreatePixmap(gd.dpy,
                                               gd.v_win.vis_xid,
                                               gd.v_win.can_dim.width,
                                               gd.v_win.can_dim.height,
                                               gd.v_win.can_dim.depth);
          gd.v_win.redraw[i] = 1;
          /* Clear drawing area */
          XFillRectangle(gd.dpy,gd.v_win.page_xid[i],
                         gd.legends.background_color->gc,
                         0,0,gd.v_win.can_dim.width,
                         gd.v_win.can_dim.height);
        }
      }

      /* Create last stage Pixmaps */
      for(i=0; i < _params.num_cache_zooms; i++) {
        gd.v_win.can_xid[i] =  XCreatePixmap(gd.dpy,
                                             gd.v_win.vis_xid,
                                             gd.v_win.can_dim.width,
                                             gd.v_win.can_dim.height,
                                             gd.v_win.can_dim.depth);
        XFillRectangle(gd.dpy,gd.v_win.can_xid[i],gd.legends.background_color->gc,
                       0,0,gd.v_win.can_dim.width,gd.v_win.can_dim.height);
      }

      /* Create Pixmap for fields that don't update automatically */
      gd.v_win.tmp_xid =  XCreatePixmap(gd.dpy,
                                        gd.v_win.vis_xid,
                                        gd.v_win.can_dim.width,
                                        gd.v_win.can_dim.height,
                                        gd.v_win.can_dim.depth);
      XFillRectangle(gd.dpy,gd.v_win.tmp_xid,gd.legends.background_color->gc,
                     0,0,gd.v_win.can_dim.width,gd.v_win.can_dim.height);
      for(i=0;i < MAX_FRAMES; i++ ) {
        if(gd.movie.frame[i].v_xid) {
          XFreePixmap(gd.dpy,gd.movie.frame[i].v_xid);
          gd.movie.frame[i].v_xid = 0;
        }
      }

      for(i=0; i < gd.movie.num_frames; i++) {

        gd.movie.frame[i].v_xid = XCreatePixmap(gd.dpy,
                                                gd.v_win.vis_xid,
                                                gd.v_win.can_dim.width,
                                                gd.v_win.can_dim.height,
                                                gd.v_win.can_dim.depth);
        gd.movie.frame[i].redraw_vert = 1;
        /* Clear drawing area */
        XFillRectangle(gd.dpy,gd.movie.frame[i].v_xid,
                       gd.legends.background_color->gc,
                       0,0,gd.v_win.can_dim.width,
                       gd.v_win.can_dim.height);
      }

      break;

    case 3:
      for(i=0;i < MAX_FRAMES; i++ ) {
        if(gd.movie.frame[i].v_xid) {
          XFreePixmap(gd.dpy,gd.movie.frame[i].v_xid);
          gd.movie.frame[i].v_xid = 0;
        }
      }
      for(i=0; i < gd.num_datafields; i++) {
        if(gd.v_win.page_xid[i]) {
          XFreePixmap(gd.dpy,gd.v_win.page_xid[i]);
          gd.v_win.page_xid[i] = 0;
        }
      }
      for(i=0; i < _params.num_cache_zooms; i++) {
        if(gd.v_win.can_xid[i]) {
          XFreePixmap(gd.dpy,gd.v_win.can_xid[i]);
          gd.v_win.can_xid[i] = 0;
        }
      }
      if(gd.v_win.tmp_xid) {
        XFreePixmap(gd.dpy,gd.v_win.tmp_xid);
        gd.v_win.tmp_xid = 0;
      }

      break;
  }
#endif
} 

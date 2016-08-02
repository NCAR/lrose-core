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
 * DATA_PU_proc.c - Notify and event callback function stubs.
 */

#define DATA_PU_PROC
 
#include "cidd.h"

/*************************************************************************
 * Notify callback function for `data_st'.
 */
void
set_data_proc(item, value, event)
	Panel_item	item;
	int		value;
	Event		*event;
{
    set_field(value);
}

/*************************************************************************
 * Set proper flags which switching fields 
 */
void
set_field(int value)
{
    int i;
    char label[32];

    gd.h_win.field = gd.field_index[value];

    for(i=0; i < gd.num_datafields; i++) {
        if(gd.mrec[i]->background_render == 0) gd.h_win.redraw[i] = 1;
    }

    if(gd.mrec[gd.h_win.field]->background_render && 
       gd.h_win.field_xid[gd.h_win.field] != 0 &&
       gd.h_win.redraw[gd.h_win.field] == 0) {
       
	 save_h_movie_frame(gd.movie.cur_frame + gd.movie.first_index,
			    gd.h_win.field_xid[gd.h_win.field],
			    gd.h_win.field);
    }
       

    for(i=0; i < MAX_FRAMES; i++) {
        gd.movie.frame[i].redraw_horiz = 1;
    }

   if(gd.movie.movie_on ) reset_data_valid_flags(1,0);

   xv_set(gd.data_pu->data_st,PANEL_VALUE,value,NULL);

   /* make sure the horiz window's slider has the correct label */
   sprintf(label,"%5.1f %s",((gd.h_win.cmin_ht + gd.h_win.cmax_ht) /2.0),
	       gd.mrec[gd.h_win.field]->units_label_sects);
   
   xv_set(gd.h_win_horiz_bw->cur_ht_msg, PANEL_LABEL_STRING, label, NULL);

}


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
 * ZOOM_PU_PROC.C - Notify and event callback function stubs.
 */

#include "cidd.h"

/*************************************************************************
 * Notify callback function for `domain_st'.
 */
void
set_domain_proc(item, value, event)
	Panel_item	item;
	int		value;
	Event		*event;
{
   gd.h_win.zoom_level = value;

    /* get latest zoom coordinates */
    gd.h_win.cmin_x = gd.h_win.zmin_x[value];
    gd.h_win.cmax_x = gd.h_win.zmax_x[value];
    gd.h_win.cmin_y = gd.h_win.zmin_y[value];
    gd.h_win.cmax_y = gd.h_win.zmax_y[value];

    /* set radial global values if radial data being displayed */
    if (gd.mrec[gd.h_win.field]->data_format == PPI_DATA_FORMAT) {
	zoom_radial(gd.h_win.zmin_x[value], gd.h_win.zmin_y[value],
		    gd.h_win.zmax_x[value], gd.h_win.zmax_y[value]);
        gd.h_win.cmin_r = gd.h_win.zmin_r;
        gd.h_win.cmax_r = gd.h_win.zmax_r;
        gd.h_win.cmin_deg = gd.h_win.zmin_deg;
        gd.h_win.cmax_deg = gd.h_win.zmax_deg;

     }

     /* if called "externally" make sure choice item is set properly */
     if(event == NULL) xv_set(item,PANEL_VALUE,value,NULL);

     set_redraw_flags(1,0);
     if(!gd.always_get_full_domain) reset_data_valid_flags(1,0);
}

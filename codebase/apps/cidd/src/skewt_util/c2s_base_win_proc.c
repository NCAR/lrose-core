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
 * C2S_BASE_WIN_PROC.c - Notify and event callback functions 
 */

#include "cidd2skewt.h"

/*************************************************************************
 * Notify callback function for `def_gen_bt'.
 */
void
gen_plot_proc(item, event)
        Panel_item      item;
        Event           *event;
{
        c2s_base_win_c2s_bw_objects *ip = (c2s_base_win_c2s_bw_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);

	gather_model_data();
	write_class_file(".","c2s_temp.cls");
	spawn_skewt(".","c2s_temp.cls");
}

/*************************************************************************
 * Notify callback function for `gen_skewt_file_bt'.
 */
void
gen_file_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	c2s_base_win_c2s_bw_objects *ip = (c2s_base_win_c2s_bw_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	xv_set(gd.Load_pu_popup1->popup1, XV_SHOW, TRUE, NULL);
}


/*************************************************************************
 * Notify callback function for `slp_tx'.
 */
Panel_setting
slp_tx_proc(item, event)
        Panel_item      item;
        Event           *event;
{
	char string[128];

        c2s_base_win_c2s_bw_objects *ip = (c2s_base_win_c2s_bw_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
        char *  value = (char *) xv_get(item, PANEL_VALUE);

	gd.base_press = atof(value);

        /* Do some sanity checking */
        if(gd.base_press < 800.0 || gd.base_press > 1200.0) {
	    gd.base_press = 1000.0;
            sprintf(string,"%g",gd.base_press);
	    xv_set(item, PANEL_VALUE,string,NULL);
        }

        return panel_text_notify(item, event);
}

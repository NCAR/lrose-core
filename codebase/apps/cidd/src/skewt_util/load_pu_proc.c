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
 * LOAD_PU_PROC.c - Notify and event callback function 
 */

#include "cidd2skewt.h"

/*************************************************************************
 * Notify callback function for `dir_tx'.
 */
Panel_setting
set_dir(item, event)
	Panel_item	item;
	Event		*event;
{
	load_pu_popup1_objects *ip = (load_pu_popup1_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	char *	value = (char *) xv_get(item, PANEL_VALUE);
	
	if(strlen(value) >0 ) {
		strncpy(gd.data_dir,value,MAXPATHLEN);
	} else {
		xv_set(item, PANEL_VALUE,".",NULL);
	}
	
	return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `fname_tx'.
 */
Panel_setting
set_fname(item, event)
	Panel_item	item;
	Event		*event;
{
	load_pu_popup1_objects *ip = (load_pu_popup1_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	char *	value = (char *) xv_get(item, PANEL_VALUE);
	
	if(strlen(value) >0 ) {
		strncpy(gd.fname,value,MAXPATHLEN);
	} else {
		xv_set(item, PANEL_VALUE,gd.fname,NULL);
	}
	
	return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `load_pu_bt'.
 */
void
ld_sv_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	load_pu_popup1_objects *ip = (load_pu_popup1_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	gather_model_data();
	 
	write_class_file(gd.data_dir,gd.fname);
}

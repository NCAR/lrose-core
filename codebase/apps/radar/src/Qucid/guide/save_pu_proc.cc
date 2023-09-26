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
 * SAVE_PU_PROCS.C - Notify and event callback function stubs for CIDD's
 * Image Save confirmation panel
 */

#define SAVE_PU_PROCS

#include "cidd.h"

/*************************************************************************
 * Update the panel with the right window's parameters
 */
void update_save_panel()
{
    static char  msg[64];

    switch(gd.save_im_win) {
        default:
        case PLAN_VIEW:  /* The horizontal window */
		    sprintf(msg,"Plan view - Image size %d X %d",
				gd.h_win.img_dim.width,gd.h_win.img_dim.height);
			xv_set(gd.save_pu->save_pu_msg,PANEL_LABEL_STRING,msg,NULL);
			xv_set(gd.save_pu->dir_tx,PANEL_VALUE,gd.h_win.image_dir,NULL);
			xv_set(gd.save_pu->fname_tx,PANEL_VALUE,gd.h_win.image_fname,NULL);
			xv_set(gd.save_pu->cmd_tx,PANEL_VALUE,gd.h_win.image_command,NULL);
        break;

        case XSECT_VIEW:  /* The vertical cross section  window */
		    sprintf(msg,"Cross Section - Image size %d X %d",
				gd.v_win.img_dim.width,gd.v_win.img_dim.height);
			xv_set(gd.save_pu->save_pu_msg,PANEL_LABEL_STRING,msg,NULL);
			xv_set(gd.save_pu->dir_tx,PANEL_VALUE,gd.v_win.image_dir,NULL);
			xv_set(gd.save_pu->fname_tx,PANEL_VALUE,gd.v_win.image_fname,NULL);
			xv_set(gd.save_pu->cmd_tx,PANEL_VALUE,gd.v_win.image_command,NULL);
        break;
    }
}
/*************************************************************************
 * Notify callback function for `dir_tx'.
 */
Panel_setting set_dir_proc(Panel_item item, Event *event)
{
	char *	value = (char *) xv_get(item, PANEL_VALUE);
	
    switch(gd.save_im_win) {
        default:
        case PLAN_VIEW:  /* The horizontal window */
			STRcopy(gd.h_win.image_dir, value,MAX_PATH_LEN);
        break;

        case XSECT_VIEW:  /* The vertical cross section  window */
			STRcopy(gd.v_win.image_dir, value,MAX_PATH_LEN);
        break;
    }
	
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `fname_tx'.
 */
Panel_setting set_fname_proc(Panel_item item, Event *event)
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);
 
    switch(gd.save_im_win) {
        default:
        case PLAN_VIEW:  /* The horizontal window */
			STRcopy(gd.h_win.image_fname, value,MAX_PATH_LEN);
        break;

        case XSECT_VIEW:  /* The vertical cross section  window */
			STRcopy(gd.v_win.image_fname, value,MAX_PATH_LEN);
        break;
    }
 
 
    return panel_text_notify(item, event);
}
 

/*************************************************************************
 * Notify callback function for `cmd_tx'.
 */
Panel_setting set_command_proc(Panel_item item, Event *event)
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);
 
    switch(gd.save_im_win) {
        default:
        case PLAN_VIEW:  /* The horizontal window */
			 STRcopy(gd.h_win.image_command, value,MAX_PATH_LEN);
        break;

        case XSECT_VIEW:  /* The vertical cross section  window */
			 STRcopy(gd.v_win.image_command, value,MAX_PATH_LEN);
        break;
    }
 
    return panel_text_notify(item, event);
}
 
/*************************************************************************
 * Notify callback function for `save_bt'.
 */
void save_image_proc(Panel_item item, Event *event)
{
    // Use unused parameters
    item = 0; event = NULL;

    dump_cidd_image(gd.save_im_win,1,0,gd.h_win.page);
}
 
/*************************************************************************
 * Notify callback function for `cancel_bt'.
 */
void cancel_save_proc(Panel_item item, Event *event)
{
    // Use unused parameters
    item = 0; event = NULL;

    xv_set(gd.save_pu->save_im_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);

}

/*************************************************************************
 * Notify callback function for `gen_name_bt'. 
 */

void gen_fname_proc(Panel_item, Event *)
{
   const char *fname;

   switch(gd.save_im_win) {
        default:
        case PLAN_VIEW:  /* The horizontal window */
            fname = gen_image_fname(gd.image_horiz_prefix,gd.mrec[gd.h_win.page]);
			STRcopy(gd.h_win.image_fname, fname,MAX_PATH_LEN);
        break;

        case XSECT_VIEW:  /* The vertical cross section  window */
            fname = gen_image_fname(gd.image_vert_prefix,gd.mrec[gd.v_win.page]);
			STRcopy(gd.v_win.image_fname, fname,MAX_PATH_LEN);
        break;
   }

   update_save_panel();
}

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
 * OVER_PU_PROC.C - Notify and event callback functions for the overlay selector
 */

#define OVER_PU_PROC

#include "cidd.h"


/*************************************************************************
 * Notify callback function for `over_pu_st'.
 * Turn on and off individual overlays 
 */
void over_pu_proc(Panel_item item, u_int value, Event *event)
{
    short    i;
    // Use unused parameters
    item = 0; event = NULL;
	char *choice_str;
   
	int num_choices = (int) xv_get(gd.over_pu->over_pu_st,PANEL_NCHOICES);

	// Loop through all menu choices.
    for (i = 0; i < num_choices; i++) {
		choice_str = (char *) xv_get(gd.over_pu->over_pu_st,PANEL_CHOICE_STRING,i);
		// Set all overlays which have  a matching control label.
		for(int j = 0; j < gd.num_map_overlays; j++ ) {
	        if((strncmp(choice_str, gd.over[j]->control_label, LABEL_LENGTH)) == 0) {
               if (value & 01) {
                  gd.over[j]->active = 1;
			   } else {
                  gd.over[j]->active = 0;
             }
		   }

        }
        value >>= 1;
    }
    set_redraw_flags(1,0);
}

/*************************************************************************
 * Notify callback function for `over_lst'.
 */
int
over_list_proc(Panel_item item, char *string, Xv_opaque client_data, Panel_list_op op, Event *event, int row)
{

        switch(op) {
        case PANEL_LIST_OP_DESELECT:
		  for(int j = 0; j < gd.num_map_overlays; j++ ) {
              if((strncmp(string, gd.over[j]->control_label,
                       LABEL_LENGTH)) == 0) {
		         gd.over[j]->active = 0;
             }
          }
        break;
			
        case PANEL_LIST_OP_SELECT:
		  for(int j = 0; j < gd.num_map_overlays; j++ ) {
              if((strncmp(string, gd.over[j]->control_label,
                       LABEL_LENGTH)) == 0) {
		         gd.over[j]->active = 1;
             }
          }
        break;

        case PANEL_LIST_OP_VALIDATE:
                break;

        case PANEL_LIST_OP_DELETE:
                break;

        case PANEL_LIST_OP_DBL_CLICK:
                break;
        }

	set_redraw_flags(1,0);

        return XV_OK;
}    

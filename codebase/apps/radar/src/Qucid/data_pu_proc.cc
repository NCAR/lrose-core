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
 * DATA_PU_PROC.c - Notify and event callback functions
 */

#define DATA_PU_PROC
 
#include "cidd.h"

/*************************************************************************
 * Set proper flags which switching fields 
 */
void set_field(int value)
{
  int i;
  int tmp;
  static int last_page = 0;
  static int last_value = 0;
  static int cur_value = 0;
  
  if(value < 0) {
    tmp = last_page;
    last_page = gd.h_win.page;
    gd.h_win.page = tmp;
    tmp = cur_value;
    cur_value = last_value;
    value = last_value;
    last_value = tmp;
  } else {
    last_page = gd.h_win.page;
    last_value = cur_value;
    cur_value = value;
    gd.h_win.page = gd.field_index[value];
    cerr << "FFFFFFFFFFFF value, new page: " << cur_value << ", " << gd.h_win.page << endl;
  }
  
  for(i=0; i < gd.num_datafields; i++) {
    if(gd.mrec[i]->auto_render == 0) gd.h_win.redraw[i] = 1;
  }
  
  if(gd.mrec[gd.h_win.page]->auto_render && 
     gd.h_win.page_pdev[gd.h_win.page] != 0 &&
     gd.h_win.redraw[gd.h_win.page] == 0) {
    
    save_h_movie_frame(gd.movie.cur_frame,
                       gd.h_win.page_pdev[gd.h_win.page],
                       gd.h_win.page);
  }
  
  for(i=0; i < MAX_FRAMES; i++) {
    gd.movie.frame[i].redraw_horiz = 1;
  }
  
  if(gd.movie.movie_on ) {
    reset_data_valid_flags(1,0);
  }
  
  // xv_set(gd.data_pu->data_st,PANEL_VALUE,value,NULL);
  
  /* make sure the horiz window's slider has the correct label */
  set_height_label();

}

#ifdef NOTNOW

/*************************************************************************
 * Notify callback function for `data_st'.
 */
void set_data_proc( Panel_item item, int value, Event *event)
{
    // Use Unused parameters
    item = 0; event = NULL;

    gd.last_event_time = time((time_t *) NULL);

    set_field(value);
    set_v_field(gd.field_index[value]);
}

/*************************************************************************
 * Notify callback function for `group_list'.
 */
int
set_group_proc(Panel_item item, const char *string, Xv_opaque client_data, Panel_list_op op, Event *event, int row)
{
    int i,j;
    char *ptr;
    char *ptr2;
    char buf[8192]; 
    char *buf2; 

    for(i=0; i < gd.num_datafields; i++) {
	gd.mrec[i]->currently_displayed = FALSE;
	xv_set(gd.fields_pu->display_list,PANEL_LIST_SELECT, i, FALSE,NULL);
    }

    for(j=0 ; j < gd.gui_P->field_list_n; j++) {  // Loop through each group button

      if(xv_get(gd.data_pu->group_list,PANEL_LIST_SELECTED,j)) {  // If group is selected 
        // Scan through each data grid/field and see if its in the group list
        for(i=0; i < gd.num_datafields; i++) {
         ptr = gd.gui_P->_field_list[j].grid_list;
	 strncpy(buf,gd.gui_P->_field_list[j].grid_list,8192);
	 ptr = strtok_r(buf," ",&buf2); // prime strtok_r
	 while (ptr != NULL) {
	     // Compare labels after Underscores are replaced with Spaces if this feature is enabled .
             ptr2 = ptr;  // use a copy of the pointer for replacement purposes
	     while(_params.replace_underscores && (ptr2 = strchr(ptr2,'_'))!= NULL) *ptr2 = ' ';
             if(strstr(gd.mrec[i]->legend_name,ptr) != NULL) {
	     // If so... add it to the menu.
	     gd.mrec[i]->currently_displayed = TRUE;
	     xv_set(gd.fields_pu->display_list,PANEL_LIST_SELECT, i, TRUE,NULL);
           } // Is a match
           ptr = strtok_r(NULL," ",&buf2);  // pick up next possible token
	 } // while there are matching strings
       }  // foreach data field
      }  // if group is selected 
    } // foreach group button

    init_field_menus();

    // Press the appripriate button.
    if(gd.mrec[gd.h_win.page]->currently_displayed == TRUE) { // Current field is still on
	int target = 0;
	int num_buttons = xv_get(gd.data_pu->data_st,PANEL_NCHOICES);

	// Find out which button is occupied by the current field 
	for(i=0; i < num_buttons; i++) {
	    if(strncmp((char *) xv_get(gd.data_pu->data_st, PANEL_CHOICE_STRING,i),
		       gd.mrec[gd.h_win.page]->button_name,NAME_LENGTH) == 0) {

		 target = i;
	    }
	}

        set_field(target);
        set_v_field(gd.field_index[target]);
    } else {  // Current field is not in the menu - Back to the Top.
        set_field(0);
        set_v_field(gd.field_index[0]);
    }

    return XV_OK;
}
#endif

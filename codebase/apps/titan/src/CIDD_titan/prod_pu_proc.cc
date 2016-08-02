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
 * PROD_PU_PROC.C - Notify and event callback functions for product
 * selection
 */

#define PROD_PU_PROC

#include "cidd.h"

/*************************************************************************
 * Notify callback function for `prod_st'.
 */
void
prod_select_proc(Panel_item, unsigned int value, Event *)
{
    short   i;
    int	    need_data = 0;
 
    for (i = 0; i < gd.syprod_P->prod_info_n; i++) {
        if (value & 01) {
	    gd.prod_mgr->set_product_active(i,TRUE);
	    need_data = 1;
	} else { 
	    gd.prod_mgr->set_product_active(i,FALSE);
	}
        value >>= 1;
    }

    if(need_data) {
      if(gd.syprod_P->short_requests) {
	gd.prod_mgr->getData(gd.movie.frame[gd.movie.cur_frame].time_start,
			     gd.movie.frame[gd.movie.cur_frame].time_end);
      } else {
	gd.prod_mgr->getData(gd.epoch_start, gd.epoch_end);
      }
    }

    set_redraw_flags(1,0);
}     

/*************************************************************************
 * Notify callback function for `prod_lst'.
 */
int
prod_list_proc(Panel_item, char *, Xv_opaque , Panel_list_op op, Event *, int row)
{
    int	    need_data = 0;
 
    switch(op) {
    case PANEL_LIST_OP_DESELECT:
	gd.prod_mgr->set_product_active(row,FALSE);
        break;
 
    case PANEL_LIST_OP_SELECT:
	gd.prod_mgr->set_product_active(row,TRUE);
        need_data = 1;
        break;
 
    case PANEL_LIST_OP_VALIDATE:
        break;
 
    case PANEL_LIST_OP_DELETE:
        break;
 
    case PANEL_LIST_OP_DBL_CLICK:
        break;
    }
 

    if(need_data) {
      if(gd.syprod_P->short_requests) {
	gd.prod_mgr->getData(gd.movie.frame[gd.movie.cur_frame].time_start,
			     gd.movie.frame[gd.movie.cur_frame].time_end);
      } else {
	gd.prod_mgr->getData(gd.epoch_start, gd.epoch_end);
      }
    }

    set_redraw_flags(1,0);
 
    return XV_OK;
}                           

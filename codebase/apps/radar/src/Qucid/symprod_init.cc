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
/************************************************************************
 * SYMPROD_INIT
 *
 */
#define SYMPROD_INIT

#include "cidd.h"

/************************************************************************
 * Instantiate and Inititalize Symbolic Product Rendering Classes 
 *
 */

void init_symprods()
{
    int value;
    double min_lat,max_lat,min_lon,max_lon; 

    gd.r_context = new RenderContext(gd.dpy,gd.h_win.vis_xid,gd.def_gc,gd.cmap,gd.proj);


    gd.prod_mgr = new ProductMgr(*gd.syprod_P,*gd.r_context, (gd.debug1 | gd.debug2));

    gd.r_context->set_scale_constant(gd.scale_constant);

    get_bounding_box(min_lat,max_lat,min_lon,max_lon);
    gd.r_context->set_clip_limits(min_lat, min_lon, max_lat, max_lon);

    if(gd.syprod_P->prod_info_n <= 32) {
        value = 0;
        for (int i = 0; i < gd.syprod_P->prod_info_n && i < 32; i++) {
          // if(!gd.run_unmapped) xv_set(gd.prod_pu->prod_st,PANEL_CHOICE_STRING,i,
          // gd.syprod_P->_prod_info[i].menu_label,NULL);

	    if(gd.syprod_P->_prod_info[i].on_by_default == TRUE) value |= 1 <<i;

	    gd.prod_mgr->set_product_active(i,(int) gd.syprod_P->_prod_info[i].on_by_default);

//	    gd.prod_mgr->set_product_text_off_threshold(i, gd.syprod_P->_prod_info[i].text_off_threshold);
        }

        // Set the widget's value and size the panel to fit the widget

		if(!gd.run_unmapped) {
          // xv_set(gd.prod_pu->prod_st, PANEL_VALUE, value,XV_SHOW,TRUE,XV_X,0,XV_Y,0, NULL);
          // xv_set(gd.prod_pu->prod_pu,XV_HEIGHT,xv_get(gd.prod_pu->prod_st,XV_HEIGHT),NULL);
          // xv_set(gd.prod_pu->prod_pu,XV_WIDTH,xv_get(gd.prod_pu->prod_st,XV_WIDTH),NULL);
	    }

    } else { // Use a  scrolling list when over 32 products are configured in

        for (int i = 0; i < gd.syprod_P->prod_info_n ; i++) {
		if(!gd.run_unmapped) {
	      // xv_set(gd.prod_pu->prod_lst,
	      //      PANEL_LIST_INSERT, i,
	      //      PANEL_LIST_STRING, i, gd.syprod_P->_prod_info[i].menu_label,
	      //      PANEL_LIST_CLIENT_DATA, i, i,
	      //      NULL);

	      if(gd.syprod_P->_prod_info[i].on_by_default == TRUE) {
		  // xv_set(gd.prod_pu->prod_lst,
		  //      PANEL_LIST_SELECT, i, TRUE,
		  //      NULL);
	      }
	    }

	    gd.prod_mgr->set_product_active(i,(int) gd.syprod_P->_prod_info[i].on_by_default);
	}

		if(!gd.run_unmapped) {
          // Set the widget's value and size the panel to fit the widget
          // xv_set(gd.prod_pu->prod_st,XV_SHOW,FALSE, NULL);
          // xv_set(gd.prod_pu->prod_lst, XV_SHOW,TRUE,XV_X,0,XV_Y,0, NULL);
          // xv_set(gd.prod_pu->prod_pu,XV_HEIGHT,xv_get(gd.prod_pu->prod_lst,XV_HEIGHT),NULL);
          // xv_set(gd.prod_pu->prod_pu,XV_WIDTH,xv_get(gd.prod_pu->prod_lst,XV_WIDTH),NULL);
		}
    }
}

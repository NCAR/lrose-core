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
 * DRAW_EXPORT_INIT
 *
 */
#define DRAW_EXPORT_INIT

#include "cidd.h"

/************************************************************************
 * INIT_DRAW_EXPORT_LINKS:  Scan param file and setup links to
 *  for drawn and exported points 
 *
 */

void init_draw_export_links()
{
    int  i,len;

    gd.draw.num_draw_products = gd.draw_P->dexport_info_n;
     
    if((gd.draw.dexport =(draw_export_info_t*)
	     calloc(gd.draw.num_draw_products,sizeof(draw_export_info_t))) == NULL) {
	 fprintf(stderr,"Unable to allocate space for %d draw.dexport sets\n",gd.draw.num_draw_products);
	 perror("CIDD");
	 exit(-1);
    }


    // Allocate space for control struct and record starting values for each product.
    for(i=0; i < gd.draw.num_draw_products;  i++) {

	// Product ID  
	len = strlen(gd.draw_P->_dexport_info[i].id_label) +1;
	gd.draw.dexport[i].product_id_label = (char *)  calloc(1,len);
        STRcopy(gd.draw.dexport[i].product_id_label,gd.draw_P->_dexport_info[i].id_label,len);

	// Allocate space for product_label_text (annotations) and set to nulls
	gd.draw.dexport[i].product_label_text =  (char *) calloc(1,TITLE_LENGTH);

	strncpy(gd.draw.dexport[i].product_label_text, gd.draw_P->_dexport_info[i].default_label,TITLE_LENGTH-1);

	// FMQ URL 
	len = strlen(gd.draw_P->_dexport_info[i].url) +1;
	if(len > NAME_LENGTH) {
	    fprintf(stderr,"URL: %s too long - Must be less than %d chars. Sorry.",
			     gd.draw_P->_dexport_info[i].url,URL_LENGTH);
	    exit(-1);
	}
	gd.draw.dexport[i].product_fmq_url =  (char *) calloc(1,URL_LENGTH);
        STRcopy(gd.draw.dexport[i].product_fmq_url,gd.draw_P->_dexport_info[i].url,URL_LENGTH);

	// Get the Default valid time  
	gd.draw.dexport[i].default_valid_minutes = gd.draw_P->_dexport_info[i].valid_minutes;

	// Get the Default ID   
	gd.draw.dexport[i].default_serial_no = gd.draw_P->_dexport_info[i].default_id_no;
    }
}

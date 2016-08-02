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
/*********************************************************************
 * convert_to_symprod.cc
 *
 * RAP, NCAR, Boulder CO
 *
 * May 1999
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "pirep_spdb2symprod.hh"
#include <rapformats/pirep.h>

static void add_text(symprod_product_t *prod,
		     int vert_align,
		     double lat, double lon,
		     char *text);

/*********************************************************************
 * convert_to_symprod() - Convert the data from the SPDB database to
 *                        symprod format.
 */

void *convert_to_symprod(spdb_chunk_ref_t *spdb_hdr,
			 void *spdb_data,
			 int spdb_len,
			 int *symprod_len)
{

  symprod_product_t *prod;
  time_t now;
  
  // make a copy the SPDB chunk, and swap

  if (spdb_len < (int) sizeof(pirep_t)) {
    return NULL;
  }

  pirep_t pirep;
  pirep = *((pirep_t *) spdb_data);
  BE_to_pirep(&pirep);

  // Initialize returned values

  *symprod_len = 0;
  
  // create struct for internal representation of product

  now = time(NULL);
  if ((prod = SYMPRODcreateProduct(now, now,
				   spdb_hdr->valid_time,
				   spdb_hdr->expire_time)) == NULL) {
    return (NULL);
  }

  // flight-level text
  
  int flevel = (int) (pirep.alt / 100.0 + 0.5);
  char text[256];
  if (Params.plot_callsign) {
    sprintf(text, "%s:%.3d", pirep.callsign, flevel);
  } else {
    sprintf(text, "%.3d", flevel);
  }
  
  add_text(prod, SYMPROD_TEXT_VERT_ALIGN_BOTTOM,
	   pirep.lat, pirep.lon, text);

  // message text
  
  add_text(prod, SYMPROD_TEXT_VERT_ALIGN_TOP,
	   pirep.lat, pirep.lon, pirep.text);

  // copy internal representation of product to output buffer

  void *return_buffer = SYMPRODproductToBuffer(prod, symprod_len);

  // Put the product buffer in BE format for transmission
  
  SYMPRODproductBufferToBE((char *) return_buffer);

  // free up internal representation of product

  SYMPRODfreeProduct(prod);
  
  return(return_buffer);

}

/////////////
// add_text()

static void add_text(symprod_product_t *prod,
		     int vert_align,
		     double lat, double lon,
		     char *text)

{

  SYMPRODaddText(prod, Params.text_color,
		 Params.text_background_color,
		 lat, lon,
		 0, 0,
		 vert_align,
		 SYMPROD_TEXT_HORIZ_ALIGN_CENTER,
		 0, Params.text_font,
		 text);

}


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
 * Sept 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "tstorms_spdb2symprod.h"

/*********************************************************************
 * convert_to_symprod() - Convert the data from the SPDB database to
 *                        symprod format.
 */

void *convert_to_symprod(spdb_chunk_ref_t *spdb_hdr,
			 void *spdb_data,
			 int spdb_len,
			 int *symprod_len)
{

  static ui08 *tstorm_buf = NULL;
  static int tstorm_alloc = 0;

  symprod_product_t *prod;
  time_t now;
  
  // Initialize returned values

  *symprod_len = 0;
  
  // Copy the SPDB data to the static local buffer

  if (tstorm_alloc < spdb_len) {
    if (tstorm_buf == NULL) {
      tstorm_buf = (ui08 *)umalloc(spdb_len);
    } else {
      tstorm_buf = (ui08 *)urealloc(tstorm_buf, spdb_len);
    }
    tstorm_alloc = spdb_len;
  }
  
  memcpy(tstorm_buf, spdb_data, spdb_len);
  
  // Convert the SPDB data to native format so we can use it.

  tstorm_spdb_buffer_from_BE(tstorm_buf);

  // check buffer len

  tstorm_spdb_header_t *header = (tstorm_spdb_header_t *) tstorm_buf;
  int expected_len = tstorm_spdb_buffer_len(header);
  
  if (expected_len != spdb_len) {
    fprintf(stderr, "ERROR - %s:convert_to_symprod\n", Prog_name);
    fprintf(stderr, "spdb_len is %d\n", spdb_len);
    fprintf(stderr, "expected_len is %d\n", expected_len);
    fprintf(stderr, "Aborting\n");
    return (NULL);
  }

  // create struct for internal representation of product

  now = time(NULL);
  if ((prod = SYMPRODcreateProduct(now, now,
				   spdb_hdr->valid_time,
				   spdb_hdr->expire_time)) == NULL) {
    return (NULL);
  }

  // Convert the SPDB data to symprod format

  tstorm_spdb_entry_t *entry =
    (tstorm_spdb_entry_t *) (tstorm_buf + sizeof(tstorm_spdb_header_t));

  for (int ientry = 0; ientry < header->n_entries; ientry++, entry++) {

    // Add the current shape polyline to the product buffer

    if (Params.plot_current) {
      add_current_polygon(prod, header, entry);
    }
    
    if (!Params.valid_forecasts_only ||
	entry->forecast_valid) {

      if (Params.debug >= DEBUG_MSGS) {
	fprintf(stderr,
		"Storm valid:\n");
	fprintf(stderr,
		"     speed = %f, top = %f\n",
		entry->speed, entry->top);
      }
      // Add the forecast shape polyline to the product buffer
      
      if (Params.plot_forecast) {
	add_forecast_polygon(prod, header, entry);
      }
      
      // add arrow
      
      if (Params.plot_vectors) {
	add_forecast_vector(prod, entry);
      }
      
      // add text
      
      if (Params.plot_trend || Params.plot_speed || Params.plot_top) {
	add_text(prod, entry);
      }

    }
    else {
      if (Params.debug >= DEBUG_MSGS) {
	fprintf(stderr,
		"Skipping storm -- invalid\n");
	fprintf(stderr,
		"     speed = %f, top = %f\n",
		entry->speed, entry->top);
      }
    }
    
  } /* ientry */
  
  // set return buffer

  // copy internal representation of product to output buffer

  void *return_buffer = SYMPRODproductToBuffer(prod, symprod_len);

//  if (Params.debug >= DEBUG_ALL) {
//    SYMPRODprintProductBuffer(stderr, (char *) return_buffer);
//  }

  // Put the product buffer in BE format for transmission
  
  SYMPRODproductBufferToBE((char *) return_buffer);

  // free up internal representation of product

  SYMPRODfreeProduct(prod);
  
  return(return_buffer);

}


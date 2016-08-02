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

#include "trec_gauge_spdb2symprod.h"

/*********************************************************************
 * convert_to_symprod() - Convert the data from the SPDB database to
 *                        symprod format.
 */

void *convert_to_symprod(spdb_chunk_ref_t *spdb_hdr,
			 void *spdb_data,
			 int spdb_len,
			 int *symprod_len)
{

  static int first_call = TRUE;
  static trec_gauge_handle_t tgauge;

  symprod_product_t *prod;
  time_t now;
  
  // Initialize

  *symprod_len = 0;
  if (first_call) {
    trec_gauge_init(&tgauge);
    first_call = FALSE;
  }
  
  // Copy the SPDB data to the static local handle and
  // convert from BE

  trec_gauge_load_from_chunk(&tgauge, spdb_data, spdb_len);

  // check buffer len

  int expected_len = sizeof(trec_gauge_hdr_t) +
    tgauge.hdr->n_forecasts * sizeof(fl32);
  
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

  // add arrow for vector
  
  if (Params.plot_vectors) {
    add_vector(prod, &tgauge);
  }
      
  // add text
  
  if (Params.plot_dbz_text) {
    add_dbz_text(prod, &tgauge);
  }
  
  // set return buffer

  // copy internal representation of product to output buffer

  void *return_buffer = SYMPRODproductToBuffer(prod, symprod_len);

  if (Params.debug >= DEBUG_ALL) {
    SYMPRODprintProductBuffer(stderr, (char *) return_buffer);
  }

  // Put the product buffer in BE format for transmission
  
  SYMPRODproductBufferToBE((char *) return_buffer);

  // free up internal representation of product

  SYMPRODfreeProduct(prod);
  
  return(return_buffer);

}


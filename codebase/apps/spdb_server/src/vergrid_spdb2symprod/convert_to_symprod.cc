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
 * June 1998
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "vergrid_spdb2symprod.hh"
#include <rapformats/VerGridRegion.hh>

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
  
  // copy the SPDB chunk into the VerGridRegion class
  // BE swapping occurs during this step

  VerGridRegion vgrid;
  vgrid.readChunk(spdb_data, spdb_len);
  
  // Initialize returned values

  *symprod_len = 0;
  
  // create struct for internal representation of product

  now = time(NULL);
  if ((prod = SYMPRODcreateProduct(now, now,
				   spdb_hdr->valid_time,
				   spdb_hdr->expire_time)) == NULL) {
    return (NULL);
  }

  // Convert the SPDB data to symprod format

  for (int i = 0; i < vgrid.nRegions; i++) {

    int do_truth = (Params.plot_truth &&
		    (vgrid.data[i].percent_covered_truth >=
		     Params.truth_percentage_threshold));
		    
    int do_forecast = (Params.plot_forecast &&
		       (vgrid.data[i].percent_covered_forecast >=
			Params.forecast_percentage_threshold));

    if (!do_truth && !do_forecast) {
      continue;
    }

    // compute circle geometry
  
    compute_geometry(vgrid.data[i].latitude,
		     vgrid.data[i].longitude,
		     vgrid.data[i].radius);
		     
    // Add the truth products
    
    if (do_truth) {
      add_truth_circle(prod);
      if (Params.plot_text) {
	add_truth_text(prod, vgrid.data[i].percent_covered_truth);
      }
    }

    // Add the forecast products
    
    if (do_forecast) {
      add_forecast_circle(prod);
      if (Params.plot_text) {
	add_forecast_text(prod, vgrid.data[i].percent_covered_forecast);
      }
    }

    if (Params.plot_time) {
      add_time_text(prod, vgrid.hdr->forecast_time);
    }

  } // i
  
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


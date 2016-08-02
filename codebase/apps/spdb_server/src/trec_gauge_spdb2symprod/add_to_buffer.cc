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
 * add_to_buffer.cc
 *
 * Routines for adding objects to the buffer
 *
 * RAP, NCAR, Boulder CO
 *
 * Oct 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "trec_gauge_spdb2symprod.h"

/*
 * file scope prototypes
 */

/*********************************************************************
 * add_vector()
 *
 *   Add an arrow to show movement vector
 *
 */

void add_vector(symprod_product_t *prod,
		trec_gauge_handle_t *tgauge)
  
{

  double lead_time =
    (tgauge->hdr->n_forecasts - 1) * tgauge->hdr->forecast_delta_time;

  double dx = (tgauge->hdr->u * lead_time / 1000.0);
  double dy = (tgauge->hdr->v * lead_time / 1000.0);

  double dist = sqrt(dx * dx + dy * dy);
  double dirn = atan2(dx, dy) * RAD_TO_DEG;

  SYMPRODaddArrowEndPt(prod,
		       Params.vector_color,
		       convert_line_type_param(Params.suggested_line_type),
		       Params.suggested_line_width,
		       convert_capstyle_param(Params.suggested_capstyle),
		       convert_joinstyle_param(Params.suggested_joinstyle),
		       tgauge->hdr->lat,
		       tgauge->hdr->lon,
		       dist, dirn,
		       Params.arrow_head_len,
		       Params.arrow_head_half_angle);

}

/*********************************************************************
 * add_dbz_text()
 *
 *   Add a SYMPROD text object for each dbz value
 */

void add_dbz_text(symprod_product_t *prod,
		  trec_gauge_handle_t *tgauge)

{

  for (int i = 0; i < tgauge->hdr->n_forecasts; i++) {

    double lead_time = i * tgauge->hdr->forecast_delta_time;
    double dx = -(tgauge->hdr->u * lead_time / 1000.0);
    double dy = -(tgauge->hdr->v * lead_time / 1000.0);
  
    double lat, lon;

    PJGLatLonPlusDxDy(tgauge->hdr->lat, tgauge->hdr->lon,
		      dx, dy, &lat, &lon);

    char dbz_text[64];

    sprintf(dbz_text, "%.1f", tgauge->dbz[i]);

    SYMPRODaddText(prod, Params.text_color, "",
		   lat, lon,
		   0, 0,
		   SYMPROD_TEXT_VERT_ALIGN_CENTER,
		   SYMPROD_TEXT_HORIZ_ALIGN_CENTER,
		   0, Params.text_font,
		   dbz_text);
    
  }

}

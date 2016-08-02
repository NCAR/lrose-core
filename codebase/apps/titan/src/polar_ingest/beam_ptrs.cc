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
/******************************************************************
 * beam_ptrs.c
 *
 * get ptrs associated with various beam data types
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * December 1994
 *
 *******************************************************************/

#include "polar_ingest.h"

static ll_params_t *LL_params;
static ui08 *LL_data;

static rp7_params_t *RP7_params;
static ui08 *RP7_data;

static bprp_params_t *BPRP_params;
static bprp_data_t *BPRP_data;

static gate_data_radar_params_t *GD_rparams;
static gate_data_field_params_t *GD_fparams;
static gate_data_beam_header_t *GD_beam_hdr;
static ui08 *GD_data;

void set_bprp_ptrs(bprp_params_t *params,
		   bprp_data_t *data)
     
{

  BPRP_params = params;
  BPRP_data = data;
  
}

void get_bprp_ptrs(bprp_params_t **params,
		   bprp_data_t **data)
     
{

  *params = BPRP_params;
  *data = BPRP_data;
  
}

void set_lincoln_ptrs(char *buffer)
     
{

  LL_params = (ll_params_t *) buffer;
  LL_data = (ui08 *) (buffer + sizeof(ll_params_t));

}

void get_lincoln_ptrs(ll_params_t **params,
		      ui08 **data)

{

  *params = LL_params;
  *data = LL_data;

}

void set_rp7_ptrs(char *buffer)
     
{

  RP7_params = (rp7_params_t *) buffer;
  RP7_data = (ui08 *) (buffer + sizeof(rp7_params_t));
  
}

void get_rp7_ptrs(rp7_params_t **params,
		  ui08 **data)
     
{

  *params = RP7_params;
  *data = RP7_data;
  
}

void set_gate_data_ptrs(gate_data_radar_params_t *rparams,
			gate_data_field_params_t *fparams,
			gate_data_beam_header_t *beam_hdr,
			ui08 *data)

{

  GD_rparams = rparams;
  GD_fparams = fparams;
  GD_beam_hdr = beam_hdr;
  GD_data = data;

}

void get_gate_data_ptrs(gate_data_radar_params_t **rparams,
			gate_data_field_params_t **fparams,
			gate_data_beam_header_t **beam_hdr,
			ui08 **data)
     
{

  *rparams = GD_rparams;
  *fparams = GD_fparams;
  *beam_hdr = GD_beam_hdr;
  *data = GD_data;

}

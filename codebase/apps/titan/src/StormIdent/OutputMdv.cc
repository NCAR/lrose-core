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
///////////////////////////////////////////////////////////////
// OutputMdv.cc
//
// Handles debugging output to MDV files. The output headers are
// in large part copied from the InputMdv object.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1998
//
///////////////////////////////////////////////////////////////

#include "OutputMdv.hh"
#include "InputMdv.hh"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <mdv/mdv_write.h>
#include <mdv/mdv_utils.h>
#include <mdv/mdv_user.h>
using namespace std;

//////////////
// Constructor

OutputMdv::OutputMdv(char *prog_name,
		     Params *params,
		     InputMdv *input_mdv,
		     int n_fields,
		     char *info,
		     char *dir)
  
{
  
  _progName = STRdup(prog_name);
  _params = params;
  MDV_init_handle(&handle);
  _inputMdv = input_mdv;
  _nFields = n_fields;
  _dir = STRdup(dir);

  _init();
  _addToInfo(info);

}

/////////////
// destructor

OutputMdv::~OutputMdv()

{
  
  STRfree(_dir);
  STRfree(_progName);
  MDV_free_handle(&handle);

}

////////////////////////////////////////////////////
// _init()
//
// Initialize the volume headers from the grid params and
// headers in an input file. Allocate the field data arrays.
//

void OutputMdv::_init()
  
{

  // copy the master header
  
  MDV_master_header_t *out_mhdr = &handle.master_hdr;
  MDV_master_header_t *in_mhdr = &_inputMdv->handle.master_hdr;
  *out_mhdr = *in_mhdr;

  // make relevant changes to master header
  // all debugging output is 2-dimensional

  out_mhdr->data_dimension = 2;
  out_mhdr->vlevel_included = TRUE;
  out_mhdr->n_fields = _nFields;
  out_mhdr->max_nz = 1;
  out_mhdr->n_chunks = 0;
  out_mhdr->field_grids_differ = FALSE;

  // copy grid

  handle.grid = _inputMdv->handle.grid;

  // allocate MDV arrays
  
  MDV_alloc_handle_arrays(&handle, out_mhdr->n_fields,
			  out_mhdr->max_nz, out_mhdr->n_chunks);

  // indicate that the field planes will be allocated elsewhere, 
  // and that the handle should not free them

  handle.field_planes_allocated = FALSE;

}

////////////////////
// fillFieldHeader()
//

void OutputMdv::fillFieldHeader(int field_num,
				char *field_name_long,
				char *field_name,
				char *units,
				char *transform)

{
  
  MDV_field_header_t *out_fhdr = handle.fld_hdrs + field_num;
  MDV_field_header_t *in_fhdr = _inputMdv->handle.fld_hdrs;
  *out_fhdr = *in_fhdr;
    
  out_fhdr->nz = 1;
  out_fhdr->encoding_type = MDV_INT8;
  out_fhdr->data_element_nbytes = 1;
  out_fhdr->field_data_offset = 0;
  out_fhdr->volume_size =
    out_fhdr->nx * out_fhdr->ny * 1 * sizeof(ui08);

  out_fhdr->grid_dz = 1.0;
  out_fhdr->grid_minz = 0.5;

  out_fhdr->bad_data_value = 0;
  out_fhdr->missing_data_value = 0;

  out_fhdr->field_code = 0;
  STRncopy(out_fhdr->field_name_long, field_name_long,
	   MDV_LONG_FIELD_LEN);
  STRncopy(out_fhdr->field_name, field_name, MDV_SHORT_FIELD_LEN);
  STRncopy(out_fhdr->units, units, MDV_UNITS_LEN);
  STRncopy(out_fhdr->transform, transform, MDV_TRANSFORM_LEN);
    
  MDV_vlevel_header_t *vhdr = handle.vlv_hdrs +field_num;
  MDV_init_vlevel_header(vhdr);
  for (int iz = 0; iz < out_fhdr->nz; iz++) {
    vhdr->vlevel_type[iz] = MDV_VERT_TYPE_Z;
    vhdr->vlevel_params[iz] = out_fhdr->grid_minz + iz * out_fhdr->grid_dz;
  }
    
}

////////////////////////////////////////////////////
// _addToInfo()
//
// Add a string to the info field in the header.
//

void OutputMdv::_addToInfo(char *info_str)
{
  STRconcat(handle.master_hdr.data_set_info, info_str, MDV_INFO_LEN);
}
  
////////////////////////////////////////////////////
// loadFieldData()
//
// Set size, scale, bias and plane pointer for a given field
//

void OutputMdv::loadFieldData(int out_field, ui08 *data_plane,
			      double scale, double bias)
  
{
  
  handle.field_plane[out_field][0] = data_plane;
  MDV_field_header_t *out_fhdr = handle.fld_hdrs + out_field;
  out_fhdr->scale = scale;
  out_fhdr->bias = bias;
    
}

////////////////////////////////////////
// writeVol()
//
// Write out merged volume in MDV format.
//

int OutputMdv::writeVol()

{

  MDV_master_header_t *out_mhdr = &handle.master_hdr;

  out_mhdr->time_gen = time(NULL);

  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "Writing MDV file, time %s, to dir %s\n",
	    utimstr(out_mhdr->time_centroid), _dir);
  }
  
  // write to directory
  
  int iret = MDV_write_to_dir(&handle, _dir, MDV_PLANE_RLE8, TRUE);

  if (iret == MDV_SUCCESS) {
    return (0);
  } else {
    return (-1);
  }

}



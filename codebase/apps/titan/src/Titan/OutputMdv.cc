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

#include <toolsa/mem.h>
#include <toolsa/str.h>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
using namespace std;

const float OutputMdv::missingVal = -9999.0;

//////////////
// Constructor

OutputMdv::OutputMdv(const string &prog_name,
		     const Params &params,
		     const InputMdv &input_mdv,
		     const string &info,
		     const string &data_set_name,
		     const string &url) :
  Worker (prog_name, params),
  _inputMdv(input_mdv),
  _url(url)
  
{
  
  // set master header

  Mdvx::master_header_t mhdr = _inputMdv.mdvx.getMasterHeader();

  mhdr.time_gen = time(NULL);
  mhdr.data_dimension = 2;
  mhdr.n_fields = 0;
  mhdr.max_nz = 1;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_included = true;
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;

  _mdvx.setMasterHeader(mhdr);
  _mdvx.setDataSetInfo(info.c_str());
  _mdvx.setDataSetName(data_set_name.c_str());
  _mdvx.setDataSetSource("Created by 'Titan' program");
  
}

/////////////
// destructor

OutputMdv::~OutputMdv()

{
  
}

////////////////////
// addField()
//

void OutputMdv::addField(const char *field_name_long,
			 const char *field_name,
			 const char *units,
			 const char *transform,
			 fl32 scale,
			 fl32 bias,
			 const ui08 *data)
  
{
  
  // field header

  Mdvx::field_header_t fhdr = _inputMdv.dbzField->getFieldHeader();
  fhdr.field_code = 0;

  fhdr.nz = 1;
  fhdr.encoding_type = Mdvx::ENCODING_INT8;
  fhdr.data_element_nbytes = 1;
  fhdr.volume_size = fhdr.nx * fhdr.ny * 1 * sizeof(ui08);

  fhdr.scaling_type = Mdvx::SCALING_SPECIFIED;
  if (!strcmp(transform, "none")) {
    fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  }

  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.dz_constant = true;
  fhdr.grid_dz = 1.0;
  fhdr.grid_minz = 0.5;
  fhdr.scale = scale;
  fhdr.bias = bias;
  fhdr.bad_data_value = 0;
  fhdr.missing_data_value = missingVal;

  fhdr.min_value = 0;
  fhdr.max_value = 0;
  fhdr.min_value_orig_vol = 0;
  fhdr.max_value_orig_vol = 0;
  
  STRncopy(fhdr.field_name_long, field_name_long,
	   MDV_LONG_FIELD_LEN);
  STRncopy(fhdr.field_name, field_name, MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr.units, units, MDV_UNITS_LEN);
  STRncopy(fhdr.transform, transform, MDV_TRANSFORM_LEN);
  
  // vlevel header

  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  vhdr.level[0] = fhdr.grid_minz;
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;

  // create field

  MdvxField *fld = new MdvxField(fhdr, vhdr, data);
  fld->computeMinAndMax(true);

  // add to object

  _mdvx.addField(fld);

}

////////////////////////
// addField() - floats
//

void OutputMdv::addField(const char *field_name_long,
			 const char *field_name,
			 const char *units,
			 const char *transform,
			 const fl32 *data)
  
{
  
  // field header

  Mdvx::field_header_t fhdr = _inputMdv.dbzField->getFieldHeader();
  fhdr.field_code = 0;

  fhdr.nz = 1;
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.volume_size = fhdr.nx * fhdr.ny * 1 * sizeof(fl32);
  
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  if (!strcmp(transform, "none")) {
    fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  }

  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.dz_constant = true;
  fhdr.grid_dz = 1.0;
  fhdr.grid_minz = 0.5;
  fhdr.scale = 1.0;
  fhdr.bias = 0.0;
  fhdr.bad_data_value = 0;
  fhdr.missing_data_value = missingVal;
 
  fhdr.min_value = 0;
  fhdr.max_value = 0;
  fhdr.min_value_orig_vol = 0;
  fhdr.max_value_orig_vol = 0;
  
  STRncopy(fhdr.field_name_long, field_name_long,
	   MDV_LONG_FIELD_LEN);
  STRncopy(fhdr.field_name, field_name, MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr.units, units, MDV_UNITS_LEN);
  STRncopy(fhdr.transform, transform, MDV_TRANSFORM_LEN);
  
  // vlevel header

  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  vhdr.level[0] = fhdr.grid_minz;
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;

  // create field

  MdvxField *fld = new MdvxField(fhdr, vhdr, data);
  fld->computeMinAndMax(true);

  // add to object

  _mdvx.addField(fld);

}

////////////////////////////////////////
// writeVol()
//
// Write out merged volume in MDV format.
//

int OutputMdv::writeVol()

{

  if (_params.debug >= Params::DEBUG_NORM) {
    time_t ftime = _mdvx.getMasterHeader().time_centroid;
    cerr << "Writing MDV file, time : "
      	 << utimstr(ftime)
	   << " to URL: " << _url << endl;
  }
  
  // write out file
  
  if (_mdvx.writeToDir(_url.c_str())) {
    cerr << "ERROR - OutputMdv::writeVol" << endl;
    cerr << _mdvx.getErrStr();
    return -1;
  }
  
  return 0;

}



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
// OutputFile.cc
//
// Handles output to MDV files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2004
//
///////////////////////////////////////////////////////////////

#include "OutputFile.hh"

#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/Mdvx.hh>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>

#include <cstring>
#include <cassert>
using namespace std;

const fl32 OutputFile::missingFl32 = -9999.0f;

//////////////
// Constructor

OutputFile::OutputFile(const string &prog_name,
		       const Params &params,
		       const MdvxProj &proj,
		       const Mdvx::master_header_t &exampleMhdr,
		       const vector<Mdvx::field_header_t> &exampleFhdrs) :
  _progName(prog_name),
  _params(params),
  _proj(proj),
  _exampleMhdr(exampleMhdr),
  _exampleFhdrs(exampleFhdrs)

{
  return;
}

/////////////
// destructor

OutputFile::~OutputFile()

{
  return;
}

////////////////////////////////////////
// write()
//
// Write out merged volume in MDV format.
//

int OutputFile::write(const time_t& mergeTime,
		      const time_t& startTime, 
		      const time_t& endTime,
		      const vector<void *> fieldData,
		      const string &dataSetInfo)
  
{

  DsMdvx out;
  out.setAppName(_progName);
  
  // set the master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);
  Mdvx::coord_t coords = _proj.getCoord();
  
  ///////////////////////////////////////  mhdr.time_gen = time(NULL);

  mhdr.time_gen = startTime;
  mhdr.time_begin = startTime + _params.output_timestamp_correction;
  mhdr.time_end = endTime + _params.output_timestamp_correction;
  mhdr.time_centroid = mergeTime + _params.output_timestamp_correction;
  mhdr.time_expire = mhdr.time_centroid +
    (mhdr.time_end - mhdr.time_begin);
  mhdr.num_data_times = 1;
  if (coords.nz == 1) {
    mhdr.data_dimension = 2;
  } else {
    mhdr.data_dimension = 3;
  }
  mhdr.forecast_time = _exampleMhdr.forecast_time;
  mhdr.forecast_delta = _exampleMhdr.forecast_delta;  
  mhdr.data_collection_type = _exampleMhdr.data_collection_type;
  mhdr.native_vlevel_type = _exampleMhdr.native_vlevel_type;
  mhdr.vlevel_type = _exampleMhdr.vlevel_type;
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.n_fields = _params.merge_fields_n;
  mhdr.max_nx = coords.nx;
  mhdr.max_ny = coords.ny;
  mhdr.max_nz = coords.nz;
  mhdr.field_grids_differ = FALSE;

  out.setMasterHeader(mhdr);
  
  if (_params.copyMetaDataFromInput){
    out.setDataSetName( _exampleMhdr.data_set_name );
    out.setDataSetSource( _exampleMhdr.data_set_source);
    out.setDataSetInfo( _exampleMhdr.data_set_info );
  } else {
    out.setDataSetName(_params.output_data_set_name);
    out.setDataSetSource(_params.output_data_set_source);
    out.setDataSetInfo(dataSetInfo.c_str());
  }

  // set the fields
  
  for (int ifld = 0; ifld < _params.merge_fields_n; ifld++) {
    
    const Mdvx::field_header_t &inFhdr = _exampleFhdrs[ifld];
    
    Mdvx::field_header_t fhdr;
    MEM_zero(fhdr);
    
    fhdr.forecast_delta = inFhdr.forecast_delta;
    fhdr.forecast_time = inFhdr.forecast_time;
    fhdr.field_code = inFhdr.field_code;
    fhdr.nx = coords.nx;
    fhdr.ny = coords.ny;
    fhdr.nz = coords.nz;
    fhdr.proj_type = coords.proj_type;
    if (_params._merge_fields[ifld].merge_encoding == Params::INT8) {
      fhdr.encoding_type = Mdvx::ENCODING_INT8;
      fhdr.data_element_nbytes = sizeof(ui08);
      fhdr.scale = _params._merge_fields[ifld].merge_scale;
      fhdr.bias = _params._merge_fields[ifld].merge_bias;
      fhdr.bad_data_value = missingInt;
      fhdr.missing_data_value = missingInt;
    } else if (_params._merge_fields[ifld].merge_encoding == Params::INT16) {
      fhdr.encoding_type = Mdvx::ENCODING_INT16;
      fhdr.data_element_nbytes = sizeof(ui16);
      fhdr.scale = _params._merge_fields[ifld].merge_scale;
      fhdr.bias = _params._merge_fields[ifld].merge_bias;
      fhdr.bad_data_value = missingInt;
      fhdr.missing_data_value = missingInt;
    } else {
      fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
      fhdr.data_element_nbytes = sizeof(fl32);
      fhdr.scale = 1.0;
      fhdr.bias = 0.0;
      fhdr.bad_data_value = missingFl32;
      fhdr.missing_data_value = missingFl32;
    }
    fhdr.volume_size =
      fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;
    
    fhdr.compression_type = Mdvx::COMPRESSION_NONE;
    fhdr.transform_type = inFhdr.transform_type;
    fhdr.scaling_type = Mdvx::SCALING_NONE;
    
    fhdr.native_vlevel_type = inFhdr.native_vlevel_type;
    fhdr.vlevel_type = inFhdr.vlevel_type;
    if(_params.use_specified_vlevels) {
      fhdr.dz_constant = FALSE;
    } else {
      fhdr.dz_constant = TRUE;
    }

    fhdr.proj_origin_lat = coords.proj_origin_lat;
    fhdr.proj_origin_lon = coords.proj_origin_lon;

    if (_params.output_projection == Params::OUTPUT_PROJ_LAMBERT) {
      fhdr.proj_param[0] = coords.proj_params.lc2.lat1;
      fhdr.proj_param[1] = coords.proj_params.lc2.lat2;
    }
    else if (_params.output_projection == Params::OUTPUT_PROJ_FLAT) {
      fhdr.proj_rotation = coords.proj_params.flat.rotation;
    }
    
    fhdr.grid_dx = coords.dx;
    fhdr.grid_dy = coords.dy;
    fhdr.grid_dz = coords.dz;

    fhdr.grid_minx = coords.minx;
    fhdr.grid_miny = coords.miny;
    fhdr.grid_minz = coords.minz;
    
    // vlevel header
    
    Mdvx::vlevel_header_t vhdr;
    MEM_zero(vhdr);

    for (int iz = 0; iz < coords.nz; iz++) {
      vhdr.type[iz] = fhdr.vlevel_type;
      if(_params.use_specified_vlevels) {
        vhdr.level[iz] = _params._vlevel_array[iz];
      } else {
        vhdr.level[iz] = _params.output_grid.minz + (iz * _params.output_grid.dz);
      }
    } // End of iz loop through vertical levels
    
    MdvxField *outField = new MdvxField(fhdr, vhdr, fieldData[ifld]);
    outField->setFieldName(_params._merge_fields[ifld].name);
    outField->setFieldNameLong(inFhdr.field_name_long);
    outField->setUnits(inFhdr.units);
    outField->setTransform(inFhdr.transform);

    outField->convertType
      ((Mdvx::encoding_type_t) _params._merge_fields[ifld].output_encoding,
       (Mdvx::compression_type_t) _params.output_compression);
    
    out.addField(outField);
    
  } // out_field

  if (_params.debug) {
    cerr << "Writing merged MDV file, time " << utimstr(mhdr.time_centroid) 
	 << " to URL " << _params.output_url << endl;
  }

  if (_params.write_as_forecast)
   {
        out.setWriteAsForecast();
   }

  if (_params.write_ldata == true ||  _params.mode == Params::REALTIME)
    out.setWriteLdataInfo();  
  else
    out.clearWriteLdataInfo();

  if (out.writeToDir(_params.output_url) != 0) {
    cerr << "ERROR - " <<  _progName << "::OutputFile::write" << endl;
    cerr << out.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote file: " << out.getPathInUse() << endl;
  }

  return 0;

}


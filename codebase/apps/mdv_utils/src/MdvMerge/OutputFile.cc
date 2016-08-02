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
// August 1998
//
///////////////////////////////////////////////////////////////

#include "OutputFile.hh"
#include "Params.hh"

#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/Mdvx.hh>
#include <toolsa/str.h>
#include <toolsa/pmu.h>

#include <cstring>
#include <cassert>
using namespace std;

//////////////
// Constructor

OutputFile::OutputFile(string prog_name,
		       Params *params,
		       MdvxProj *grid,
		       NowcastQueue &nowcastQueue)

{
  //
  // Make copies of the inout arguments.
  //
  _progName = prog_name;
  _params = params;
  _handle = new DsMdvx;
  _handle->setWriteLdataInfo();
  _grid = grid;
  _nowcastQueue = nowcastQueue;


}

/////////////
// destructor

OutputFile::~OutputFile()

{
  
  _grid = 0;
  delete _handle;

}

////////////////////////////////////////////////////
// initHeaders()
//
// Initialize the volume headers from the grid params and
// headers in an input file. Allocate the field data arrays.
//

void OutputFile::initHeaders(DsMdvx *input_mdv)
{

  // get the input master header to use for a template 
  Mdvx::master_header_t in_mhdr = input_mdv->getMasterHeader();

  // clear the master header
  memset(&_outMasterHdr, 0, sizeof(_outMasterHdr));

  // fill the master header
  Mdvx::coord_t coords = _grid->getCoord();
 
  _outMasterHdr.num_data_times = in_mhdr.num_data_times;
  if (coords.nz == 1) {
    _outMasterHdr.data_dimension = 2;
  } else {
    _outMasterHdr.data_dimension = 3;
  }
  _outMasterHdr.data_collection_type = in_mhdr.data_collection_type;
  _outMasterHdr.native_vlevel_type = in_mhdr.native_vlevel_type;
  _outMasterHdr.vlevel_type = in_mhdr.vlevel_type;
  _outMasterHdr.vlevel_included = TRUE;
  _outMasterHdr.grid_orientation = in_mhdr.grid_orientation;
  _outMasterHdr.data_ordering = in_mhdr.data_ordering;
  if (_params->set_field_nums) {
    _outMasterHdr.n_fields = _params->field_nums_n2;
  } 
  else {
    _outMasterHdr.n_fields = _params->field_names_n2;
  }
  _outMasterHdr.max_nx = coords.nx;
  _outMasterHdr.max_ny = coords.ny;
  _outMasterHdr.max_nz = coords.nz;
  _outMasterHdr.n_chunks = 0;
  _outMasterHdr.field_grids_differ = FALSE;
  _outMasterHdr.sensor_lon = in_mhdr.sensor_lon;
  _outMasterHdr.sensor_lat = in_mhdr.sensor_lat;
  _outMasterHdr.sensor_alt = in_mhdr.sensor_alt;

  // data set name and source
  
  STRncopy(_outMasterHdr.data_set_name, _params->data_set_name, MDV_NAME_LEN);
  STRncopy(_outMasterHdr.data_set_source, _params->data_set_source, MDV_NAME_LEN);
  
  // fill in field headers and vlevel headers
  
  for (int i = 0; i < _outMasterHdr.n_fields; i++) {
    
    MdvxField *inField = input_mdv->getFieldByNum(i);

    assert(inField != 0);

    Mdvx::field_header_t in_fhdr = inField->getFieldHeader();

    Mdvx::field_header_t out_fhdr;
    memset(&out_fhdr, 0, sizeof(out_fhdr));

    out_fhdr.nx = coords.nx;
    out_fhdr.ny = coords.ny;
    out_fhdr.nz = coords.nz;
    out_fhdr.proj_type = coords.proj_type;

    
    if (_params->merge_type == Params::MERGE_INT8) {
      out_fhdr.encoding_type = Mdvx::ENCODING_INT8;
      out_fhdr.data_element_nbytes = sizeof(ui08);
      out_fhdr.bad_data_value = 1;
      out_fhdr.missing_data_value = 0;
      out_fhdr.volume_size =
      out_fhdr.nx * out_fhdr.ny * out_fhdr.nz * out_fhdr.data_element_nbytes;
    }
    else {
      out_fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
      out_fhdr.data_element_nbytes = sizeof(fl32);
      out_fhdr.bad_data_value = -9998.0;
      out_fhdr.missing_data_value = -9999.0;
      out_fhdr.volume_size =
      out_fhdr.nx * out_fhdr.ny * out_fhdr.nz * out_fhdr.data_element_nbytes;
    }
      
    //    out_fhdr.compression_type = Mdvx::COMPRESSION_ZLIB;

    out_fhdr.compression_type = Mdvx::COMPRESSION_NONE;
    out_fhdr.transform_type = in_fhdr.transform_type;
    out_fhdr.scaling_type = in_fhdr.scaling_type;

    out_fhdr.native_vlevel_type = in_fhdr.native_vlevel_type;
    out_fhdr.vlevel_type = in_fhdr.vlevel_type;
    out_fhdr.dz_constant = in_fhdr.dz_constant;

    out_fhdr.field_data_offset = 0;

    out_fhdr.proj_origin_lat = coords.proj_origin_lat;
    out_fhdr.proj_origin_lon = coords.proj_origin_lon;

    out_fhdr.grid_dx = coords.dx;
    out_fhdr.grid_dy = coords.dy;
    out_fhdr.grid_dz = coords.dz;

    out_fhdr.grid_minx = coords.minx;
    out_fhdr.grid_miny = coords.miny;
    out_fhdr.grid_minz = coords.minz;

    if (_params->output_projection == Params::OUTPUT_PROJ_LAMBERT) {
      out_fhdr.proj_param[0] = coords.proj_params.lc2.lat1;
      out_fhdr.proj_param[1] = coords.proj_params.lc2.lat2;
    }
    else if (_params->output_projection == Params::OUTPUT_PROJ_FLAT) {
      out_fhdr.proj_rotation = coords.proj_params.flat.rotation;
    }

    out_fhdr.field_code = in_fhdr.field_code;
    STRncopy(out_fhdr.field_name_long, in_fhdr.field_name_long,
	     MDV_LONG_FIELD_LEN);
    STRncopy(out_fhdr.field_name, in_fhdr.field_name, MDV_SHORT_FIELD_LEN);
    STRncopy(out_fhdr.units, in_fhdr.units, MDV_UNITS_LEN);
    STRncopy(out_fhdr.transform, in_fhdr.transform, MDV_TRANSFORM_LEN);
    
    _fieldNames.push_back(out_fhdr.field_name);

    Mdvx::vlevel_header_t in_vhdr = inField->getVlevelHeader();
    Mdvx::vlevel_header_t out_vhdr;
    memset(&out_vhdr, 0, sizeof(out_vhdr));
    for (int j = 0; j < coords.nz; j++) {
      out_vhdr.type[j] = in_vhdr.type[j];   // Assume all levels are the same type
      if(Mdvx::dzIsConstant(in_fhdr, in_vhdr)) {
	out_vhdr.level[j] = coords.minz + (j * coords.dz);
      }
      else {
	out_vhdr.level[j] = in_vhdr.level[j];
      }
    }
    
    MdvxField *outField = new MdvxField(out_fhdr, out_vhdr);
    _handle->addField(outField);
    outField = 0;

  } // out_field

}

////////////////////////////////////////////////////
// addToInfo()
//
// Add a string to the info field in the header.
//

void OutputFile::addToInfo(const char *info_str)
{
  STRconcat(_outMasterHdr.data_set_info, info_str, MDV_INFO_LEN);
  _handle->setDataSetInfo(_outMasterHdr.data_set_info);
}
  
  
////////////////////////////////////////////////////
// clearVol()
//
// Clears out the volume data, initialize
//

void OutputFile::clearVol()
  
{
  Mdvx::coord_t coords = _grid->getCoord();
  int npoints_vol = coords.nx * coords.ny * coords.nz;
  for (int out_field = 0; out_field < _handle->getNFields(); out_field++) {
    MdvxField *field = _handle->getFieldByNum(out_field);
    assert(field != 0);
    Mdvx::field_header_t fhdr = field->getFieldHeader();
    
    if (_params->merge_type == Params::MERGE_INT8) {
      ui08 *outp = (ui08*) field->getVol();
      if (_params->fill_value == Params::FILL_MISSING_VALUE) {
	memset(outp, (int)fhdr.missing_data_value, npoints_vol);
      }
      else {
	memset(outp, (int)fhdr.bad_data_value, npoints_vol);
      }
    }
    else {
      fl32 *outp = (fl32*) field->getVol();
      fl32 fill;
      if (_params->fill_value == Params::FILL_MISSING_VALUE) {
	fill = fhdr.missing_data_value;
      }
      else {
	fill = fhdr.bad_data_value;
      }
      for (int i = 0; i < npoints_vol; i++, outp++) {
	*outp = fill;
      }
    }
  }
  
  _handle->setDataSetInfo("");

}

////////////////////////////////////////////////////
// loadScaleAndBias()
//
// Set scale and bias for a given field
//

void OutputFile::loadScaleAndBias(const int& i, const double& scale, 
				  const double& bias)
  
{
    MdvxField *field = _handle->getFieldByNum(i);
    assert(field != 0);
    Mdvx::field_header_t fhdr = field->getFieldHeader();
    fhdr.scaling_type = Mdvx::SCALING_SPECIFIED;
    fhdr.scale = scale;
    fhdr.bias = bias;
    field->setFieldHeader(fhdr);
    
}

////////////////////////////////////////
// writeVol()
//
// Write out merged volume in MDV format.
//

bool OutputFile::writeVol(const time_t& merge_time, const time_t& start_time, 
			  const time_t& end_time)

{

  _outMasterHdr.time_gen = time(0);

  _outMasterHdr.time_begin = start_time;
  _outMasterHdr.time_end = end_time;
  _outMasterHdr.time_centroid = merge_time;
  _outMasterHdr.time_expire = _outMasterHdr.time_centroid +
    (_outMasterHdr.time_end - _outMasterHdr.time_begin);
  
  _handle->setMasterHeader(_outMasterHdr);


  // encode and compress for output
  for (int i = 0; i < _outMasterHdr.n_fields; i++)
  {
    MdvxField *field = _handle->getFieldByNum(i);

    Mdvx::scaling_type_t output_scaling_type;
    Mdvx::compression_type_t output_compression_type;
    //
    // Initialize this just to avoid compiler warnings.
    //
    output_compression_type = Mdvx::COMPRESSION_NONE;

    if(_params->use_scaling_info)
      output_scaling_type = Mdvx::SCALING_SPECIFIED;
    else 
      output_scaling_type = Mdvx::SCALING_DYNAMIC;

    if (_params->use_compression_info)
    {
      switch (_params->_compression_info[i])
      {
      case Params::COMPRESSION_NONE :
	output_compression_type = Mdvx::COMPRESSION_NONE;
	break;
      case Params::COMPRESSION_RLE :
	output_compression_type = Mdvx::COMPRESSION_RLE;
	break;
      case Params::COMPRESSION_LZO :
	output_compression_type = Mdvx::COMPRESSION_LZO;
	break;
      case Params::COMPRESSION_ZLIB :
	output_compression_type = Mdvx::COMPRESSION_ZLIB;
	break;
      case Params::COMPRESSION_BZIP :
	output_compression_type = Mdvx::COMPRESSION_BZIP;
	break;
      case Params::COMPRESSION_GZIP :
	output_compression_type = Mdvx::COMPRESSION_GZIP;
	break;
      } /* endswitch - _params->_compression_info[i] */
    }
    else
    {
      output_compression_type = Mdvx::COMPRESSION_ZLIB;
    }

    if (field->convertType(Mdvx::ENCODING_INT8,
			   output_compression_type, 
			   output_scaling_type,
			   _params->_scaling_info[i].scale,
			   _params->_scaling_info[i].bias) != 0 )
    {
      cerr << "ERROR - " <<  _progName << endl;
      cerr << field->getErrStr();
      cerr << "*** Exiting ***" << endl << endl;
      return false;
    }

  } /* endfor - i */
    

  if (_params->debug) {
    cerr << "Writing combined MDV file, time " << utimstr(_outMasterHdr.time_centroid) 
	 << ", to URL " << _params->output_url << endl;
  }

  if (_handle->writeToDir(_params->output_url) != 0) {
    cerr << "ERROR - " <<  _progName << endl;
    cerr << "Error writing data for time ";
    cerr << utimstr(_outMasterHdr.time_centroid);
    cerr << " to URL " << _params->output_url << endl;
    cerr << "*** Exiting ***" << endl << endl;
    return false;
  }

  //
  // clear out the fields
  //
  _handle->clearFields();



  //
  // Fire the FMQ trigger, if requested. It may be desired to delay
  // before doing this.
  //
  if ( _params->fire_fmq_trigger ){
    time_t nowcastFrequency = (time_t)(_params->nowcast_frequency * 60);
    size_t nowcastCount     = (size_t)(_params->nowcast_count);
    //
    // Delay before we send the trigger.
    // Register with PMU every 5 seconds if we have not
    // attached ourselves to a forked process.
    //
    for (int k=0; k < _params->sleep_before_trigger; k++){
      sleep(1);
      if ( k % 5 == 0){
	PMU_auto_register("Sleeping before firing trigger.");
      }
    }
    //
    // Send the trigger. Again, if this fails it is a sevre error,
    // and the process should exit immediately.
    //
    if (_nowcastQueue.fireTrigger( "MdvMerge", _outMasterHdr.time_centroid,
				   nowcastCount, nowcastFrequency )){
      cerr << "Failed to fire nowcast trigger!" << endl;
      exit(-1);
    }
  }

  return true;
}


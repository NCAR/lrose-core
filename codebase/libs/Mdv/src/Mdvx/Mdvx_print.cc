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
//////////////////////////////////////////////////////////
// Mdvx_print.cc
//
// Print routines for Mdvx class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1999
//
//////////////////////////////////////////////////////////

#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/MdvxRadar.hh>
#include <Mdv/MdvxTimeStamp.hh>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <iomanip>
using namespace std;

#ifndef BOOL_STR
#define BOOL_STR(a) (a == FALSE ? "false" : "true")
#endif

///////////////////////////////////////////////////////
// printFormats()
//
// print state of current, read and write formats

void Mdvx::printFormats(ostream &out, bool force /* = false */) const

{

  if (!force) {
    if (_internalFormat == FORMAT_MDV &&
        _readFormat == FORMAT_MDV &&
        _writeFormat == FORMAT_MDV) {
      return;
    }
  }

  out << endl;
  out << "-------------------------------" << endl;
  out << "MDV format states" << endl;
  out << "  Internal format: " << format2Str(_internalFormat) << endl;
  out << "  Read     format: " << format2Str(_readFormat) << endl;
  out << "  Write    format: " << format2Str(_writeFormat) << endl;
  out << "-------------------------------" << endl;
  out << endl;

}

  
///////////////////////////////////////////////////////
// printAllHeaders()
//
// print all headers in object

void Mdvx::printAllHeaders(ostream &out) const

{

  printMasterHeader(out);

  for (size_t i = 0; i < _fields.size(); i++) {
    const Mdvx::field_header_t &fhdr = _fields[i]->getFieldHeader();
    printFieldHeader(fhdr, out);
  }
  
  for (size_t i = 0; i < _fields.size(); i++) {
    const Mdvx::field_header_t &fhdr = _fields[i]->getFieldHeader();
    const Mdvx::vlevel_header_t &vhdr = _fields[i]->getVlevelHeader();
    printVlevelHeader(vhdr, fhdr.nz, fhdr.field_name, out);
  }
  
  for (size_t i = 0; i < _chunks.size(); i++) {
    const Mdvx::chunk_header_t &chdr = _chunks[i]->getHeader();
    printChunkHeader(chdr, out);
  }

}

///////////////////////////////////////////////////////
// printAllFileHeaders()
//
// print all file headers in object

void Mdvx::printAllFileHeaders(ostream &out) const

{

  if (_mhdrFile.struct_id != MASTER_HEAD_MAGIC_COOKIE_64) {
    out << "==>> File master header not yet read" << endl;
  } else {
    printMasterHeader(_mhdrFile, out);
  }

  for (size_t i = 0; i < _fhdrsFile.size(); i++) {
    if (_fhdrsFile[i].struct_id != FIELD_HEAD_MAGIC_COOKIE_64) {
      out << "==>> File field header not yet read, field num: " << i << endl;
    } else {
      printFieldHeader(_fhdrsFile[i], out);
    }
    if (_vhdrsFile.size() > i) {
      if (_vhdrsFile[i].struct_id != VLEVEL_HEAD_MAGIC_COOKIE_64) {
        out << "==>> File vlevel header not yet read, field num: " << i << endl;
      } else {
        printVlevelHeader(_vhdrsFile[i], _fhdrsFile[i].nz,
                          _fhdrsFile[i].field_name, out);
      }
    }
  }
  
  for (size_t i = 0; i < _chdrsFile.size(); i++) {
    if (_chdrsFile[i].struct_id != CHUNK_HEAD_MAGIC_COOKIE_64) {
      out << "==>> File chunk header not yet read, chunk num: " << i << endl;
    } else {
      printChunkHeader(_chdrsFile[i], out);
    }
  }

}

///////////////////////////////////////////////////////
// print master header in full

void Mdvx::printMasterHeader(ostream &out) const

{

  if (_internalFormat == FORMAT_NCF) {
    printNcfInfo(out);
    return;
  }

  printMasterHeader(_mhdr, out, _dataSetInfo);

}

///////////////////////////////////////////////////////
// print master header in full

void Mdvx::printMasterHeader(const master_header_t &mhdr,
			     ostream &out,
                             const string dataSetInfo /* = "" */)

{

  out << endl;
  out << "Master header" << endl;
  out << "-------------" << endl;
  out << endl;

  out << "record_len1:          " << mhdr.record_len1 << endl;
  out << "struct_id:            " << mhdr.struct_id << endl;
  out << "revision_number:      " << mhdr.revision_number << endl;
  out << endl;

  out << "time_gen:             " << _timeStr(mhdr.time_gen) << endl;
  out << "user_time:            " << _timeStr(mhdr.user_time) << endl;
  out << "time_begin:           " << _timeStr(mhdr.time_begin) << endl;
  out << "time_end:             " << _timeStr(mhdr.time_end) << endl;
  out << "time_centroid:        " << _timeStr(mhdr.time_centroid) << endl;
  if (mhdr.time_expire == 0) {
    out << "time_expire:          " << mhdr.time_expire << endl;
  } else {
    out << "time_expire:          " << _timeStr(mhdr.time_expire) << endl;
  }
  out << "time_written:         " << _timeStr(mhdr.time_written) << endl;

  out << "epoch:                " << mhdr.epoch << endl;
  out << "forecast_time:        " << _timeStr(mhdr.forecast_time) << endl;
  out << "forecast_delta:       " << mhdr.forecast_delta << endl;

  out << "num_data_times:       " << mhdr.num_data_times << endl;
  out << "index_number:         " << mhdr.index_number << endl;
  out << "data_dimension:       " << mhdr.data_dimension << endl;
  out << "data_collection_type: "
      << collectionType2Str(mhdr.data_collection_type) << endl;
  out << "user_data:            " << mhdr.user_data << endl;
  out << "native_vlevel_type:   "
      << vertType2Str(mhdr.native_vlevel_type) << endl;
  out << "vlevel_type:          "
      << vertType2Str(mhdr.vlevel_type) << endl;
  out << "vlevel_included:      "
      << BOOL_STR(mhdr.vlevel_included) << endl;
  out << "grid_orientation:     "
      << orientType2Str(mhdr.grid_orientation) << endl;
  out << "data_ordering:        "
      << orderType2Str(mhdr.data_ordering) << endl;
  out << "n_fields:             " << mhdr.n_fields << endl;
  out << "max_nx:               " << mhdr.max_nx << endl;
  out << "max_ny:               " << mhdr.max_ny << endl;
  out << "max_nz:               " << mhdr.max_nz << endl;
  out << "n_chunks:             " << mhdr.n_chunks << endl;
  out << "field_hdr_offset:     " << mhdr.field_hdr_offset << endl;
  out << "vlevel_hdr_offset:    " << mhdr.vlevel_hdr_offset << endl;
  out << "chunk_hdr_offset:     " << mhdr.chunk_hdr_offset << endl;
  out << "field_grids_differ:   "
      << BOOL_STR(mhdr.field_grids_differ) << endl;
  for (int i = 0; i < 8; i++) {
    out << "user_data_si32[" << i << "]:    "
	<< mhdr.user_data_si32[i] << endl;
  }
  out << endl;
  for (int i = 0; i < 6; i++)
    out << "user_data_fl32[" << i << "]:    "
	<< mhdr.user_data_fl32[i] << endl;
  out << "sensor_lon:           " << mhdr.sensor_lon << endl;
  out << "sensor_lat:           " << mhdr.sensor_lat << endl;
  out << "sensor_alt:           " << mhdr.sensor_alt << endl;
  out << endl;
  out << "data_set_info:" << endl;
  if (dataSetInfo.size() > 0) {
    out << dataSetInfo << endl << endl;
  } else {
    out << mhdr.data_set_info << endl << endl;
  }
  out << "data_set_name:        " << mhdr.data_set_name << endl;
  out << "data_set_source:      " << mhdr.data_set_source << endl;
  out << endl;
  out << "record_len2:          " << mhdr.record_len2 << endl;
  out << endl << endl;
  
}  
 
void Mdvx::printMasterHeader(const master_header_32_t &mhdr32,
			     ostream &out,
                             const string dataSetInfo /* = "" */)

{

  out << endl;
  out << "Master header - 32-bit" << endl;
  out << "----------------------" << endl;
  out << endl;

  out << "record_len1:          " << mhdr32.record_len1 << endl;
  out << "struct_id:            " << mhdr32.struct_id << endl;
  out << "revision_number:      " << mhdr32.revision_number << endl;
  out << endl;

  out << "time_gen:             " << _timeStr(mhdr32.time_gen) << endl;
  out << "user_time:            " << _timeStr(mhdr32.user_time) << endl;
  out << "time_begin:           " << _timeStr(mhdr32.time_begin) << endl;
  out << "time_end:             " << _timeStr(mhdr32.time_end) << endl;
  out << "time_centroid:        " << _timeStr(mhdr32.time_centroid) << endl;
  if (mhdr32.time_expire == 0) {
    out << "time_expire:          " << mhdr32.time_expire << endl;
  } else {
    out << "time_expire:          " << _timeStr(mhdr32.time_expire) << endl;
  }
  out << "time_written:         " << _timeStr(mhdr32.time_written) << endl;

  out << "epoch:                " << mhdr32.epoch << endl;
  out << "forecast_time:        " << _timeStr(mhdr32.forecast_time) << endl;
  out << "forecast_delta:       " << mhdr32.forecast_delta << endl;

  out << "num_data_times:       " << mhdr32.num_data_times << endl;
  out << "index_number:         " << mhdr32.index_number << endl;
  out << "data_dimension:       " << mhdr32.data_dimension << endl;
  out << "data_collection_type: "
      << collectionType2Str(mhdr32.data_collection_type) << endl;
  out << "user_data:            " << mhdr32.user_data << endl;
  out << "native_vlevel_type:   "
      << vertType2Str(mhdr32.native_vlevel_type) << endl;
  out << "vlevel_type:          "
      << vertType2Str(mhdr32.vlevel_type) << endl;
  out << "vlevel_included:      "
      << BOOL_STR(mhdr32.vlevel_included) << endl;
  out << "grid_orientation:     "
      << orientType2Str(mhdr32.grid_orientation) << endl;
  out << "data_ordering:        "
      << orderType2Str(mhdr32.data_ordering) << endl;
  out << "n_fields:             " << mhdr32.n_fields << endl;
  out << "max_nx:               " << mhdr32.max_nx << endl;
  out << "max_ny:               " << mhdr32.max_ny << endl;
  out << "max_nz:               " << mhdr32.max_nz << endl;
  out << "n_chunks:             " << mhdr32.n_chunks << endl;
  out << "field_hdr_offset:     " << mhdr32.field_hdr_offset << endl;
  out << "vlevel_hdr_offset:    " << mhdr32.vlevel_hdr_offset << endl;
  out << "chunk_hdr_offset:     " << mhdr32.chunk_hdr_offset << endl;
  out << "field_grids_differ:   "
      << BOOL_STR(mhdr32.field_grids_differ) << endl;
  for (int i = 0; i < 8; i++) {
    out << "user_data_si32[" << i << "]:    "
	<< mhdr32.user_data_si32[i] << endl;
  }
  out << endl;
  for (int i = 0; i < 6; i++)
    out << "user_data_fl32[" << i << "]:    "
	<< mhdr32.user_data_fl32[i] << endl;
  out << "sensor_lon:           " << mhdr32.sensor_lon << endl;
  out << "sensor_lat:           " << mhdr32.sensor_lat << endl;
  out << "sensor_alt:           " << mhdr32.sensor_alt << endl;
  out << endl;
  out << "data_set_info:" << endl;
  if (dataSetInfo.size() > 0) {
    out << dataSetInfo << endl << endl;
  } else {
    out << mhdr32.data_set_info << endl << endl;
  }
  out << "data_set_name:        " << mhdr32.data_set_name << endl;
  out << "data_set_source:      " << mhdr32.data_set_source << endl;
  out << endl;
  out << "record_len2:          " << mhdr32.record_len2 << endl;
  out << endl << endl;
  
}  
 
///////////////////////////////////////////////////////
// print master header in summary

void Mdvx::printMasterHeaderSummary(const master_header_t &mhdr,
				    ostream &out)

{

  out << endl;
  out << "  Master header summary" << endl;
  out << "  ---------------------" << endl;
  out << endl;
  out << "  data_set_name:        " << mhdr.data_set_name << endl;
  out << "  time_gen:             " << _timeStr(mhdr.time_gen) << endl;
  out << "  time_centroid:        " << _timeStr(mhdr.time_centroid) << endl;
  out << "  n_fields:             " << mhdr.n_fields << endl;
  out << endl;

}  
 
///////////////////////////////////////////////////////
// print field header in full

void Mdvx::printFieldHeader(const field_header_t &fhdr,
			    ostream &out)

{
  
  out << endl;
  out << "Field header" << endl;
  out << "------------" << endl;
  out << endl;
  out << "field_name_long:        " << fhdr.field_name_long << endl;
  out << "field_name:             " << fhdr.field_name << endl;
  out << "units:                  " << fhdr.units << endl;
  out << "transform:              " << fhdr.transform << endl;
  out << endl;
  out << "record_len1:            " << fhdr.record_len1 << endl;
  out << "struct_id:              " << fhdr.struct_id << endl;
  out << endl;
  out << "field_code:             " << fhdr.field_code << endl;
  out << "user_time1:             " << _timeStr(fhdr.user_time1) << endl;
  out << "forecast_delta:         " << fhdr.forecast_delta << endl;
  out << "user_time2:             " << _timeStr(fhdr.user_time2) << endl;
  out << "user_time3:             " << _timeStr(fhdr.user_time3) << endl;
  out << "forecast_time:          " << _timeStr(fhdr.forecast_time) << endl;
  out << "user_time4:             " << _timeStr(fhdr.user_time4) << endl;
  out << "nx:                     " << fhdr.nx << endl;
  out << "ny:                     " << fhdr.ny << endl;
  out << "nz:                     " << fhdr.nz << endl;
  out << "proj_type:              "
      << projType2Str(fhdr.proj_type) << endl;
  out << "encoding_type:          "
      << encodingType2Str(fhdr.encoding_type) << endl;
  out << "data_element_nbytes:    " << fhdr.data_element_nbytes << endl;
  out << "field_data_offset:      " << fhdr.field_data_offset << endl;
  out << "volume_size:            " << fhdr.volume_size << endl;
  for (int i = 0; i < 10; i++) {
    out << "user_data_si32[" << i << "]:      "
	<< fhdr.user_data_si32[i] << endl;
  }
  out << "compression_type:       "
      << compressionType2Str(fhdr.compression_type) << endl;
  out << "requested_compression:  "
      << compressionType2Str(fhdr.requested_compression) << endl;
  out << "transform_type:         "
      << transformType2Str(fhdr.transform_type) << endl;
  if (fhdr.encoding_type == ENCODING_FLOAT32 ||
      fhdr.encoding_type == ENCODING_RGBA32) {
    out << "scaling_type:           not applicable" << endl;
  } else {
    out << "scaling_type:           "
	<< scalingType2Str(fhdr.scaling_type) << endl;
  }
  out << endl;

  out << "native_vlevel_type:     "
      << vertType2Str(fhdr.native_vlevel_type) << endl;
  out << "vlevel_type:            "
      << vertType2Str(fhdr.vlevel_type) << endl;
  out << "dz_constant:            " << fhdr.dz_constant << endl;
  out << "data_dimension:         " << fhdr.data_dimension << endl;
  out << endl;

  out << "zoom_clipped:           " << fhdr.zoom_clipped << endl;
  out << "zoom_no_overlap:        " << fhdr.zoom_no_overlap << endl;
  out << endl;

  out << "proj_origin_lon:        " << fhdr.proj_origin_lon << endl;
  out << "proj_origin_lat:        " << fhdr.proj_origin_lat << endl;
  out << "proj_rotation:          " << fhdr.proj_rotation << endl;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; i++) {
    if (i < 10) {
      out << "proj_param[" << i << "]:          "
          << fhdr.proj_param[i] << endl;
    } else {
      out << "proj_param[" << i << "]:         "
          << fhdr.proj_param[i] << endl;
    }
  }
  out << "proj4_str:              " << fhdr.proj4_str << endl;
  out << "vert_reference:         " << fhdr.vert_reference << endl;
  out << endl;

  streamsize original_precision = out.precision();
  out.precision(8);
  out << "grid_dx:                " << fhdr.grid_dx << endl;
  out << "grid_dy:                " << fhdr.grid_dy << endl;
  out << "grid_dz:                " << fhdr.grid_dz << endl;
  out << "grid_minx:              " << fhdr.grid_minx << endl;
  out << "grid_miny:              " << fhdr.grid_miny << endl;
  out << "grid_minz:              " << fhdr.grid_minz << endl;
  out.precision(original_precision);
  
  out << "scale:                  " << fhdr.scale << endl;
  out << "bias:                   " << fhdr.bias << endl;
  out << "bad_data_value:         " << fhdr.bad_data_value << endl;
  out << "missing_data_value:     "
      << fhdr.missing_data_value << endl;
  out << "proj_rotation:          " << fhdr.proj_rotation << endl;
  for (int i = 0; i < 4; i++) {
    out << "user_data_fl32[" << i << "]:      "
	<< fhdr.user_data_fl32[i] << endl;
  }
  out << "min_value:              " << fhdr.min_value << endl;
  out << "max_value:              " << fhdr.max_value << endl;
  out << "min_value_orig_vol:     " << fhdr.min_value_orig_vol << endl;
  out << "max_value_orig_vol:     " << fhdr.max_value_orig_vol << endl;
  out << "record_len2:            " << fhdr.record_len2 << endl;

  out << endl << endl;
   
}

void Mdvx::printFieldHeader(const field_header_32_t &fhdr32,
			    ostream &out)

{
  
  out << endl;
  out << "Field header - 32-bit" << endl;
  out << "---------------------" << endl;
  out << endl;
  out << "field_name_long:        " << fhdr32.field_name_long << endl;
  out << "field_name:             " << fhdr32.field_name << endl;
  out << "units:                  " << fhdr32.units << endl;
  out << "transform:              " << fhdr32.transform << endl;
  out << endl;
  out << "record_len1:            " << fhdr32.record_len1 << endl;
  out << "struct_id:              " << fhdr32.struct_id << endl;
  out << endl;
  out << "field_code:             " << fhdr32.field_code << endl;
  out << "user_time1:             " << _timeStr(fhdr32.user_time1) << endl;
  out << "forecast_delta:         " << fhdr32.forecast_delta << endl;
  out << "user_time2:             " << _timeStr(fhdr32.user_time2) << endl;
  out << "user_time3:             " << _timeStr(fhdr32.user_time3) << endl;
  out << "forecast_time:          " << _timeStr(fhdr32.forecast_time) << endl;
  out << "user_time4:             " << _timeStr(fhdr32.user_time4) << endl;
  out << "nx:                     " << fhdr32.nx << endl;
  out << "ny:                     " << fhdr32.ny << endl;
  out << "nz:                     " << fhdr32.nz << endl;
  out << "proj_type:              "
      << projType2Str(fhdr32.proj_type) << endl;
  out << "encoding_type:          "
      << encodingType2Str(fhdr32.encoding_type) << endl;
  out << "data_element_nbytes:    " << fhdr32.data_element_nbytes << endl;
  out << "field_data_offset:      " << fhdr32.field_data_offset << endl;
  out << "volume_size:            " << fhdr32.volume_size << endl;
  for (int i = 0; i < 10; i++) {
    out << "user_data_si32[" << i << "]:      "
	<< fhdr32.user_data_si32[i] << endl;
  }
  out << "compression_type:       "
      << compressionType2Str(fhdr32.compression_type) << endl;
  out << "requested_compression:  "
      << compressionType2Str(fhdr32.requested_compression) << endl;
  out << "transform_type:         "
      << transformType2Str(fhdr32.transform_type) << endl;
  if (fhdr32.encoding_type == ENCODING_FLOAT32 ||
      fhdr32.encoding_type == ENCODING_RGBA32) {
    out << "scaling_type:           not applicable" << endl;
  } else {
    out << "scaling_type:           "
	<< scalingType2Str(fhdr32.scaling_type) << endl;
  }
  out << endl;

  out << "native_vlevel_type:     "
      << vertType2Str(fhdr32.native_vlevel_type) << endl;
  out << "vlevel_type:            "
      << vertType2Str(fhdr32.vlevel_type) << endl;
  out << "dz_constant:            " << fhdr32.dz_constant << endl;
  out << "data_dimension:         " << fhdr32.data_dimension << endl;
  out << endl;

  out << "zoom_clipped:           " << fhdr32.zoom_clipped << endl;
  out << "zoom_no_overlap:        " << fhdr32.zoom_no_overlap << endl;
  out << endl;

  out << "proj_origin_lon:        " << fhdr32.proj_origin_lon << endl;
  out << "proj_origin_lat:        " << fhdr32.proj_origin_lat << endl;
  out << "proj_rotation:          " << fhdr32.proj_rotation << endl;
  for (int i = 0; i < MDV32_MAX_PROJ_PARAMS; i++) {
    out << "proj_param[" << i << "]:          "
	<< fhdr32.proj_param[i] << endl;
  }
  out << "vert_reference:         " << fhdr32.vert_reference << endl;
  out << endl;
  out << "grid_dx:                " << fhdr32.grid_dx << endl;
  out << "grid_dy:                " << fhdr32.grid_dy << endl;
  out << "grid_dz:                " << fhdr32.grid_dz << endl;
  out << "grid_minx:              " << fhdr32.grid_minx << endl;
  out << "grid_miny:              " << fhdr32.grid_miny << endl;
  out << "grid_minz:              " << fhdr32.grid_minz << endl;
  out << "scale:                  " << fhdr32.scale << endl;
  out << "bias:                   " << fhdr32.bias << endl;
  out << "bad_data_value:         " << fhdr32.bad_data_value << endl;
  out << "missing_data_value:     "
      << fhdr32.missing_data_value << endl;
  out << "proj_rotation:          " << fhdr32.proj_rotation << endl;
  for (int i = 0; i < 4; i++) {
    out << "user_data_fl32[" << i << "]:      "
	<< fhdr32.user_data_fl32[i] << endl;
  }
  out << "min_value:              " << fhdr32.min_value << endl;
  out << "max_value:              " << fhdr32.max_value << endl;
  out << "min_value_orig_vol:     " << fhdr32.min_value_orig_vol << endl;
  out << "max_value_orig_vol:     " << fhdr32.max_value_orig_vol << endl;
  out << "record_len2:            " << fhdr32.record_len2 << endl;

  out << endl << endl;
   
}

//////////////////////////////
// print field header summary

void Mdvx::printFieldHeaderSummary(const field_header_t &fhdr,
				   ostream &out)

{
  
  out << endl;
  out << "  Field header summary - field: " << fhdr.field_name << endl;
  out << "  -----------------------------" << endl;
  out << endl;
 
  out << "  Field Name (long):       " << fhdr.field_name_long << endl;
  out << "  Units:                   " << fhdr.units << endl;
  
  out << "  Encoding Type:           "
      << encodingType2Str(fhdr.encoding_type) << endl;

  out << "  Forecast Delta:          " << fhdr.forecast_delta << endl;
  out << "  Forecast Time:           " << _timeStr(fhdr.forecast_time) << endl;
 
  out << "  (Nx,Ny,Nz):              "
      << fhdr.nx << ", " << fhdr.ny << ", " << fhdr.nz << endl;
 
  out << "  Grid Spacing (dx,dy,dz): "
      << fhdr.grid_dx << ", " << fhdr.grid_dy << ", " << fhdr.grid_dz << endl;
 
  out << "  Grid (minx,miny,minz):   "
      << fhdr.grid_minx << ", " << fhdr.grid_miny << ", "
      << fhdr.grid_minz << endl;
  
  out << "  Origin (Long,Lat):       "
      << fhdr.proj_origin_lon << ", " << fhdr.proj_origin_lat << endl;
  
  out << "  Projection:              " 
      << projType2Str(fhdr.proj_type) << endl;
  
  out << "  proj4_str:              " << fhdr.proj4_str << endl;

  out << "  Scale, bias:             "
      << fhdr.scale << ", " << fhdr.bias << endl;

  out << endl;

}

//////////////////////////////////////////////////////////
// print vlevel header

void Mdvx::printVlevelHeader(const vlevel_header_t &vhdr,
			     const int nz,
			     const char *field_name,
			     ostream &out)

{ 

  out << "Vlevel_header for field: " << field_name << endl;
  out << "------------------------" << endl;
  out << "64-bit version" << endl;
  out << "record_len1:             " << vhdr.record_len1 << endl;
  out << "struct_id:               " << vhdr.struct_id << endl;
  out << "vlevel type              "
      << vertType2Str(vhdr.type[0]) << endl;

  for (int i = 0; i < nz; i++) {
    out << "vlevel[" << setw(3) << i << "]:             "
	<< vhdr.level[i] << endl;
  } // i

  out << "record_len2:             " << vhdr.record_len2;
  out << endl;
  out << endl;

}
 
void Mdvx::printVlevelHeader(const vlevel_header_32_t &vhdr32,
			     const int nz,
			     const char *field_name,
			     ostream &out)

{ 

  out << "Vlevel_header for field: " << field_name << endl;
  out << "------------------------" << endl;
  out << "32-bit version" << endl;
  out << "record_len1:             " << vhdr32.record_len1 << endl;
  out << "struct_id:               " << vhdr32.struct_id << endl;
  out << "vlevel type              "
      << vertType2Str(vhdr32.type[0]) << endl;

  for (int i = 0; i < nz; i++) {
    out << "vlevel[" << setw(3) << i << "]:             "
	<< vhdr32.level[i] << endl;
  } // i

  out << "record_len2:             " << vhdr32.record_len2;
  out << endl;
  out << endl;

}
 
//////////////////////////////////////////////////////////
// print chunk header

void Mdvx::printChunkHeader(const chunk_header_t &chdr,
			    ostream &out)

{

  out << endl;
  out << "Chunk header" << endl;
  out << "------------" << endl;
  out << "record_len1:           " << chdr.record_len1 << endl;
  out << "struct_id:             " << chdr.struct_id << endl;
  out << "chunk_id:              " << chdr.chunk_id << endl;
  out << "                       " << chunkId2Str(chdr.chunk_id) << endl;
  out << "chunk_data_offset:     " << chdr.chunk_data_offset << endl;
  out << "size:                  " << chdr.size << endl;
  out << "info:                  " << chdr.info << endl;
  out << "record_len2:           " << chdr.record_len2 << endl;
  out << endl;

}

void Mdvx::printChunkHeader(const chunk_header_32_t &chdr32,
			    ostream &out)

{

  out << endl;
  out << "Chunk header - 32-bit" << endl;
  out << "---------------------" << endl;
  out << "record_len1:           " << chdr32.record_len1 << endl;
  out << "struct_id:             " << chdr32.struct_id << endl;
  out << "chunk_id:              " << chdr32.chunk_id << endl;
  out << "                       " << chunkId2Str(chdr32.chunk_id) << endl;
  out << "chunk_data_offset:     " << chdr32.chunk_data_offset << endl;
  out << "size:                  " << chdr32.size << endl;
  out << "info:                  " << chdr32.info << endl;
  out << "record_len2:           " << chdr32.record_len2 << endl;
  out << endl;

}

/////////////////////////////////////////////////
// print the chunks which Mdvx recognizes

void Mdvx::printChunks(ostream &out) const

{

  if (_internalFormat == FORMAT_NCF) {
    return;
  }

  // radar-specific chunks
  
  MdvxRadar radar;
  if (radar.loadFromMdvx(*this) == 0) {
    radar.print(out);
  }

  // chunks
  
  for (int ii = 0; ii < (int) _chunks.size(); ii++) {
    
    const MdvxChunk &chunk = *_chunks[ii];
    chunk.printHeader(out);

    int chunkId = chunk.getId();

    switch (chunkId) {

      case CHUNK_COMMENT: {

        cout << (char *) chunk.getData() << endl;

      } break;

      case CHUNK_NOWCAST_DATA_TIMES: {

        MdvxTimeStamp tstamp(chunk);
        tstamp.print(cout);

      } break;

      case CHUNK_TEXT_DATA: {

        cout << (char *) chunk.getData() << endl;

      } break;

      case CHUNK_VSECT_WAY_PTS_32: {

        MemBuf buf;
        buf.add(chunk.getData(), chunk.getSize());
        printVsectWayPtsBuf32(buf, out);

      } break;
      
      case CHUNK_VSECT_WAY_PTS_64: {
        
        MemBuf buf;
        buf.add(chunk.getData(), chunk.getSize());
        printVsectWayPtsBuf64(buf, out);

      } break;
      
      case CHUNK_VSECT_SAMPLE_PTS_32: {

        MemBuf buf;
        buf.add(chunk.getData(), chunk.getSize());
        printVsectSamplePtsBuf32(buf, out);

      } break;

      case CHUNK_VSECT_SAMPLE_PTS_64: {

        MemBuf buf;
        buf.add(chunk.getData(), chunk.getSize());
        printVsectSamplePtsBuf64(buf, out);

      } break;

      case CHUNK_VSECT_SEGMENTS_32: {

        MemBuf buf;
        buf.add(chunk.getData(), chunk.getSize());
        printVsectSegmentsBuf32(buf, out);

      } break;

      case CHUNK_VSECT_SEGMENTS_64: {

        MemBuf buf;
        buf.add(chunk.getData(), chunk.getSize());
        printVsectSegmentsBuf64(buf, out);

      } break;

      default: {}

    } // switch
    
  } // ii

}

////////////////////////////////////////////////
// return string representation of data format

string Mdvx::format2Str(const int format)

{

  switch(format)  {
    case FORMAT_XML:
      return "FORMAT_XML"; 
    case FORMAT_NCF:
      return "FORMAT_NCF"; 
    case FORMAT_RADX:
      return "FORMAT_RADX"; 
    default:
      return "FORMAT_MDV"; 
  }

}

////////////////////////////////////////////////
// return enum representation of data format

Mdvx::mdv_format_t Mdvx::str2Format(const string &format)

{
  
  if (format.compare("FORMAT_XML") == 0) {
    return FORMAT_XML;
  } else if (format.compare("FORMAT_NCF") == 0) {
    return FORMAT_NCF;
  } else if (format.compare("FORMAT_RADX") == 0) {
    return FORMAT_RADX;
  } else {
    return FORMAT_MDV;
  }

}

////////////////////////////////////////////////
// return string representation of proj type

const char *Mdvx::projType2Str(const int proj_type)

{

  switch(proj_type)  {
  case PROJ_NATIVE:
    return("Native"); 
  case PROJ_LATLON:
    return("Latitude/Longitude Grid (units in degrees)"); 
  case PROJ_ARTCC:
    return("ARTCC"); 
  case PROJ_LAMBERT_CONF:
    return("Lambert Conformal"); 
  case PROJ_MERCATOR:
    return("Mercator"); 
  case PROJ_POLAR_STEREO:
    return("Polar Stereographic"); 
  case PROJ_POLAR_ST_ELLIP:
    return("Polar Stereographic Equidistant"); 
  case PROJ_CYL_EQUIDIST:
    return("Cylindrical Equidistant"); 
  case PROJ_FLAT:
    return("Flat (Cartesian) (units in KM)"); 
  case PROJ_POLAR_RADAR:
    return("Polar Radar"); 
  case PROJ_RADIAL:
    return("Radial"); 
  case PROJ_VSECTION:
    return("Vert section"); 
  case PROJ_OBLIQUE_STEREO:
    return("Oblique Stereographic"); 
  case PROJ_RHI_RADAR:
    return("RHI Radar"); 
  case PROJ_TIME_HEIGHT:
    return("Time-height profile"); 
  case PROJ_TRANS_MERCATOR:
    return("Transverse Mercator"); 
  case PROJ_ALBERS:
    return("Albers Equal Area"); 
  case PROJ_LAMBERT_AZIM:
    return("Lambert Azimuthal Equal Area"); 
  case PROJ_VERT_PERSP:
    return("Vertical Perspective (sat view)"); 
  default:
    return (_labelledInt("Unknown Projection Type", proj_type));
  }

}

////////////////////////////////////////////////
// return string representation of vertical type

const char *Mdvx::vertType2Str(const int vert_type)

{

  switch(vert_type)  {
  case VERT_TYPE_UNKNOWN: 
    return("Unknown"); 
  case VERT_TYPE_SURFACE: 
    return("Surface"); 
  case VERT_TYPE_SIGMA_P:
    return("Sigma P"); 
  case VERT_TYPE_PRESSURE:
    return("Pressure (units mb)"); 
  case VERT_TYPE_Z:
    return("Constant Altitude (units KM MSL)"); 
  case VERT_TYPE_SIGMA_Z:
    return("Sigma Z"); 
  case VERT_TYPE_ETA:
    return("ETA"); 
  case VERT_TYPE_THETA:
    return("Theta"); 
  case VERT_TYPE_MIXED:
    return("Mixed"); 
  case VERT_TYPE_ELEV:
    return("Elevation angles - radar"); 
  case VERT_TYPE_COMPOSITE:
    return("Composite"); 
  case VERT_TYPE_CROSS_SEC:
    return("Cross Secional View"); 
  case VERT_SATELLITE_IMAGE:
    return("Satellite Image"); 
  case VERT_VARIABLE_ELEV:
    return("Variable elevation scan"); 
  case VERT_FIELDS_VAR_ELEV:
    return("Field specifc Var. elev. scan"); 
  case VERT_FLIGHT_LEVEL:
    return("Flight level"); 
  case VERT_EARTH_IMAGE:
    return("Earth Conformal Image"); 
  case VERT_TYPE_AZ:
    return("Azimuth angles - radar RHI"); 
  case VERT_TYPE_TOPS:
    return("Tops"); 
  case VERT_TYPE_ZAGL_FT:
    return("ZAgl_ft"); 
  case VERT_TYPE_VARIABLE: 
    return("Variable"); 
  case VERT_TYPE_WRF_ETA: 
    return("WRF Eta"); 
  default:
    return (_labelledInt("Unknown Vertical Type", vert_type));
  }
}  
 
////////////////////////////////////////////////
// return string for units for x coordinate
// based on projection

const char *Mdvx::projType2XUnits(const int proj_type)

{

  switch(proj_type)  {
  case PROJ_LATLON:
    return("deg"); 
  case PROJ_ARTCC:
  case PROJ_LAMBERT_CONF:
  case PROJ_MERCATOR:
  case PROJ_POLAR_STEREO:
  case PROJ_POLAR_ST_ELLIP:
  case PROJ_CYL_EQUIDIST:
  case PROJ_FLAT:
  case PROJ_ALBERS:
  case PROJ_LAMBERT_AZIM:
    return("km"); 
  case PROJ_POLAR_RADAR:
  case PROJ_RHI_RADAR:
    return("km"); 
  case PROJ_RADIAL:
    return("m"); 
  case PROJ_NATIVE:
    return("");
  case PROJ_TIME_HEIGHT:
    return("sec");
  default:
    return("km"); 
  }

}

////////////////////////////////////////////////
// return string for units for y coordinate
// based on projection

const char *Mdvx::projType2YUnits(const int proj_type)

{

  switch(proj_type)  {
  case PROJ_LATLON:
    return("deg"); 
  case PROJ_ARTCC:
    //   case PROJ_STEREOGRAPHIC:
  case PROJ_LAMBERT_CONF:
  case PROJ_MERCATOR:
  case PROJ_POLAR_STEREO:
  case PROJ_POLAR_ST_ELLIP:
  case PROJ_CYL_EQUIDIST:
  case PROJ_FLAT:
    return("km"); 
  case PROJ_POLAR_RADAR:
  case PROJ_RHI_RADAR:
    return("deg"); 
  case PROJ_RADIAL:
    return("deg"); 
  case PROJ_NATIVE:
    return("");
  case PROJ_TIME_HEIGHT:
    return("");
  default:
    return("km"); 
  }

}

////////////////////////////////////////////////
// return string for units for z coordinate
// based on vlevel type

const char *Mdvx::vertTypeZUnits(const int vert_type)

{

  switch(vert_type)  {
  case VERT_TYPE_SURFACE: 
    return(""); 
  case VERT_TYPE_SIGMA_P:
    return("SigmaP"); 
  case VERT_TYPE_PRESSURE:
    return("mb"); 
  case VERT_TYPE_Z:
    return("km"); 
  case VERT_TYPE_SIGMA_Z:
    return("SigmaZ"); 
  case VERT_TYPE_ETA:
    return("ETA"); 
  case VERT_TYPE_THETA:
    return("Theta"); 
  case VERT_TYPE_MIXED:
    return("Mixed"); 
  case VERT_TYPE_ELEV:
  case VERT_TYPE_AZ:
    return("deg"); 
  case VERT_TYPE_TOPS:
    return("km"); 
  case VERT_TYPE_COMPOSITE:
    return(""); 
  case VERT_TYPE_CROSS_SEC:
    return(""); 
  case VERT_SATELLITE_IMAGE:
    return(""); 
  case VERT_VARIABLE_ELEV:
    return("deg"); 
  case VERT_FIELDS_VAR_ELEV:
    return("deg"); 
  case VERT_FLIGHT_LEVEL:
    return("FL"); 
  case VERT_TYPE_ZAGL_FT:
    return("ft"); 
  case VERT_TYPE_WRF_ETA:
    return("Eta"); 
  default:
    return(""); 
  }
}  
 
////////////////////////////////////////////////
// return string representation of encoding type

const char *Mdvx::encodingType2Str(const int encoding_type)

{
  switch(encoding_type)  {
  case ENCODING_ASIS:
    return("ENCODING_ASIS"); 
  case ENCODING_INT8:
    return("ENCODING_INT8 (CHAR/BYTE)"); 
  case ENCODING_INT16:
    return("ENCODING_INT16 (SHORT)"); 
  case ENCODING_FLOAT32:
    return("ENCODING_FLOAT32 (FLOAT)"); 
  case ENCODING_RGBA32:
    return("ENCODING_RGBA32 (IMAGE)"); 
  case PLANE_RLE8:
    return("PLANE_RLE8 - run-length encoded plane"); 
  default:
    return (_labelledInt("Unknown Encoding Type", encoding_type));
  }
}  
 
///////////////////////////////////////////////////
// return string representation of collection type

const char *Mdvx::collectionType2Str(const int collection_type)

{

  switch(collection_type)  {
  case DATA_MEASURED:
    return("Measured");
  case DATA_EXTRAPOLATED:
    return("Extrapolated");
  case DATA_FORECAST:
    return("Forecast");
  case DATA_SYNTHESIS:
    return("Synthesis");
  case DATA_MIXED:
    return("Mixed");
  case DATA_IMAGE:
    return("RGBA Image");
  case DATA_GRAPHIC:
    return("RGBA Graphic");
  case DATA_CLIMO_ANA:
    return("Climotography from Model Analisys");
  case DATA_CLIMO_OBS:
    return("Climotography from Observational Data");
  default:
    return (_labelledInt("Unknown Collection Type", collection_type));
  }

}

///////////////////////////////////////////////////
// return string representation of orientation type

const char *Mdvx::orientType2Str(const int orient_type)

{

  switch(orient_type) {
  case ORIENT_OTHER:
    return("ORIENT_OTHER");
  case ORIENT_SN_WE:
    return("ORIENT_SN_WE");
  case ORIENT_NS_WE:
    return("ORIENT_NS_WE");
  case ORIENT_SN_EW:
    return("ORIENT_SN_EW");
  case ORIENT_NS_EW:
    return("ORIENT_NS_EW");
  default:
    return (_labelledInt("Unknown Orientation", orient_type));
  }

}

///////////////////////////////////////////////////
// return string representation of data order

const char *Mdvx::orderType2Str(const int order_type)

{

  switch(order_type) {

  case ORDER_XYZ:
    return("ORDER_XYZ");
  case ORDER_YXZ:
    return("ORDER_YXZ");
  case ORDER_XZY:
    return("ORDER_XZY");
  case ORDER_YZX:
    return("ORDER_YZX");
  case ORDER_ZXY:
    return("ORDER_ZXY");
  case ORDER_ZYX:
    return("ORDER_ZYX");
  default:
    return (_labelledInt("Unknown Data Order", order_type));
  }

}


///////////////////////////////////////////////////
// return string representation of data compression

const char *Mdvx::compressionType2Str(const int compression_type)

{

  switch(compression_type) {
    
  case COMPRESSION_ASIS:
    return("COMPRESSION_ASIS");
  case COMPRESSION_NONE:
    return("COMPRESSION_NONE");
  case COMPRESSION_RLE:
    return("COMPRESSION_RLE");
  case COMPRESSION_LZO:
    return("COMPRESSION_LZO");
  case COMPRESSION_ZLIB:
    return("COMPRESSION_ZLIB");
  case COMPRESSION_BZIP:
    return("COMPRESSION_BZIP");
  case COMPRESSION_GZIP:
    return("COMPRESSION_GZIP");
  case COMPRESSION_GZIP_VOL:
    return("COMPRESSION_GZIP_VOL");
  default:
    return (_labelledInt("Unknown compression type", compression_type));
  }

}


///////////////////////////////////////////////////
// return string representation of data transform

const char *Mdvx::transformType2Str(const int transform_type)

{

  switch(transform_type) {

  case DATA_TRANSFORM_NONE:
    return("DATA_TRANSFORM_NONE - Depicts an area or volume in space");
  case DATA_TRANSFORM_LOG:
    return("DATA_TRANSFORM_LOG - Natural log of value");
  case DATA_TRANSFORM_POINT:
    return("DATA_TRANSFORM_POINT - Depicts a point in space.");
  case DATA_TRANSFORM_SUM:
    return("DATA_TRANSFORM_SUM - Sum of values");
  case DATA_TRANSFORM_DIFF:
    return("DATA_TRANSFORM_DIFF - Difference Of values ");
  case DATA_TRANSFORM_PROD:
    return("DATA_TRANSFORM_PROD - Product Of values");
  case DATA_TRANSFORM_MAXIMUM:
    return("DATA_TRANSFORM_MAXIMUM - Maximum Of values");
  case DATA_TRANSFORM_MINIMUM:
    return("DATA_TRANSFORM_MINIMUM - Minimum Of values");
  case DATA_TRANSFORM_MEAN:
    return("DATA_TRANSFORM_MEAN - (Average)");
  case DATA_TRANSFORM_MEDIAN:
    return("DATA_TRANSFORM_MEDIAN");
  case DATA_TRANSFORM_MODE:
    return("DATA_TRANSFORM_MODE");
  case DATA_TRANSFORM_MID_RANGE:
    return("DATA_TRANSFORM_MID_RANGE - Average of maximum and minimum");
  case DATA_TRANSFORM_STDDEV:
    return("DATA_TRANSFORM_STDDEV");
  case DATA_TRANSFORM_VAR:
    return("DATA_TRANSFORM_VAR - Variance");
  case DATA_TRANSFORM_COVAR:
    return("DATA_TRANSFORM_COVAR - Covariance");
  case DATA_TRANSFORM_NORM:
    return("DATA_TRANSFORM_NORM Normalized data");
  default:
    return (_labelledInt("Unknown transform type", transform_type));
  }

}


///////////////////////////////////////////////////
// return string representation of data scaling

const char *Mdvx::scalingType2Str(const int scaling_type)

{

  switch(scaling_type) {

  case SCALING_NONE:
    return("SCALING_NONE");
  case SCALING_ROUNDED:
    return("SCALING_ROUNDED");
  case SCALING_DYNAMIC:
    return("SCALING_DYNAMIC");
  case SCALING_INTEGRAL:
    return("SCALING_INTEGRAL");
  case SCALING_SPECIFIED:
    return("SCALING_SPECIFIED");
  default:
    return (_labelledInt("Unknown scaling type", scaling_type));
  }

}

///////////////////////////////////////////////////
// return string representation of chunk ID

const char *Mdvx::chunkId2Str(const int chunk_id)

{

  switch (chunk_id) {
  case CHUNK_DOBSON_VOL_PARAMS:
    return ("CHUNK_DOBSON_VOL_PARAMS");
    break;
  case CHUNK_DOBSON_ELEVATIONS:
    return ("CHUNK_DOBSON_ELEVATIONS");
    break;
  case CHUNK_NOWCAST_DATA_TIMES:
    return ("CHUNK_NOWCAST_DATA_TIMES");
    break;
  case CHUNK_DSRADAR_PARAMS:
    return ("CHUNK_DSRADAR_PARAMS");
    break;
  case CHUNK_DSRADAR_ELEVATIONS:
    return ("CHUNK_DSRADAR_ELEVATIONS");
    break;
  case CHUNK_DSRADAR_AZIMUTHS:
    return ("CHUNK_DSRADAR_AZIMUTHS");
    break;
  case CHUNK_VARIABLE_ELEV:
    return ("CHUNK_VARIABLE_ELEV");
    break;
  case CHUNK_TEXT_DATA:
    return ("CHUNK_TEXT_DATA");
    break;
  case CHUNK_CLIMO_INFO:
    return ("CHUNK_CLIMO_INFO");
    break;
  case CHUNK_COMMENT:
    return ("CHUNK_COMMENT");
    break;
  case CHUNK_DSRADAR_CALIB:
    return ("CHUNK_DSRADAR_CALIB");
    break;
  case CHUNK_VSECT_WAY_PTS_32:
    return ("CHUNK_VSECT_WAY_PTS_32");
    break;
  case CHUNK_VSECT_WAY_PTS_64:
    return ("CHUNK_VSECT_WAY_PTS_64");
    break;
  case CHUNK_VSECT_SAMPLE_PTS_32:
    return ("CHUNK_VSECT_SAMPLE_PTS_32");
    break;
  case CHUNK_VSECT_SAMPLE_PTS_64:
    return ("CHUNK_VSECT_SAMPLE_PTS_64");
    break;
  case CHUNK_VSECT_SEGMENTS_32:
    return ("CHUNK_VSECT_SEGMENTS_32");
    break;
  case CHUNK_VSECT_SEGMENTS_64:
    return ("CHUNK_VSECT_SEGMENTS_64");
    break;
  default:
    return (_labelledInt("Unknown chunk type", chunk_id));
  }

}

/////////////////////////////
// string for rendering time

const char* Mdvx::_timeStr(const time_t ttime)

{
  if (ttime == -1 || ttime == 0) {
    return "not set";
  } else {
    return (utimstr(ttime));
  }
}

///////////////////////////////////////
// write labelled int to static string

const char *Mdvx::_labelledInt(const char*label, int num)

{

  if (strlen(label) > _printStrLen - 32) {
    STRncopy(_printStr, label, _printStrLen);
  } else {
    sprintf(_printStr, "%s: %d", label, num);
  }

  return (_printStr);

}

///////////////////////////////////////////////////
// return string representation of climatology type

const char *Mdvx::climoType2Str(const int climo_type)
{
  switch (climo_type) {
  case CLIMO_TYPE_MEAN:
    return ("CLIMO_TYPE_MEAN");
    break;
  case CLIMO_TYPE_STD_DEV:
    return ("CLIMO_TYPE_STD_DEV");
    break;
  case CLIMO_TYPE_MAX:
    return ("CLIMO_TYPE_MAX");
    break;
  case CLIMO_TYPE_MIN:
    return ("CLIMO_TYPE_MIN");
    break;
  case CLIMO_TYPE_NUM_OBS:
    return ("CLIMO_TYPE_NUM_OBS");
    break;
  case CLIMO_TYPE_NUM_OBS_GT:
    return ("CLIMO_TYPE_NUM_OBS_GT");
    break;
  case CLIMO_TYPE_NUM_OBS_GE:
    return ("CLIMO_TYPE_NUM_OBS_GE");
    break;
  case CLIMO_TYPE_NUM_OBS_LT:
    return ("CLIMO_TYPE_NUM_OBS_LT");
    break;
  case CLIMO_TYPE_NUM_OBS_LE:
    return ("CLIMO_TYPE_NUM_OBS_LE");
    break;
  default:
    return (_labelledInt("Unknown climatology type", climo_type));
  }

}

////////////////////////////////////
// print entire vol

void Mdvx::printVol(ostream &out,
                    const Mdvx *mdvx,
                    bool printFieldFileHeadersAlso /* = false*/,
                    bool printData /* = false*/,
                    bool transformToLinear /* = false*/,
                    bool printNative /* = false*/,
                    bool printCanonical /* = false*/,
                    int printNlinesData /* = -1 */)
{

  out << endl;
  out << "File path: " << mdvx->getPathInUse() << endl;

  // master header

  mdvx->printMasterHeader(out);
  if (mdvx->getDataSetInfo().size() > 510) {
    out << "======== Full DataSetInfo ==========" << endl;
    out << mdvx->getDataSetInfo() << endl;
    out << "====================================" << endl;
  }
  
  // fields
  
  for (size_t i = 0; i < mdvx->getNFields(); i++) {

    MdvxField *field = mdvx->getField(i);

    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    mdvx->printFieldHeader(fhdr, out);
    const Mdvx::vlevel_header_t &vhdr = field->getVlevelHeader();
    mdvx->printVlevelHeader(vhdr, fhdr.nz, fhdr.field_name, out);

    if (printFieldFileHeadersAlso) {
      const Mdvx::field_header_t *fhdrFile = field->getFieldHeaderFile();
      const Mdvx::vlevel_header_t *vhdrFile = field->getVlevelHeaderFile();
      if (fhdrFile && vhdrFile) {
        out << "============ Field header as in file =============" << endl;
        mdvx->printFieldHeader(*fhdrFile, out);
        out << "==================================================" << endl;
        out << "=========== Vlevel header as in file =============" << endl;
        mdvx->printVlevelHeader(*vhdrFile,
                                fhdrFile->nz, fhdrFile->field_name, out);
        out << "==================================================" << endl;
      }
    }

    MdvxProj proj(mdvx->getMasterHeader(), field->getFieldHeader());
    MdvxRadar mdvxRadar;
    if (mdvxRadar.loadFromMdvx(*mdvx) == 0) {
      DsRadarParams radar = mdvxRadar.getRadarParams();
      proj.setSensorPosn(radar.latitude, radar.longitude, radar.altitude);
    }
    proj.print(out);

    if (printData) {
      if (transformToLinear) {
	if (field->transform2Linear()){
	  cerr << field->getErrStr() << endl;
	}
      }
      field->printVoldata(out, printNative, true, true,
                          printCanonical, printNlinesData);
    }
  }

  // chunks

  mdvx->printChunks(out);

}
  
////////////////////////
// print volume summary

void Mdvx::printVolSummary(ostream &out,
                           const Mdvx *mdvx)
{
  
  out << endl;
  out << "File path: " << mdvx->getPathInUse() << endl;
  
  // master header
  
  const Mdvx::master_header_t &mhdr = mdvx->getMasterHeader();
  mdvx->printFormats(out);
  mdvx->printMasterHeaderSummary(mhdr, out);
  
  // fields
  
  for (size_t i = 0; i < mdvx->getNFields(); i++) {
    MdvxField *field = mdvx->getField(i);
    MdvxProj proj(field->getFieldHeader());
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    mdvx->printFieldHeaderSummary(fhdr, out);
  }

}
  
/////////////////////////////
// print volume in GIS format

void Mdvx::printVolGis(ostream &out,
                       const Mdvx *mdvx,
                       bool startAtTop /*= true*/)
{
  
  // some sanity checks

  if (mdvx->getNFields() < 1) {
    cerr << "ERROR - Mdvx, GIS mode" << endl;
    cerr << "  No fields found" << endl;
    return;
  }
  
  MdvxField *field = mdvx->getFieldByNum(0);
  const Mdvx::field_header_t &fhdr = field->getFieldHeader();
  MdvxProj proj(fhdr);
  
  if (mdvx->getNFields() > 1) {
    cerr << "WARNING - Mdvx, GIS mode" << endl;
    cerr << "  More than one field found" << endl;
    cerr << "  Only the first field will be output" << endl;
    cerr << "  Output field name: " << fhdr.field_name << endl;
  }
  if (fhdr.nz > 1) {
    cerr << "WARNING - Mdvx, GIS mode" << endl;
    cerr << "  More than one plane found" << endl;
    cerr << "  Only the first plane will be output" << endl;
  }
  bool cellSizesDiffer = false;
  if (fabs(fhdr.grid_dx - fhdr.grid_dy) > 1.0e-5) {
    cellSizesDiffer = true;
    cerr << "WARNING - Mdvx, GIS mode" << endl;
    cerr << "  Grid cell size differs in X and Y" << endl;
    cerr << "    dx: " << fhdr.grid_dx << endl;
    cerr << "    dy: " << fhdr.grid_dy << endl;
    cerr << "  Will output dx and dy" << endl;
  }

  // convert to floats, uncompressed, linear
  
  field->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

  if (field->transform2Linear()){
    cerr << field->getErrStr() << endl;
  }

  // ESRI header

  out << "nrows " << fhdr.ny << endl;
  out << "ncols " << fhdr.nx << endl;
  out << "xllcorner " << fhdr.grid_minx << endl;
  out << "yllcorner " << fhdr.grid_miny << endl;
  if (cellSizesDiffer) {
    out << "dx " << fhdr.grid_dx << endl;
    out << "dy " << fhdr.grid_dy << endl;
  } else {
    out << "cellsize " << fhdr.grid_dx << endl;
  }
  out << "NODATA_value " << fhdr.missing_data_value << endl;

  // print out starting at top row

  int iy_begin, iy_end, iy_increment;
  if (startAtTop) {
    iy_begin = fhdr.ny - 1;
    iy_end = -1;
    iy_increment = -1;
  } else {
    out << "Bottom row first" << endl;
    iy_begin = 0;
    iy_end = fhdr.ny;
    iy_increment = 1;
  }

  for (int iy = iy_begin; iy != iy_end; iy += iy_increment) {
    
    fl32 *data = ((fl32 *) field->getVol()) + (iy * fhdr.nx);

    for (int ix = 0; ix < fhdr.nx; ix++, data++) {
      
      out << *data;
      if (ix == fhdr.nx - 1) {
	out << endl;
      } else {
	out << " ";
      }

    } // ix

  } // iy

}
  
/////////////////////////////////
// print volume in tabular format

void Mdvx::printVolTable(ostream &out,
                         const Mdvx *mdvx)
{
  
  // some sanity checks

  if (mdvx->getNFields() < 1) {
    cerr << "ERROR - Mdvx, TABLE mode" << endl;
    cerr << "  No fields found" << endl;
    return;
  }

  const vector<MdvxField *> fields = mdvx->getFields();

  const Mdvx::field_header_t &fhdr0 = fields[0]->getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr0 = fields[0]->getVlevelHeader();

  int nxPrint = fhdr0.nx;
  int nyPrint = fhdr0.ny;
  int nzPrint = fhdr0.nz;
  for (size_t ii = 1; ii < fields.size(); ii++) {
    const Mdvx::field_header_t &fhdr = fields[ii]->getFieldHeader();
    if (fhdr.nx != fhdr0.nx || fhdr.ny != fhdr0.ny || fhdr.nz != fhdr0.nz) {
      cerr << "WARNING - Mdvx, TABLE mode" << endl;
      cerr << "  Field sizes differ" << endl;
      cerr << "  Will use smallest dimensions for printing table" << endl;
    }
    nxPrint = MIN(nxPrint, fhdr.nx);
    nyPrint = MIN(nyPrint, fhdr.ny);
    nzPrint = MIN(nzPrint, fhdr.nz);
  }
  
  // convert to floats, uncompressed, linear
  
  for (size_t ii = 0; ii < fields.size(); ii++) {
    fields[ii]->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

    if (fields[ii]->transform2Linear()){
      cerr << fields[ii]->getErrStr() << endl;
    }

    fields[ii]->setPlanePtrs();
  }

  // print headers

  const Mdvx::master_header_t &mhdr = mdvx->getMasterHeader();

  out << "# MDV file - tabular output" << endl;
  out << "# File: " << mdvx->getPathInUse() << endl;
  out << "# Time: " << DateTime::strn(mhdr.time_centroid) << endl;

  out << "# Fields: z y x";
  for (size_t ii = 0; ii < fields.size(); ii++) {
    out << " " << fields[ii]->getFieldHeader().field_name;
  }
  out << endl;
  
  out << "# Units:";
  int projType = fields[0]->getFieldHeader().proj_type;
  int vlevelType = fields[0]->getFieldHeader().vlevel_type;
  out << " " << Mdvx::vertTypeZUnits(vlevelType);
  out << " " << Mdvx::projType2YUnits(projType);
  out << " " << Mdvx::projType2XUnits(projType);
  for (size_t ii = 0; ii < fields.size(); ii++) {
    out << " " << fields[ii]->getFieldHeader().units;
  }
  out << endl;

  // print values

  double minx = fhdr0.grid_minx;
  double miny = fhdr0.grid_miny;
  double dx = fhdr0.grid_dx;
  double dy = fhdr0.grid_dy;

  for (int iz = 0; iz < nzPrint; iz++) {
    for (int iy = 0; iy < nyPrint; iy++) {
      for (int ix = 0; ix < nxPrint; ix++) {
	
	out << vhdr0.level[iz];
	out << " " << miny + iy * dy;
	out << " " << minx + ix * dx;

	for (size_t ii = 0; ii < fields.size(); ii++) {
	  fl32 *plane = (fl32 *) fields[ii]->getPlane(iz);
	  const Mdvx::field_header_t &fhdr = fields[ii]->getFieldHeader();
	  int64_t offset = iy * fhdr.nx + ix;
	  fl32 val = plane[offset];
	  out << " " << val;
	}
	out << endl;

      } // ix
    } // iy
  } // iz

}

////////////////////////
// print all headers

void Mdvx::printAllHeaders(ostream &out,
                           const Mdvx *mdvx)
{

  out << endl;
  out << "File path: " << mdvx->getPathInUse() << endl;

  // master header

  const Mdvx::master_header_t &mhdr = mdvx->getMasterHeaderFile();
  mdvx->printFormats(out);
  mdvx->printMasterHeader(mhdr, out);
  if (mdvx->getDataSetInfo().size() > 510) {
    out << "======== Full DataSetInfo ==========" << endl;
    out << mdvx->getDataSetInfo() << endl;
    out << "====================================" << endl;
  }
  
  // fields
  
  for (int i = 0; i < mhdr.n_fields; i++) {
    const Mdvx::field_header_t &fhdr = mdvx->getFieldHeaderFile(i);
    mdvx->printFieldHeader(fhdr, out);
    const Mdvx::vlevel_header_t &vhdr = mdvx->getVlevelHeaderFile(i);
    mdvx->printVlevelHeader(vhdr, fhdr.nz, fhdr.field_name, out);
    MdvxProj proj(mhdr, fhdr);
    proj.print(out);
  }

  for (int i = 0; i < mhdr.n_chunks; i++) {
    const Mdvx::chunk_header_t &chdr = mdvx->getChunkHeaderFile(i);
    mdvx->printChunkHeader(chdr, out);
  }

}
  
/////////////////////////////////////////////////
// print the time list

void Mdvx::printTimeList(ostream &out,
                         const Mdvx *mdvx,
                         MdvxTimeList::time_list_mode_t mode,
                         const string &url,
                         time_t startTime,
                         time_t endTime,
                         time_t searchTime,
                         time_t genTime,
                         int marginSecs)
  
{

  switch(mode) {
  
  case MdvxTimeList::MODE_VALID: {

    out << "TIME LIST - MODE VALID" << endl;
    out << "  URL: " << url << endl;
    out << "  Start time: "
	 << DateTime::strn(startTime) << endl;
    out << "  End time: " << DateTime::strn(endTime) << endl;

    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    bool hasForecasts = mdvx->timeListHasForecasts();
    if (valid.size() == 0) {
      out << "    No times returned." << endl;
    } else {
      if (hasForecasts) {
	out << "  Valid/gen times:" << endl;
      } else {
	out << "  Valid times:" << endl;
      }
      for (size_t ii = 0; ii < valid.size(); ii++) {
	if (hasForecasts) {
	  out << "    "
	       << DateTime::strn(valid[ii]) << " / "
	       << DateTime::strn(gen[ii]) << endl;
	} else {
	  out << "    " << DateTime::strn(valid[ii]) << endl;
	}
      } // ii
    }

    break;
  }

  case MdvxTimeList::MODE_GENERATE: {

    out << "TIME LIST - MODE GENERATE" << endl;
    out << "  URL: " << url << endl;
    out << "  Start time: "
	 << DateTime::strn(startTime) << endl;
    out << "  End time: " << DateTime::strn(endTime) << endl;

    const vector<time_t> &gen = mdvx->getGenTimes();
    if (gen.size() == 0) {
      out << "    No times returned." << endl;
    } else {
      out << "  Gen times:" << endl;
      for (size_t ii = 0; ii < gen.size(); ii++) {
	out << "    " << DateTime::strn(gen[ii]) << endl;
      } // ii
    }

    break;
  }

  case MdvxTimeList::MODE_FORECAST: {

    out << "TIME LIST - MODE FORECAST" << endl;
    out << "  URL: " << url << endl;
    out << "  Gen time: " << DateTime::strn(genTime) << endl;
    
    const vector<time_t> &valid = mdvx->getValidTimes();
    if (valid.size() == 0) {
      out << "    No times returned." << endl;
    } else {
      out << "  Forecast times:" << endl;
      for (size_t ii = 0; ii < valid.size(); ii++) {
	out << "    " << DateTime::strn(valid[ii]) << endl;
      } // ii
    }

    break;
  }

  case MdvxTimeList::MODE_GEN_PLUS_FCASTS: {

    out << "TIME LIST - MODE GEN_PLUS_FORECASTS" << endl;
    out << "  URL: " << url << endl;
    out << "  Start time: "
	 << DateTime::strn(startTime) << endl;
    out << "  End time: "
	 << DateTime::strn(endTime) << endl;

    const vector<time_t> &gen = mdvx->getGenTimes();
    const vector<vector<time_t> > &ftArray =
      mdvx->getForecastTimesArray();

    if (gen.size() == 0) {
      out << "    No times returned." << endl;
    } else {
      out << "  Gen times:" << endl;
      for (size_t ii = 0; ii < gen.size(); ii++) {
	out << "    " << DateTime::strn(gen[ii]) << endl;
      } // ii
      for (size_t ii = 0; ii < ftArray.size(); ii++) {
	out << "Gen time: " << DateTime::strn(gen[ii]) << endl;
	out << "  Forecast times:" << endl;
	if (ii < ftArray.size()) {
	  const vector<time_t> &forecastTimes = ftArray[ii];
	  for (size_t jj = 0; jj < forecastTimes.size(); jj++) {
	    out << "    "
		 << DateTime::strn(forecastTimes[jj]) << endl;
	  }
	}
      } // ii
    }
  break;
  }
  
  case MdvxTimeList::MODE_VALID_MULT_GEN: {

    out << "TIME LIST - MODE VALID_MULT_GEN" << endl;
    out << "  URL: " << url << endl;
    out << "  Start time: "
	 << DateTime::strn(startTime) << endl;
    out << "  End time: " << DateTime::strn(endTime) << endl;

    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    if (valid.size() == 0) {
      out << "    No times returned." << endl;
    } else {
      out << "  Valid/gen times:" << endl;
      for (size_t ii = 0; ii < valid.size(); ii++) {
	out << "    "
	     << DateTime::strn(valid[ii]) << " / "
	     << DateTime::strn(gen[ii]) << endl;
      } // ii
    }

    break;
  }

  case MdvxTimeList::MODE_FIRST: {
    out << "TIME LIST - MODE FIRST" << endl;
    out << "  URL: " << url << endl;
    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    bool hasForecasts = mdvx->timeListHasForecasts();
    if (valid.size() == 0) {
      out << "    No time returned." << endl;
    } else {
      if (hasForecasts) {
	out << "  First valid/gen time:" << endl;
	out << "    "
	     << DateTime::strn(valid[0]) << " / "
	     << DateTime::strn(gen[0]) << endl;
      } else {
	out << "  First time:" << endl;
	out << "    " << DateTime::strn(valid[0]) << endl;
      }
    }
    break;
  }

  case MdvxTimeList::MODE_LAST: {
    out << "TIME LIST - MODE LAST" << endl;
    out << "  URL: " << url << endl;
    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    bool hasForecasts = mdvx->timeListHasForecasts();
    if (valid.size() == 0) {
      out << "    No time returned." << endl;
    } else {
      if (hasForecasts) {
	out << "  Last valid/gen time:" << endl;
	out << "    "
	     << DateTime::strn(valid[0]) << " / "
	     << DateTime::strn(gen[0]) << endl;
      } else {
	out << "  Last time:" << endl;
	out << "    " << DateTime::strn(valid[0]) << endl;
      }
    }
    break;
  }

  case MdvxTimeList::MODE_CLOSEST: {
    out << "TIME LIST - MODE CLOSEST" << endl;
    out << "  URL: " << url << endl;
    out << "  Search time: "
	 << DateTime::strn(searchTime) << endl;
    out << "  Time margin: " << marginSecs << endl;
    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    bool hasForecasts = mdvx->timeListHasForecasts();
    if (valid.size() == 0) {
      out << "    No time returned." << endl;
    } else {
      if (hasForecasts) {
	out << "  Closest valid/gen time:" << endl;
	out << "    "
	     << DateTime::strn(valid[0]) << " / "
	     << DateTime::strn(gen[0]) << endl;
      } else {
	out << "  Closest time:" << endl;
	out << "    " << DateTime::strn(valid[0]) << endl;
      }
    }
    break;
  }

  case MdvxTimeList::MODE_FIRST_BEFORE: {
    out << "TIME LIST - MODE FIRST_BEFORE" << endl;
    out << "  URL: " << url << endl;
    out << "  Search time: "
	 << DateTime::strn(searchTime) << endl;
    out << "  Time margin: " << marginSecs << endl;
    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    bool hasForecasts = mdvx->timeListHasForecasts();
    if (valid.size() == 0) {
      out << "    No time returned." << endl;
    } else {
      if (hasForecasts) {
	out << "  First before valid/gen time:" << endl;
	out << "    "
	     << DateTime::strn(valid[0]) << " / "
	     << DateTime::strn(gen[0]) << endl;
      } else {
	out << "  First before time:" << endl;
	out << "    " << DateTime::strn(valid[0]) << endl;
      }
    }
    break;
  }

  case MdvxTimeList::MODE_FIRST_AFTER: {
    out << "TIME LIST - MODE FIRST_AFTER" << endl;
    out << "  URL: " << url << endl;
    out << "  Search time: "
	 << DateTime::strn(searchTime) << endl;
    out << "  Time margin: " << marginSecs << endl;
    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    bool hasForecasts = mdvx->timeListHasForecasts();
    if (valid.size() == 0) {
      out << "    No time returned." << endl;
    } else {
      if (hasForecasts) {
	out << "  First after valid/gen time:" << endl;
	out << "    "
	     << DateTime::strn(valid[0]) << " / "
	     << DateTime::strn(gen[0]) << endl;
      } else {
	out << "  First after time:" << endl;
	out << "    " << DateTime::strn(valid[0]) << endl;
      }
    }
    break;
  }

  case MdvxTimeList::MODE_BEST_FCAST: {
    out << "TIME LIST - MODE BEST_FORECAST" << endl;
    out << "  URL: " << url << endl;
    out << "  Search time: "
	 << DateTime::strn(searchTime) << endl;
    out << "  Time margin: " << marginSecs << endl;
    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    if (valid.size() == 0) {
      out << "    No time returned." << endl;
    } else {
      out << "  Best forecast valid/gen time:" << endl;
      out << "    "
	   << DateTime::strn(valid[0]) << " / "
	   << DateTime::strn(gen[0]) << endl;
    }
    break;
  }

  case MdvxTimeList::MODE_SPECIFIED_FCAST: {
    out << "TIME LIST - MODE SPECIFIED_FORECAST" << endl;
    out << "  URL: " << url << endl;
    out << "  Generate time: "
	 << DateTime::strn(genTime) << endl;
    out << "  Search time: "
	 << DateTime::strn(searchTime) << endl;
    out << "  Time margin: " << marginSecs << endl;
    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    if (valid.size() == 0) {
      out << "    No time returned." << endl;
    } else {
      out << "  Specified forecast valid/gen time:" << endl;
      out << "    "
	   << DateTime::strn(valid[0]) << " / "
	   << DateTime::strn(gen[0]) << endl;
    }
    break;
  }

    default: {}
    
  } // switch

}
  
/////////////////////////////////////////////////
// print the time height profile

void Mdvx::printTimeHeight(ostream &out,
                           const Mdvx *mdvx,
                           bool printData /* = false*/,
                           bool transformToLinear /* = false*/,
                           bool printNative /* = false */)

{

  // master header
  
  // const Mdvx::master_header_t &mhdr = mdvx->getMasterHeader();
  mdvx->printFormats(out);
  mdvx->printMasterHeader(out);
  
  // fields
  
  for (size_t i = 0; i < mdvx->getNFields(); i++) {

    MdvxField *field = mdvx->getField(i);
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    mdvx->printFieldHeader(fhdr, out);

    const Mdvx::vlevel_header_t &vhdr = field->getVlevelHeader();
    mdvx->printVlevelHeader(vhdr, fhdr.nz, fhdr.field_name, out);

    if (printData) {
      if (transformToLinear) {
	if (field->transform2Linear()){
	  cerr << field->getErrStr() << endl;
	}
      }
      field->printTimeHeightData(out,
				 mdvx->getValidTimes(), printNative);
    }

  } // i

}


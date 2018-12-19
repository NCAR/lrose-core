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
// AccumData.cc
//
// Accumulation data object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2015
//
//////////////////////////////////////////////////////////
//
// This module acummulates the precip data in fl32 arrays.
//
///////////////////////////////////////////////////////////

#include "AccumData.hh"
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <toolsa/TaArray.hh>
#include <toolsa/mem.h>
#include <physics/vil.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <Spdb/DsSpdb.hh>
#include <rapformats/zr.h>
#include <cmath>
using namespace std;

//////////////
// Constructor

AccumData::AccumData(const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)

{

  // set up rate names
  
  _rateFieldNames.clear();
  _accumFieldNames.clear();
  _inputIsDepth.clear();

  for (int ii = 0; ii < _params.field_names_n; ii++) {
    _rateFieldNames.push_back(_params._field_names[ii].rate_name);
    _accumFieldNames.push_back(_params._field_names[ii].accum_name);
    _inputIsDepth.push_back(_params._field_names[ii].input_is_depth);
  }
  
  init();

}

/////////////
// Destructor

AccumData::~AccumData()

{

  free();

}

//////////////////////////////
// initialize for computations

void AccumData::init()

{

  free();

  _dataFound = false;
  _nxy = 0;
  MEM_zero(_grid);
  
  _actualAccumPeriod = 0.0;
  _targetAccumPeriod = 0.0;
  
}

////////////////////////
// free up memory arrays

void AccumData::free()

{

  for (size_t ii = 0; ii < _accumFieldData.size(); ii++) {
    delete[] _accumFieldData[ii];
  }
  _accumFieldData.clear();

}

///////////////////
// setTargetPeriod()
//
// Set the target period for accumulation.
//
// The actual accumulation period is determined from
// durations provided. The precip depths
// are later adjusted to the exact target period.

void AccumData::setTargetPeriod(double period)

{
  _targetAccumPeriod = period;
}

/////////////////
// processInput()
//
// Process input data from an MDV file.

int AccumData::processFile(const string &file_path,
                           time_t file_time,
                           double file_duration)

{

  PMU_auto_register("AccumData::processInput");

  time_t fileStartTime = file_time - (int) (file_duration + 0.5);

  // read in the file
  
  DsMdvx mdvx;
  mdvx.setReadPath(file_path);    
  
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  for (size_t ii = 0; ii < _rateFieldNames.size(); ii++) {
    mdvx.addReadField(_rateFieldNames[ii].c_str());
  }
  mdvx.setReadComposite(); // ensure 2D
  
  if (mdvx.readVolume()) {
    cerr << "ERROR - AccumData::processInput" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }

  // if first data, set things up. Else check that grid has
  // not changed
  
  Mdvx::master_header_t mhdr = mdvx.getMasterHeader();
  const MdvxField *fld0 = mdvx.getField(0);
  Mdvx::field_header_t fhdr0 = fld0->getFieldHeader();
  MdvxProj proj(mhdr, fhdr0);

  if (!_dataFound) {
    
    _grid = proj.getCoord();
    _nxy = _grid.nx * _grid.ny;
    
    for (size_t ii = 0; ii < _rateFieldNames.size(); ii++) {
      fl32 *rateField = new fl32[_nxy];
      memset(rateField, 0, _nxy * sizeof(fl32));
      _accumFieldData.push_back(rateField);
    }

    if (_params.debug >= Params::DEBUG_NORM) {    
      cerr << "AccumData::processFile mhdr.time_begin = "
           << DateTime(mhdr.time_begin) << endl;
      cerr << "AccumData::processFile mhdr.time_end = "
           << DateTime(mhdr.time_end) << endl;
    }
  
    _dataStartTime = fileStartTime;
    _dataEndTime = file_time;
    _dataFound = true;

  } else {
    
    // check input geometry

    const Mdvx::coord_t &coord = proj.getCoord();
    if (coord.proj_type != _grid.proj_type ||
        fabs(coord.proj_origin_lat -_grid.proj_origin_lat) > 0.001 ||
        fabs(coord.proj_origin_lon - _grid.proj_origin_lon) > 0.001 ||
        coord.nx != _grid.nx ||
        coord.ny != _grid.ny ||
        fabs(coord.minx -_grid.minx) > 0.001 ||
        fabs(coord.miny - _grid.miny) > 0.001 ||
        fabs(coord.dx - _grid.dx) > 0.001 ||
        fabs(coord.dy - _grid.dy) > 0.001) {
      
      fprintf(stderr, "ERROR - %s:AccumData::processInput\n",
              _progName.c_str());
      fprintf(stderr, "Input file grid has changed.\n");
      fprintf(stderr, "Original grid:\n");
      fprintf(stderr, "==============\n");
      proj.printCoord(_grid, cerr);
      fprintf(stderr, "Grid for time %s\n",
              utimstr(mhdr.time_centroid));
      fprintf(stderr, "===============================\n");
      proj.printCoord(proj.getCoord(), cerr);
      return (-1);
      
    } // if (coord.proj_type != ...
    
  } // if (!_dataFound) 
  
  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "AccumData: processing file at time: "
         << DateTime::strm(file_time) << endl;
  }
  
  // set the times
  
  _dataStartTime = MIN(_dataStartTime, file_time);
  _dataEndTime = MAX(_dataEndTime, file_time);
  _actualAccumPeriod += file_duration;

  // update accum for each rate field

  for (size_t ii = 0; ii < _rateFieldNames.size(); ii++) {
    
    const MdvxField *fld = mdvx.getField(_rateFieldNames[ii].c_str());
    if (fld == NULL) {
      cerr << "ERROR - AccumData::processFile" << endl;
      cerr << "  Cannot find rate field: " << _rateFieldNames[ii] << endl;
      return -1;
    }
    
    _updateAccum(_rateFieldNames[ii], _inputIsDepth[ii],
                 fld, _accumFieldData[ii], file_duration);
    
  } // ii

  return (0);

}

//////////////////////////////////////////////////////
// Using supplied rate, add to the accumulated grid.
//

void AccumData::_updateAccum(const string &fieldName,
                             bool inputIsDepth,
                             const MdvxField *fld,
                             fl32 *accumFieldData,
                             double fileDuration)
  
{
  
  if (inputIsDepth) {

    fl32 *depth = (fl32 *) fld->getVol();
    fl32 missing = fld->getFieldHeader().missing_data_value;
    fl32 *accum = accumFieldData;
    for (int ii = 0; ii < _nxy; ii++, depth++, accum++) {
      if (*depth != missing) {
        (*accum) += *depth; // mm
      }
    } // i

  } else {

    fl32 *rate = (fl32 *) fld->getVol();
    fl32 missing = fld->getFieldHeader().missing_data_value;
    fl32 *accum = accumFieldData;
    for (int ii = 0; ii < _nxy; ii++, rate++, accum++) {
      if (*rate != missing) {
        (*accum) += *rate * (fileDuration / 3600.0); // mm from mm/hr
      }
    } // i

  }
  
}

////////////////////////////////////////
// write()

int AccumData::write(const string &output_url)
  
{
  
  PMU_auto_register("AccumData::write");

  if (!_dataFound) {
    cerr << "WARNING - AccumData::write" << endl;
    cerr << "  No data found" << endl;
    return 0;
  }

  if (_params.debug) {
    cerr << "AccumData::write\n";
    cerr << "  endTime: " << DateTime::strm(_dataEndTime) << endl;
  }
  
  DsMdvx mdvx;
  
  // set the master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  MdvxProj proj(_grid);

  mhdr.time_gen = _dataStartTime;
  mhdr.time_begin = _dataStartTime;
  mhdr.time_end = _dataEndTime;

  mhdr.time_centroid = _dataEndTime;
  mhdr.time_expire = _dataEndTime;
  
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 2;
  
  mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  
  mhdr.max_nx = _grid.nx;
  mhdr.max_ny = _grid.ny;
  mhdr.max_nz = 1;
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;

  mhdr.sensor_lon = _grid.proj_origin_lon;
  mhdr.sensor_lat = _grid.proj_origin_lat;
  
  // info
  
  char info[2048];
  sprintf(info, "Precip accumulation from rate.\n");
  STRncopy(mhdr.data_set_info, info, MDV_INFO_LEN);
  STRncopy(mhdr.data_set_name, _params.data_set_name, MDV_NAME_LEN);
  STRncopy(mhdr.data_set_source, _params.data_set_source, MDV_NAME_LEN);
  
  mdvx.setMasterHeader(mhdr);

  // fill in field headers and vlevel headers
  
  mdvx.clearFields();
  
  for (size_t ifield = 0; ifield < _rateFieldNames.size(); ifield++) {
    
    Mdvx::field_header_t fhdr;
    MEM_zero(fhdr);
    Mdvx::vlevel_header_t vhdr;
    MEM_zero(vhdr);

    fhdr.nx = _grid.nx;
    fhdr.ny = _grid.ny;
    fhdr.nz = 1;

    fhdr.proj_type = (Mdvx::projection_type_t) _grid.proj_type;
    
    fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
    fhdr.data_element_nbytes = sizeof(fl32);
    fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
    fhdr.compression_type = Mdvx::COMPRESSION_NONE;
    fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
    fhdr.scaling_type = Mdvx::SCALING_NONE;

    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    fhdr.dz_constant = true;

    proj.syncXyToFieldHdr(fhdr);

    fhdr.grid_dz = 1;
    fhdr.grid_minz = 0;

    fhdr.proj_rotation = 0.0;
    fhdr.bad_data_value = -9999.0;
    fhdr.missing_data_value = -9999.0;

    _setFieldName(mhdr, fhdr,
                  _rateFieldNames[ifield],
                  _accumFieldNames[ifield]);
    
    // vlevel header
    
    vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
    vhdr.level[0] = 0;

    // fill array with data adjusted for expected duration
    
    fl32 *adjusted = new fl32[_nxy];
    _adjustDepth(_accumFieldData[ifield], adjusted);

    // create field
    
    MdvxField *field = new MdvxField(fhdr, vhdr, adjusted);
    field->compress(Mdvx::COMPRESSION_GZIP);
    
    // add field to mdvx object
    
    mdvx.addField(field);

    // free up

    delete[] adjusted;

  } // ifield
  
  mdvx.setWriteLdataInfo();
  
  if (_params.debug) {
    cerr << "Writing data, for time: "
	 << DateTime::strm(_dataEndTime) << " to url:" 
	 << output_url << endl;
  }
  
  if (mdvx.writeToDir(output_url)) {
    cerr << "ERROR - AccumData::write()" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote file: " << mdvx.getPathInUse() << endl;
    cerr << endl;
  }

  return 0;

}

////////////////////////////////////////////////////
// _setFieldName()
//
// Sets the field name, units etc
//

void AccumData::_setFieldName(const Mdvx::master_header_t &mhdr,
                              Mdvx::field_header_t &fhdr,
                              const string &rateName,
                              const string &accumName)
  
{

  char fieldNameLong[1024];
  if (_params.accum_method == Params::DAILY_ACCUM) {
    date_time_t start, end;
    start.unix_time = mhdr.time_begin;
    uconvert_from_utime(&start);
    end.unix_time = mhdr.time_end;
    uconvert_from_utime(&end);
    sprintf(fieldNameLong,
	    "%s_%.2d:%.2dZ_to_%.2d:%.2dZ", accumName.c_str(),
	    start.hour, start.min, end.hour, end.min);
  } else {
    int dur = mhdr.time_end - mhdr.time_begin;
    double dur_day = dur / 86400.0;
    int dur_hour = dur / 3600;
    int dur_min = (dur % 3600) / 60;
    if (dur_day > 1.0) {
      sprintf(fieldNameLong, "%s-%.1fd", accumName.c_str(), dur_day);
    } else if (dur_min == 0) {
      sprintf(fieldNameLong, "%s-%dh", accumName.c_str(), dur_hour);
    } else {
      sprintf(fieldNameLong, "%s-%d:%.2dh", accumName.c_str(), dur_hour, dur_min);
    }
  }
  
  STRncopy(fhdr.field_name, accumName.c_str(), MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr.field_name_long, fieldNameLong, MDV_LONG_FIELD_LEN);
  STRncopy(fhdr.units, "mm", MDV_UNITS_LEN);
  
}


////////////////////
// computeAdjusted()
//
// Computes the precip data adjusted for the ratio of the
// actual accum period to the target accum period.
//

void AccumData::_adjustDepth(const fl32 *accum,
                             fl32 *adjusted)
  
{

  double ratio = 1.0;
  if (_actualAccumPeriod > 0) {
    ratio = _targetAccumPeriod / _actualAccumPeriod;
  }
  if (ratio < 0.5 || ratio > 2.0) {
    ratio = 1.0;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "AccumData::_adjustDepth" << endl;
    cerr << "  _targetAccumPeriod: " << _targetAccumPeriod << endl;
    cerr << "  _actualAccumPeriod: " << _actualAccumPeriod << endl;
    cerr << "  ratio: " << ratio << endl;
  }

  // fl32 maxDepth = _params.max_precip_depth;
  fl32 minDepth = _params.min_precip_depth;
  fl32 missing = -9999.0;
  
  for (int ii = 0; ii < _nxy; ii++, accum++, adjusted++) {
    fl32 adj = *accum * ratio;
    /* if (adj > maxDepth) {
      adj = maxDepth;
      } else */ if (adj < minDepth) {
      adj = missing;
    }
    *adjusted = adj;
  }
    
}


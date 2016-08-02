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
// Sept 1998
//
///////////////////////////////////////////////////////////////

#include "OutputFile.hh"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <Mdv/MdvxField.hh>
using namespace std;

//////////////
// Constructor

OutputFile::OutputFile(const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)
  
{

  _precipFieldNum = -1;
  _rateFieldNum = -1;
  _maxDbzFieldNum = -1;
  _maxVilFieldNum = -1;

  // compute number of fields - field 0 is always precip

  _precipFieldNum = 0;
  _nFields = 1;
  if (_params.generate_rate_grid) {
    _rateFieldNum = _nFields;
    _nFields++;
  }
  if (_params.generate_max_dbz_grid) {
    _maxDbzFieldNum = _nFields;
    _nFields++;
  }
  if (_params.generate_max_vil_grid) {
    _maxVilFieldNum = _nFields;
    _nFields++;
  }

}

/////////////
// destructor

OutputFile::~OutputFile()

{
  
}

////////////////////////////////////////
// write()
//
// Write out the precip data
//

int OutputFile::write(time_t start_time,
		      time_t end_time,
		      time_t centroid_time,
		      int forecast_lead_time,
                      int accum_period_secs,
		      const Mdvx::coord_t &grid,
		      fl32 *precip,
		      fl32 *rate,
		      fl32 *max_dbz,
		      fl32 *max_vil)
  
{
  
  DsMdvx mdvx;

  // set the master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  MdvxProj proj(grid);

  mhdr.time_gen = start_time;
  mhdr.time_begin = start_time;
  mhdr.time_end = end_time;

  if (_params.debug) {
    cerr << "OutputFile::write\n";
    cerr << "  centroid_time = " << DateTime(centroid_time) << endl;
    cerr << endl;
  }
  
  mhdr.time_centroid = centroid_time;
  mhdr.time_expire = end_time;
  
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 2;
  
  if(_params.accum_method == Params::SINGLE_FILE_FORECAST)
    mhdr.data_collection_type = Mdvx::DATA_FORECAST;
  else
    mhdr.data_collection_type = Mdvx::DATA_MEASURED;

  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;


  mhdr.max_nx = grid.nx;
  mhdr.max_ny = grid.ny;
  mhdr.max_nz = 1;
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;

  mhdr.sensor_lon = grid.proj_origin_lon;
  mhdr.sensor_lat = grid.proj_origin_lat;
  
  // info
  
  char info[2048];
  if (_params.set_max_precip_depth) {
    sprintf(info,
	    "Precip accumulation using ZR and reflectivity composite.\n"
	    "  ZR coeff, expon: %g, %g\n"
	    "  Hail threshold: %g\n"
	    "  Low dBZ threshold: %g\n"
	    "  Max precip depth: %g\n"
	    "  min_ht, max_ht: %g, %g\n",
	    _params.zr.coeff,
	    _params.zr.expon,
	    _params.hail_dbz_threshold,
	    _params.low_dbz_threshold,
	    _params.max_precip_depth,
	    _params.composite_min_altitude,
	    _params.composite_max_altitude);
  } else {
    sprintf(info,
	    "Precip accumulation using ZR and reflectivity composite.\n"
	    "  ZR coeff, expon: %g, %g\n"
	    "  Hail threshold: %g\n"
	    "  Low dBZ threshold: %g\n"
	    "  min_ht, max_ht: %g, %g\n",
	    _params.zr.coeff,
	    _params.zr.expon,
	    _params.hail_dbz_threshold,
	    _params.low_dbz_threshold,
	    _params.composite_min_altitude,
	    _params.composite_max_altitude);
  }
  
  STRncopy(mhdr.data_set_info, info, MDV_INFO_LEN);
  STRncopy(mhdr.data_set_name, _params.data_set_name, MDV_NAME_LEN);
  STRncopy(mhdr.data_set_source, _params.data_set_source, MDV_NAME_LEN);
  
  mdvx.setMasterHeader(mhdr);

  if (_params.debug) {
    cerr << "OutputFile::write\n";
    cerr << "mhdr.time_centroid = " << DateTime(mhdr.time_centroid) << endl;
    cerr << endl;
  }

  // fill in field headers and vlevel headers
  
  mdvx.clearFields();
  
  for (int out_field = 0; out_field < _nFields; out_field++) {
    
    Mdvx::field_header_t fhdr;
    MEM_zero(fhdr);
    Mdvx::vlevel_header_t vhdr;
    MEM_zero(vhdr);

    fhdr.nx = grid.nx;
    fhdr.ny = grid.ny;
    fhdr.nz = 1;

    fhdr.proj_type = (Mdvx::projection_type_t) grid.proj_type;
    
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

    // Replace missing data with a value of zero.
    // Need to set missing to something else here.
    if(_params.set_missing_to_zero)
    {
      fhdr.bad_data_value = -99.0;
      fhdr.missing_data_value = -99.0;
    }
    else 
    {
      fhdr.bad_data_value = 0.0;
      fhdr.missing_data_value = 0.0;
    }
    
    if (out_field == _precipFieldNum) {
      _setFieldName(mhdr, fhdr, accum_period_secs, "precip", "Precip Accum", "mm");
    } else if (out_field == _rateFieldNum) {
      _setFieldName(mhdr, fhdr, accum_period_secs, "rate", "Precip rate", "mm/hr");
    } else if (out_field == _maxDbzFieldNum) {
      _setFieldName(mhdr, fhdr, accum_period_secs, "maxdbz", "Max dBZ", "dBZ");
    } else {
      _setFieldName(mhdr, fhdr, accum_period_secs, "maxvil", "Max vil", "kg/m2");
    }
    
    // vlevel header
    
    vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
    vhdr.level[0] = 0;
    
    // create field
    
    MdvxField *field;
    
    if (out_field == _precipFieldNum) {
      field = new MdvxField(fhdr, vhdr, precip);
      if (_params.store_precip_as_log) {
	field->transform2Log();
	field->convertDynamic((Mdvx::encoding_type_t) _params.output_encoding_type,
			      Mdvx::COMPRESSION_GZIP);
      } else {
	field->convertRounded((Mdvx::encoding_type_t) _params.output_encoding_type,
			      Mdvx::COMPRESSION_GZIP);
      }
    } else if (out_field == _rateFieldNum) {
      field = new MdvxField(fhdr, vhdr, rate);
      field->convertRounded((Mdvx::encoding_type_t) _params.output_encoding_type,
			    Mdvx::COMPRESSION_GZIP);
    } else if (out_field == _maxDbzFieldNum) {
      field = new MdvxField(fhdr, vhdr, max_dbz);
      field->convertRounded((Mdvx::encoding_type_t) _params.output_encoding_type,
			    Mdvx::COMPRESSION_GZIP);
    } else {
      field = new MdvxField(fhdr, vhdr, max_vil);
      field->convertRounded((Mdvx::encoding_type_t) _params.output_encoding_type,
			    Mdvx::COMPRESSION_GZIP);
    }

    // add field to mdvx object
    
    mdvx.addField(field);

  } // out_field

  mdvx.setForecastLeadSecs(forecast_lead_time);
  mdvx.setForecastTime(mhdr.time_gen + forecast_lead_time);
  mdvx.setWriteLdataInfo();

  if (_params.write_as_forecast)
  {
    mdvx.setWriteAsForecast();
  }

  if (_params.debug) {
    cerr << "Writing data, for time: "
	 << DateTime::str(centroid_time) << " to url:" 
	 << _params.output_precip_dir << endl;
  }
  
  if (mdvx.writeToDir(_params.output_precip_dir)) {
    cerr << "ERROR - OutputFile::write()" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////
// _setFieldName()
//
// Sets the field name, units etc
//

void OutputFile::_setFieldName(const Mdvx::master_header_t &mhdr,
			       Mdvx::field_header_t &fhdr,
                               int accum_period_secs,
			       const char *name,
			       const char *name_long,
			       const char *units)
  
{

  char fieldNameLong[1024];
  if (_params.accum_method == Params::ACCUM_FROM_TIME_OF_DAY) {
    date_time_t start, end;
    start.unix_time = mhdr.time_begin;
    uconvert_from_utime(&start);
    end.unix_time = mhdr.time_end;
    uconvert_from_utime(&end);
    sprintf(fieldNameLong,
	    "%s %.2d:%.2dZ to %.2d:%.2dZ", name_long,
	    start.hour, start.min, end.hour, end.min);
  } else {
    // int dur = mhdr.time_end - mhdr.time_begin;
    int dur = accum_period_secs;
    double dur_day = dur / 86400.0;
    int dur_hour = dur / 3600;
    int dur_min = (dur % 3600) / 60;
    char dur_str[64];
    if (dur_day > 1.0) {
      sprintf(dur_str, "%.1fd %s", dur_day, name_long);
    } else if (dur_min == 0) {
      sprintf(dur_str, "%dh %s", dur_hour, name_long);
    } else {
      sprintf(dur_str, "%d:%.2dh %s", dur_hour, dur_min, name_long);
    }
    STRncopy(fieldNameLong, dur_str, MDV_LONG_FIELD_LEN);
  }
  

  STRncopy(fhdr.field_name, name, MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr.field_name_long, fieldNameLong, MDV_LONG_FIELD_LEN);
  STRncopy(fhdr.units, units, MDV_UNITS_LEN);
  
}





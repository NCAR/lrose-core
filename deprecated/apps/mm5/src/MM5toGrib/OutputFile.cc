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
// Octiber 1998
//
///////////////////////////////////////////////////////////////

#include "OutputFile.hh"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <toolsa/udatetime.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxFieldCode.hh>
#include <mm5/MM5Data.hh>
using namespace std;

//////////////
// Constructor

OutputFile::OutputFile(const string &prog_name,
		       const Params &params,
		       time_t model_time,
		       time_t forecast_time,
		       int forecast_delta,
		       const MM5Data &inData) :
  _progName(prog_name), _params(params)

{

  _initMdvx(model_time, forecast_time, forecast_delta, inData);

}

/////////////
// destructor

OutputFile::~OutputFile()

{
  
}

////////////////////////////////////////
// writeVol()
//
// Write out volume in MDV format.
//

int OutputFile::writeVol()
  
{

  PMU_auto_register("In OutputFile::writeVol");
  
  if (_params.debug) {
    fprintf(stderr, "Writing MDV file, time %s, to url %s\n",
	    utimstr(_mdvx.getMasterHeader().time_centroid), _params.output_url);
  }
  
  // set precip fields to log scale
  
  MdvxField *rain_con = _mdvx.getField("rain_con");
  if (rain_con) {
    rain_con->transform2Log();
  }
  MdvxField *rain_non = _mdvx.getField("rain_non");
  if (rain_non) {
    rain_non->transform2Log();
  }
  MdvxField *rain_total = _mdvx.getField("rain_total");
  if (rain_total) {
    rain_total->transform2Log();
  }

  // write to directory
  
  _mdvx.setWriteLdataInfo();
  if (_mdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - OutputFile::writeVol" << endl;
    cerr << _mdvx.getErrStr() << endl;
    return -1;
  } else {
    return 0;
  }

}


// Return the reference latitude appropriate for this hemisphere.
// The grib spec defines: "Latin 1 - The first latitude from the pole
// at which the secant cone cuts the spherical earth."
// http://www.nco.ncep.noaa.gov/pmb/docs/on388/tabled.html
fl32 OutputFile::getLambertLat1(fl32 centerLat, fl32 oneLat, fl32 anotherLat)
{
  if (centerLat >= 0.0)
    // North pole
    return max(oneLat, anotherLat);
  else
    // South pole
    return min(oneLat, anotherLat);
}

fl32 OutputFile::getLambertLat2(fl32 centerLat, fl32 oneLat, fl32 anotherLat)
{
  if (centerLat >= 0.0)
    return min(oneLat, anotherLat);
  else
    return max(oneLat, anotherLat);
}


////////////////////////////////////////////////////
// _initMdvx()
//
// Initialize the MDVX object
//

void OutputFile::_initMdvx(time_t model_time, time_t forecast_time,
			   int forecast_delta, const MM5Data &inData)
  
{

  // set the master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  mhdr.time_gen = model_time;
  mhdr.time_begin = forecast_time;
  mhdr.time_end = forecast_time;
  mhdr.time_centroid = forecast_time;
  mhdr.time_expire = forecast_time + forecast_delta;
    
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 3;
  
  mhdr.data_collection_type = Mdvx::DATA_FORECAST;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SIGMA_P;

  switch (_params.output_levels) {
  case Params::NATIVE_SIGMA_LEVELS:
    mhdr.vlevel_type = Mdvx::VERT_TYPE_SIGMA_P;
    mhdr.max_nz = inData.nSigma;
    break;
  case Params::FLIGHT_LEVELS:
    mhdr.vlevel_type = Mdvx::VERT_FLIGHT_LEVEL;
    mhdr.max_nz = _params.flight_levels_n;
    break;
  case Params::PRESSURE_LEVELS:
    mhdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
    mhdr.max_nz = _params.pressure_levels_n;
    break;
  case Params::HEIGHT_LEVELS:
    mhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
    mhdr.max_nz = _params.height_levels_n;
    break;
  }

  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  if (_params.output_projection == Params::OUTPUT_PROJ_NATIVE) {
    mhdr.max_nx = inData.nLon;
    mhdr.max_ny = inData.nLat;
  } else {
    mhdr.max_nx = _params.output_grid.nx;
    mhdr.max_ny = _params.output_grid.ny;
  }

  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = 0.0;
  mhdr.sensor_lat = 0.0;
  mhdr.sensor_alt = 0.0;

  STRncopy(mhdr.data_set_info, _params.data_set_info, MDV_INFO_LEN);
  STRncopy(mhdr.data_set_name, _params.data_set_name, MDV_NAME_LEN);
  STRncopy(mhdr.data_set_source, _params.data_set_source, MDV_NAME_LEN);

  _mdvx.setMasterHeader(mhdr);
  
  // fill in field headers and vlevel headers
  
  // add fields

  _mdvx.clearFields();
  
  for (int out_field = 0; out_field < _params.output_fields_n; out_field++) {
    
    Mdvx::field_header_t fhdr;
    MEM_zero(fhdr);
    Mdvx::vlevel_header_t vhdr;
    MEM_zero(vhdr);

    fhdr.forecast_delta = forecast_delta;
    fhdr.forecast_time = forecast_time;

    switch (_params.output_projection) {
      
    case Params::OUTPUT_PROJ_FLAT: {
      fhdr.proj_type = Mdvx::PROJ_FLAT;
      break;
    }

    case Params::OUTPUT_PROJ_LATLON: {
      fhdr.proj_type = Mdvx::PROJ_LATLON;
      break;
    }
    
    case Params::OUTPUT_PROJ_LAMBERT: {
      fhdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
      fhdr.proj_param[0] = _params.lambert_lat1;
      fhdr.proj_param[1] = _params.lambert_lat2;
      break;
    }
    
    case Params::OUTPUT_PROJ_STEREOGRAPHIC: {
      fhdr.proj_type = Mdvx::PROJ_OBLIQUE_STEREO;
      fhdr.proj_param[0] = _params.stereographic_tangent_lat;
      fhdr.proj_param[1] = _params.stereographic_tangent_lon;
      break;
    }

    case Params::OUTPUT_PROJ_MERCATOR: {
      fhdr.proj_type = Mdvx::PROJ_MERCATOR;
      break;
    }

    case Params::OUTPUT_PROJ_NATIVE: {
      switch (inData.proj_type) {
      case MM5Data::LAMBERT_CONF: {
	fhdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
	fhdr.proj_param[0] = getLambertLat1(inData.center_lat, inData.true_lat1, inData.true_lat2);
	fhdr.proj_param[1] = getLambertLat2(inData.center_lat, inData.true_lat1, inData.true_lat2);
	break;
      }
      case MM5Data::STEREOGRAPHIC: {
	fhdr.proj_type = Mdvx::PROJ_OBLIQUE_STEREO;
	fhdr.proj_param[0] = inData.true_lat1;
	fhdr.proj_param[1] = inData.center_lon;
	break;
      }
      case MM5Data::MERCATOR: {
	fhdr.proj_type = Mdvx::PROJ_MERCATOR;
	break;
      }
      default: {
	fhdr.proj_type = Mdvx::PROJ_UNKNOWN;
      }
      } // switch proj_type
      break;
    }
    } // switch output_projection
    
    if (_params.output_projection == Params::OUTPUT_PROJ_NATIVE) {

      fhdr.proj_origin_lat = inData.center_lat;
      fhdr.proj_origin_lon = inData.center_lon;
      fhdr.nx = inData.nLon;
      fhdr.ny = inData.nLat;
      fhdr.grid_dx = inData.grid_distance;
      fhdr.grid_dy = inData.grid_distance;
      fhdr.grid_minx = inData.minx_cross;
      fhdr.grid_miny = inData.miny_cross;

    } else {

      fhdr.proj_origin_lat = _params.output_origin.lat;
      fhdr.proj_origin_lon = _params.output_origin.lon;
      fhdr.nx = _params.output_grid.nx;
      fhdr.ny = _params.output_grid.ny;
      fhdr.grid_dx = _params.output_grid.dx;
      fhdr.grid_dy = _params.output_grid.dy;
      fhdr.grid_minx = _params.output_grid.minx;
      fhdr.grid_miny = _params.output_grid.miny;

    }
    
    _npointsPlane = fhdr.nx * fhdr.ny;
    
    fhdr.compression_type = Mdvx::COMPRESSION_NONE;
    fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
    fhdr.scaling_type = Mdvx::SCALING_NONE;

    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SIGMA_P;
    switch (_params.output_levels) {
    case Params::NATIVE_SIGMA_LEVELS:
      fhdr.vlevel_type = Mdvx::VERT_TYPE_SIGMA_P;
      fhdr.nz = inData.nSigma;
      fhdr.grid_minz = inData.get_halfSigma(0);
      fhdr.grid_dz = 0.1;
      break;
    case Params::FLIGHT_LEVELS:
      fhdr.vlevel_type = Mdvx::VERT_FLIGHT_LEVEL;
      fhdr.nz = _params.flight_levels_n;
      fhdr.grid_minz = _params._flight_levels[0];
      fhdr.grid_dz = 1.0;
      break;
    case Params::PRESSURE_LEVELS:
      fhdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
      fhdr.nz = _params.pressure_levels_n;
      fhdr.grid_minz = _params._pressure_levels[0];
      fhdr.grid_dz = 1.0;
      break;
    case Params::HEIGHT_LEVELS:
      fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
      fhdr.nz = _params.height_levels_n;
      fhdr.grid_minz = _params._height_levels[0];
      fhdr.grid_dz = 1.0;
      break;
    }
    fhdr.dz_constant = false;

    if (_params._output_fields[out_field].name > Params::START_2D_FIELDS) {
      fhdr.nz = 1;
      fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
      fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    }
    
    fhdr.proj_rotation = 0.0;
    
    fhdr.bad_data_value = MM5Data::MissingDouble;
    fhdr.missing_data_value = MM5Data::MissingDouble;

    fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
    fhdr.data_element_nbytes = sizeof(fl32);
    if (_params._output_fields[out_field].name > Params::START_2D_FIELDS) {
      fhdr.volume_size = fhdr.nx * fhdr.ny * sizeof(fl32);
    } else {
      fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
    }

    switch (_params._output_fields[out_field].name) {

      MdvxFieldCode::entry_t codeEntry;

      // raw 3-d fields

    case Params::U_FIELD:

      MdvxFieldCode::getEntryByAbbrev("U GRD", codeEntry);
      if (_params.wind_speed_in_knots) {
	_setFieldName(fhdr, "U", "u-component of wind", "kts",
		      "none", codeEntry.code);
      } else {
	_setFieldName(fhdr, "U", "u-component of wind", "m/s",
		      "none", codeEntry.code);
      }
      break;

    case Params::V_FIELD:
      MdvxFieldCode::getEntryByAbbrev("V GRD", codeEntry);
      if (_params.wind_speed_in_knots) {
	_setFieldName(fhdr, "V", "v-component of wind", "kts",
		      "none", codeEntry.code);
      } else {
	_setFieldName(fhdr, "V", "v-component of wind", "m/s",
		      "none", codeEntry.code);
      }
      break;
      
    case Params::TK_FIELD:
      MdvxFieldCode::getEntryByAbbrev("TMP", codeEntry);
      _setFieldName(fhdr, "TK", "Temperature", "K",
		    "none", codeEntry.code);
      break;
      
    case Params::Q_FIELD:
      MdvxFieldCode::getEntryByAbbrev("MIXR", codeEntry);
      _setFieldName(fhdr, "Q", "Mixing ratio", "kg/kg",
		    "none", codeEntry.code);
      break;
      
    case Params::CLW_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "clw", "Cloud liquid mixing ratio", "kg/kg",
		    "none", codeEntry.code);
      break;

    case Params::RNW_FIELD:
      MdvxFieldCode::getEntryByAbbrev("RWMR", codeEntry);
      _setFieldName(fhdr, "rnw", "Rain mixing ratio", "kg/kg",
		    "none", codeEntry.code);
      break;

    case Params::ICE_FIELD:
      MdvxFieldCode::getEntryByAbbrev("ICMR", codeEntry);
      _setFieldName(fhdr, "ice", "Ice mixing ratio", "kg/kg",
		    "none", codeEntry.code);
      break;

    case Params::SNOW_FIELD:
      MdvxFieldCode::getEntryByAbbrev("SNMR", codeEntry);
      _setFieldName(fhdr, "snow", "Snow mixing ratio", "kg/kg",
		    "none", codeEntry.code);
      break;

    case Params::GRAUPEL_FIELD:
      MdvxFieldCode::getEntryByAbbrev("GRMR", codeEntry);
      _setFieldName(fhdr, "graupel", "Graupel mixing ratio", "kg/kg",
		    "none", codeEntry.code);
      break;

    case Params::NCI_FIELD:
      MdvxFieldCode::getEntryByAbbrev("NCIP", codeEntry);
      _setFieldName(fhdr, "nci", "Number conc ice", "#/m^3",
		    "none", codeEntry.code);
      break;
      
    case Params::RAD_TEND_FIELD:
      MdvxFieldCode::getEntryByAbbrev("TTRAD", codeEntry);
      _setFieldName(fhdr, "rad_tend", "Atmospheric radiation tendency",
		    "K/day", "none", codeEntry.code);
      break;

    case Params::W_FIELD:
      MdvxFieldCode::getEntryByAbbrev("DZDT", codeEntry);
      _setFieldName(fhdr, "W", "w-component of wind", "m/s",
		    "none", codeEntry.code);
      break;

    case Params::P_FIELD:
      MdvxFieldCode::getEntryByAbbrev("PRESA", codeEntry);
      _setFieldName(fhdr, "P", "pressure perturbation", "Pa",
		    "none", codeEntry.code);
      break;

      // derived 3-d fields

    case Params::TC_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Temp", "Temperature", "C",
		    "none", codeEntry.code);
      break;
      
    case Params::WSPD_FIELD:
      MdvxFieldCode::getEntryByAbbrev("WIND", codeEntry);
      if (_params.wind_speed_in_knots) {
	_setFieldName(fhdr, "Speed", "Wind speed", "kts",
		      "none", codeEntry.code);
      } else {
	_setFieldName(fhdr, "Speed", "Wind speed", "m/s",
		      "none", codeEntry.code);
      }
      break;

    case Params::WDIR_FIELD:
      MdvxFieldCode::getEntryByAbbrev("WDIR", codeEntry);
      _setFieldName(fhdr, "Wdir", "Wind dirn", "degT",
		    "none", codeEntry.code);
      break;

    case Params::Z_FIELD:
      MdvxFieldCode::getEntryByAbbrev("HGT", codeEntry);
      _setFieldName(fhdr, "z", "Geoptential height", "m",
		    "none", codeEntry.code);
      break;

    case Params::DIVERGENCE_FIELD:
      MdvxFieldCode::getEntryByAbbrev("ABS D", codeEntry);
      _setFieldName(fhdr, "divergence", "divergence", "/s",
		    "none", codeEntry.code);
      break;

    case Params::PRESSURE_FIELD:
      MdvxFieldCode::getEntryByAbbrev("PRES", codeEntry);
      _setFieldName(fhdr, "pressure", "Absolute pressure", "hPa",
		    "none", codeEntry.code);
      break;

    case Params::RH_FIELD:
      MdvxFieldCode::getEntryByAbbrev("R H", codeEntry);
      _setFieldName(fhdr, "RH", "Relative humidity", "%",
		    "none", codeEntry.code);
      break;

    case Params::DEWPT_FIELD:
      MdvxFieldCode::getEntryByAbbrev("DPT", codeEntry);
      _setFieldName(fhdr, "DewPt", "Dew point", "C",
		    "none", codeEntry.code);
      break;

    case Params::TURB_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Turb", "Turbulence severity - ITFA", "",
		    "none", codeEntry.code);
      break;
      
    case Params::ICING_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Icing", "Icing severity index", "",
		    "none", codeEntry.code);
      break;

      // ITFA debug 3-d fields
      
    case Params::BROWN1_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Brown1", "Brown1", "1/s",
		    "none", codeEntry.code);
      break;
      
    case Params::BROWN2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Brown2", "Brown2", "m2/s3",
		    "none", codeEntry.code);
      break;
      
    case Params::CCAT_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Ccat", "Ccat", "1/s3",
		    "none", codeEntry.code);
      break;
      
    case Params::COLSON_PANOFSKY_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Colson_panofsky", "Colson_panofsky", "m2/s2",
		    "none", codeEntry.code);
      break;
      
    case Params::DEF_SQR_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Def_sqr", "Def_gqr", "1/s",
		    "none", codeEntry.code);
      break;
      
    case Params::ELLROD1_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Ellrod1", "Ellrod1", "1/s2",
		    "none", codeEntry.code);
      break;
      
    case Params::ELLROD2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Ellrod2", "Ellrod2", "1/s2",
		    "none", codeEntry.code);
      break;
      
    case Params::DUTTON_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Dutton", "Dutton", "1/s",
		    "none", codeEntry.code);
      break;
      
    case Params::ENDLICH_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Endlich", "Endlich", "deg/s",
		    "none", codeEntry.code);
      break;
      
    case Params::HSHEAR_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Hshear", "Hshear", "1/s",
		    "none", codeEntry.code);
      break;
      
    case Params::LAZ_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Laz", "Laz", "m2/s2",
		    "none", codeEntry.code);
      break;
      
    case Params::PVORT_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Pvort", "Pvort", "pvu",
		    "none", codeEntry.code);
      break;
      
    case Params::PVORT_GRADIENT_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Pvort_gradient", "Pvort_gradient", "pvu/m",
		    "none", codeEntry.code);
      break;
      
    case Params::NGM1_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Ngm1", "Ngm1", "m/s2",
		    "none", codeEntry.code);
      break;
      
    case Params::NGM2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Ngm2", "Ngm2", "K/s2",
		    "none", codeEntry.code);
      break;
      
    case Params::RICHARDSON_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Richardson", "Richardson", "",
		    "none", codeEntry.code);
      break;
      
    case Params::RIT_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Rit", "Rit", "m2/s2",
		    "none", codeEntry.code);
      break;
      
    case Params::SAT_RI_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Sat_ri", "Sat_ri", "",
		    "none", codeEntry.code);
      break;
      
    case Params::STABILITY_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Stability", "Stability", "",
		    "none", codeEntry.code);
      break;
      
    case Params::VORT_SQR_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Vort_sqr", "Vort_sqr", "1/s2",
		    "none", codeEntry.code);
      break;
      
    case Params::VWSHEAR_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Vwshear", "Vwshear", "1/s",
		    "none", codeEntry.code);
      break;
      
    case Params::TKE_KH3_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "TkeKh3", "TkeKh3", "m2/s2",
		    "none", codeEntry.code);
      break;
      
    case Params::TKE_KH4_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "TkeKh4", "TkeKh4", "m2/s2",
		    "none", codeEntry.code);
      break;
      
    case Params::TKE_KH5_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "TkeKh5", "TkeKh5", "m(2/3)/s",
		    "none", codeEntry.code);
      break;
      
      // raw 2-d fields

    case Params::START_2D_FIELDS:
      break;

    case Params::GROUND_T_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "ground_t", "Ground temperature",
		    "K", "none", codeEntry.code);
      break;

    case Params::RAIN_CON_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "rain_con", "Rain accumulation - convective",
		    "cm", "none", codeEntry.code);
      break;

    case Params::RAIN_NON_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "rain_non", "Rain accumulation - non-convective",
		    "cm", "none", codeEntry.code);
      break;

    case Params::TERRAIN_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "terrain", "Terrain height",
		    "m", "none", codeEntry.code);
      break;

    case Params::CORIOLIS_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "coriolis", "Coriolis factor",
		    "1/s", "none", codeEntry.code);
      break;

    case Params::RES_TEMP_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "res_temp", "Infinite res slab temp",
		    "K", "none", codeEntry.code);
      break;

    case Params::LAND_USE_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "land_use", "Land use category",
		    "1/s", "none", codeEntry.code);
      break;

    case Params::SNOWCOVR_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "snowcovr", "Snow cover flags",
		    "", "none", codeEntry.code);
      break;

    case Params::TSEASFC_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "tseasfc", "Sea surface temperature",
		    "K", "none", codeEntry.code);
      break;

    case Params::PBL_HGT_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "pbl_hgt", "Pbl height",
		    "m", "none", codeEntry.code);
      break;

    case Params::REGIME_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "regime", "Pbl regime",
		    "1/s", "none", codeEntry.code);
      break;

    case Params::SHFLUX_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "shflux", "Sensible heat flux",
		    "W/m^2", "none", codeEntry.code);
      break;

    case Params::LHFLUX_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "lhflux", "Latent heat flux",
		    "W/m^2", "none", codeEntry.code);
      break;

    case Params::UST_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "ust", "Frictional velocity",
		    "m/s", "none", codeEntry.code);
      break;

    case Params::SWDOWN_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "swdown", "Surface downward shortwave radiation",
		    "W/m^2", "none", codeEntry.code);
      break;

    case Params::LWDOWN_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "lwdown", "Surface downward longwave radiation",
		    "W/m^2", "none", codeEntry.code);
      break;

    case Params::SOIL_T_1_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "soil_t_1", "Soil temperature in layer 1",
		    "K", "none", codeEntry.code);
      break;

    case Params::SOIL_T_2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "soil_t_2", "Soil temperature in layer 2",
		    "K", "none", codeEntry.code);
      break;

    case Params::SOIL_T_3_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "soil_t_3", "Soil temperature in layer 3",
		    "K", "none", codeEntry.code);
      break;

    case Params::SOIL_T_4_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "soil_t_4", "Soil temperature in layer 4",
		    "K", "none", codeEntry.code);
      break;

    case Params::SOIL_T_5_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "soil_t_5", "Soil temperature in layer 5",
		    "K", "none", codeEntry.code);
      break;

    case Params::SOIL_T_6_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "soil_t_6", "Soil temperature in layer 6",
		    "K", "none", codeEntry.code);
      break;

    case Params::T2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "T2", "2-meter Temperature",
                    "K", "none", codeEntry.code);
      break;

    case Params::Q2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Q2", "2-meter Mixing Ratio",
                    "kg kg{-1}", "none", codeEntry.code);
      break;

    case Params::U10_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "U10", "10-meter U Component",
                    "m s{-1}", "none", codeEntry.code);
      break;

    case Params::V10_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "V10", "10-meter V Component",
                    "m s{-1}", "none", codeEntry.code);
      break;

    case Params::SFCRNOFF_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "sfcrnoff", "Surface runoff",
		    "mm", "none", codeEntry.code);
      break;

    case Params::SWFRAC_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "swfrac", "Soil moisture ratio from saturation",
		      "", "none", codeEntry.code);
      break;

    case Params::SUNALT_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "sunalt", "Sun altitude", "deg", "none", codeEntry.code);
      break;

    case Params::SUNAZM_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "sunazm", "Sun azimuth", "deg", "none", codeEntry.code);
      break;

    case Params::MOONALT_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "moonalt", "Moon altitude", "deg", "none", codeEntry.code);
      break;

    case Params::MOONAZM_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "moonazm", "Moon azimuth", "deg", "none", codeEntry.code);
      break;

    case Params::SUNILL_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "sunill", "Sun illuminance", "lux", "none", codeEntry.code);
      break;

    case Params::MOONILL_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "moonill", "Moon illuminance", "lux", "none", codeEntry.code);
      break;

    case Params::TOTALILL_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "totalill", "Total illuminance", "lux", "none", codeEntry.code);
      break;

    case Params::CLWI_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "clwi", "Cloud liquid water", "kg/m2", "none", codeEntry.code);
      break;

    case Params::RNWI_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "rnwi", "Cloud rain water", "kg/m2", "none", codeEntry.code);
      break;

    case Params::ICEI_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "icei", "Cloud ice water", "kg/m2", "none", codeEntry.code);
      break;

    case Params::SNOWI_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "snowi", "Cloud snow water", "kg/m2", "none", codeEntry.code);
      break;

    case Params::PWV_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "pwv", "Cloud precipitable water", "kg/m2", "none", codeEntry.code);
      break;

    case Params::SUN_BTW:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "sun_btw", "Sun - beginning of twilight", "hr", "none", codeEntry.code);
      break;

    case Params::SUN_ETW:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "sun_etw", "Sun - end of twilight", "hr", "none", codeEntry.code);
      break;

    case Params::SUN_ABTW:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "sun_abtw", "Sun - azim. beginning of twilight", "deg", "none", codeEntry.code);
      break;

    case Params::SUN_AETW:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "sun_aetw", "Sun - azim. end of twilight", "deg", "none", codeEntry.code);
      break;

    case Params::SUN_RISE:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "sun_rise", "Sun - rise time", "hr", "none", codeEntry.code);
      break;

    case Params::SUN_SET:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "sun_set", "Sun - set time", "hr", "none", codeEntry.code);
      break;

    case Params::SUN_ARIS:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "sun_aris", "Sun - azim. rise", "deg", "none", codeEntry.code);
      break;

    case Params::SUN_ASET:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "sun_aset", "Sun - azim. set", "deg", "none", codeEntry.code);
      break;

    case Params::MOON_RIS:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "moon_ris", "Moon - rise", "hr", "none", codeEntry.code);
      break;

    case Params::MOON_SET:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "moon_set", "Moon - set", "hr", "none", codeEntry.code);
      break;

    case Params::MOON_ARI:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "moon_ari", "Moon - azim. rise", "deg", "none", codeEntry.code);
      break;

    case Params::MOON_ASE:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "moon_ase", "Moon - azim. set", "deg", "none", codeEntry.code);
      break;

    case Params::NLAT_FIELD:
      MdvxFieldCode::getEntryByAbbrev("NLAT", codeEntry);
      _setFieldName(fhdr, "latitude", "North latitude", "deg", "none", codeEntry.code);
      break;

    case Params::ELON_FIELD:
      MdvxFieldCode::getEntryByAbbrev("ELON", codeEntry);
      _setFieldName(fhdr, "longitude", "East longitude", "deg", "none", codeEntry.code);
      break;

      // derived 2-d fields

    case Params::FZLEVEL_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "FZLevel", "freezing level",
		    "FL", "none", codeEntry.code);
      break;

    case Params::RAIN_TOTAL_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "rain_total", "Rain accumulation - con + non",
		    "mm", "none", codeEntry.code);
      break;

    case Params::RH2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "RH2", "Relative humidity at 2 meters", "%",
                    "none", codeEntry.code);
      break;

    case Params::DEWPT2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "DewPt2", "Dew point at 2 meters", "C",
                    "none", codeEntry.code);
      break;

    case Params::MSLP2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("PRMSL", codeEntry);
      _setFieldName(fhdr, "pressure2", "Pressure reduced to MSL at 2 meters", "Pa",
                    "none", codeEntry.code);
      break;

    } // switch
    
    // vlevel header
    
    if (_params._output_fields[out_field].name > Params::START_2D_FIELDS) {

      vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
      vhdr.level[0] = 0;

    } else {

      switch (_params.output_levels) {
      case Params::NATIVE_SIGMA_LEVELS:
	for (int iz = 0; iz < fhdr.nz; iz++) {
	  vhdr.type[iz] = Mdvx::VERT_TYPE_SIGMA_P;
	  vhdr.level[iz] = inData.get_halfSigma(iz);
	}
	break;
      case Params::FLIGHT_LEVELS:
	for (int iz = 0; iz < fhdr.nz; iz++) {
	  vhdr.type[iz] = Mdvx::VERT_FLIGHT_LEVEL;
	  vhdr.level[iz] = _params._flight_levels[iz];
	}
	break;
      case Params::PRESSURE_LEVELS:
	for (int iz = 0; iz < fhdr.nz; iz++) {
	  vhdr.type[iz] = Mdvx::VERT_TYPE_PRESSURE;
	  vhdr.level[iz] = _params._pressure_levels[iz];
	}
	break;
      case Params::HEIGHT_LEVELS:
	for (int iz = 0; iz < fhdr.nz; iz++) {
	  vhdr.type[iz] = Mdvx::VERT_TYPE_Z;
	  vhdr.level[iz] = _params._height_levels[iz];
	}
	break;
      }
      
    }

    // create field

    MdvxField *field = new MdvxField(fhdr, vhdr, NULL, true);

    // add field to mdvx object

    _mdvx.addField(field);
    
  } // out_field

  // set the output format

  if (_params.output_path_in_forecast_format) {
    _mdvx.setWriteAsForecast();
  }

}

////////////////////////////////////////////////////
// _setFieldName()
//
// Sets the field name, units etc
//

void OutputFile::_setFieldName(Mdvx::field_header_t &fhdr,
			       const char *name,
			       const char *name_long,
			       const char *units,
			       const char *transform,
			       const int field_code)
  
{

  STRncopy(fhdr.field_name, name, MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr.field_name_long, name_long, MDV_LONG_FIELD_LEN);
  STRncopy(fhdr.units, units, MDV_UNITS_LEN);
  STRncopy(fhdr.transform, transform, MDV_TRANSFORM_LEN);
  fhdr.field_code = field_code;
  
}


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
#include "WRFData.hh"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <toolsa/udatetime.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxFieldCode.hh>
using namespace std;

//////////////
// Constructor

OutputFile::OutputFile(const string &prog_name,
		       const Params &params,
		       time_t model_time,
		       time_t forecast_time,
		       int forecast_delta,
		       WRFData &inData,
		        Params::afield_name_map_t *field_name_map) :
  _progName(prog_name), _params(params), _field_name_map(field_name_map)

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
  
  // set precip fields to log scale if they are INT8
  
  MdvxField *rainc = _mdvx.getField("rainc");
  if (rainc &&
      rainc->getFieldHeader().encoding_type == Mdvx::ENCODING_INT8) {
    rainc->transform2Log();
  }
  MdvxField *rainnc = _mdvx.getField("rainnc");
  if (rainnc &&
      rainnc->getFieldHeader().encoding_type == Mdvx::ENCODING_INT8) {
    rainnc->transform2Log();
  }
  MdvxField *rain_total = _mdvx.getField("rain_total");
  if (rain_total &&
      rain_total->getFieldHeader().encoding_type == Mdvx::ENCODING_INT8) {
    rain_total->transform2Log();
  }

  // write to directory

  _mdvx.setAppName(_progName);
  _mdvx.setWriteLdataInfo();
  if (_mdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - OutputFile::writeVol" << endl;
    cerr << _mdvx.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    fprintf(stderr, "Wrote file: %s\n", _mdvx.getPathInUse().c_str());
  }

  return 0;
  
}

////////////////////////////////////////////////////
// _initMdvx()
//
// Initialize the MDVX object
//

void OutputFile::_initMdvx(time_t model_time, time_t forecast_time,
			   int forecast_delta, WRFData &inData)
  
{

  // set the master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  mhdr.time_gen = model_time;
  mhdr.time_begin = forecast_time;
  mhdr.time_end = forecast_time;
  mhdr.time_centroid = forecast_time;
  mhdr.time_expire = forecast_time;
    
  mhdr.forecast_time = forecast_time;
  mhdr.forecast_delta = forecast_delta;

  mhdr.num_data_times = 1;
  mhdr.data_dimension = 3;
  
  mhdr.data_collection_type = Mdvx::DATA_FORECAST;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_WRF_ETA;

  switch (_params.output_levels) {
  case Params::NATIVE_VERTICAL_LEVELS:
    mhdr.vlevel_type = Mdvx::VERT_TYPE_WRF_ETA;
    mhdr.max_nz = inData.getNEta();
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
    mhdr.max_nx = inData.getNLon();
    mhdr.max_ny = inData.getNLat();
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

  //
  // Add the base state to the user_data_fl32 variable. This may
  // be useful down the road if the data are going to be put onto sigma-z
  // (or just plain Z) height co-ordinates. Niles Oien 9/23/2005
  //
  /*   -- no Mrf Array in the WRF data
  fl32 **mrf = inData.getMrfArray();
  
  mhdr.user_data_fl32[0] = mrf[4][1]; // Non-hydrostatic base state sea-level pressure (Pa)
  mhdr.user_data_fl32[1] = mrf[4][2]; // Non-hydrostatic base state sea-level temperature (K)
  mhdr.user_data_fl32[2] = mrf[4][3]; // Non-hydrostatic base state lapse rate d(T)/d(ln P)
  mhdr.user_data_fl32[3] = mrf[4][4]; // Non-hydrostatic base state isothermal stratospheric temperature (K)
  mhdr.user_data_fl32[4] = inData.get_pTop(); // Top pressure (Pa)
  */
  _mdvx.setMasterHeader(mhdr);
  
  // fill in field headers and vlevel headers
  
  // add fields

  _mdvx.clearFields();
  
  for (int out_field = 0; out_field < _params.output_fields_n; out_field++) {
    
    Mdvx::field_header_t fhdr;
    MEM_zero(fhdr);
    Mdvx::vlevel_header_t vhdr;
    MEM_zero(vhdr);

    fhdr.forecast_delta = forecast_time - model_time;
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
      switch (inData.getProjType()) {
      case WRFData::LAMBERT_CONF: {
	fhdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
	fhdr.proj_param[0] = inData.getTrueLat1();
	fhdr.proj_param[1] = inData.getTrueLat2();
	break;
      }
      case WRFData::STEREOGRAPHIC: {
	fhdr.proj_type = Mdvx::PROJ_POLAR_STEREO;
	fhdr.proj_param[0] = inData.getProjCenterLon();
	fhdr.proj_param[1] =
	  inData.getTrueLat1() >= 0 ? Mdvx::POLE_NORTH : Mdvx::POLE_SOUTH;
	fhdr.proj_param[2] = inData.calculate_central_scale();
	break;
      }
      case WRFData::MERCATOR: {
	fhdr.proj_type = Mdvx::PROJ_MERCATOR;
	break;
      }
      default: {
	fhdr.proj_type = Mdvx::PROJ_UNKNOWN;
      }
      } // endswitch - inData.getProjType()
      break;
    }
    } // switch output_projection
    
    if (_params.output_projection == Params::OUTPUT_PROJ_NATIVE) {

      fhdr.proj_origin_lat = inData.getProjCenterLat();
      fhdr.proj_origin_lon = inData.getProjCenterLon();
      fhdr.nx = inData.getNLon();
      fhdr.ny = inData.getNLat();
      fhdr.grid_dx = inData.getGridDx();
      fhdr.grid_dy = inData.getGridDy();
      fhdr.grid_minx = inData.getGridMinX();
      fhdr.grid_miny = inData.getGridMinY();

      if (_params._output_fields[out_field].name == Params::U_EDGE_FIELD)
	{
	  fhdr.nx++;
	  fhdr.grid_minx -= fhdr.grid_dx / 2.0;
	}

	if(_params._output_fields[out_field].name == Params::V_EDGE_FIELD)
	  {
	fhdr.ny++;
	fhdr.grid_miny -= fhdr.grid_dy / 2.0;
	  }

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

    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_WRF_ETA;
    switch (_params.output_levels) {
    case Params::NATIVE_VERTICAL_LEVELS:
      fhdr.vlevel_type = Mdvx::VERT_TYPE_WRF_ETA;
      fhdr.nz = inData.getNEta();
      fhdr.grid_minz = inData.get_halfEta(0);
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
    
    fhdr.bad_data_value = WRFData::MISSING_DOUBLE;
    fhdr.missing_data_value = WRFData::MISSING_DOUBLE;

    fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
    fhdr.data_element_nbytes = sizeof(fl32);
    if (_params._output_fields[out_field].name > Params::START_2D_FIELDS) {
      fhdr.volume_size = fhdr.nx * fhdr.ny * sizeof(fl32);
    } else {
      fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
    }

    switch (_params._output_fields[out_field].name) {

      MdvxFieldCode::entry_t codeEntry;
      
      // raw 3-d fields on cross (center) points

    case Params::U_FIELD:

      MdvxFieldCode::getEntryByAbbrev("U GRD", codeEntry);
      if (_params.wind_speed_in_knots) {
	_setFieldName(fhdr, _params._output_fields[out_field].name, "kts",
		      "none", codeEntry.code);
      } else {
	_setFieldName(fhdr, _params._output_fields[out_field].name, "m/s",
		      "none", codeEntry.code);
      }
      break;

    case Params::V_FIELD:
      MdvxFieldCode::getEntryByAbbrev("V GRD", codeEntry);
      if (_params.wind_speed_in_knots) {
	_setFieldName(fhdr, _params._output_fields[out_field].name, "kts",
		      "none", codeEntry.code);
      } else {
	_setFieldName(fhdr,_params._output_fields[out_field].name, "m/s",
		      "none", codeEntry.code);
      }
      break;
      
    case Params::Q_FIELD:
      MdvxFieldCode::getEntryByAbbrev("MIXR", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "kg/kg",
		    "none", codeEntry.code);
      break;
      
    case Params::CLW_FIELD:
      MdvxFieldCode::getEntryByAbbrev("CLWMR", codeEntry);
      _setFieldName(fhdr,_params._output_fields[out_field].name, "kg/kg",
		    "none", codeEntry.code);
      break;

    case Params::RNW_FIELD:
      MdvxFieldCode::getEntryByAbbrev("RWMR", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "kg/kg",
		    "none", codeEntry.code);
      break;

    case Params::ICE_FIELD:
      MdvxFieldCode::getEntryByAbbrev("ICMR", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "kg/kg",
		    "none", codeEntry.code);
      break;

    case Params::QNRAIN_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "kg-1",
		    "none", codeEntry.code);
      break;

    case Params::QNCLOUD_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "kg-1",
		    "none", codeEntry.code);
      break;

    case Params::SNOW_FIELD:
      MdvxFieldCode::getEntryByAbbrev("SNMR", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "kg/kg",
		    "none", codeEntry.code);
      break;

    case Params::GRAUPEL_FIELD:
      MdvxFieldCode::getEntryByAbbrev("GRMR", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "kg/kg",
		    "none", codeEntry.code);
      break;
      /*
    case Params::NCI_FIELD:
      MdvxFieldCode::getEntryByAbbrev("NCIP", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "#/m^3",
		    "none", codeEntry.code);
      break;
      
    case Params::RAD_TEND_FIELD:
      MdvxFieldCode::getEntryByAbbrev("TTRAD", codeEntry);
      _setFieldName(fhdr,_params._output_fields[out_field].name,
		    "K/day", "none", codeEntry.code);
      break;
      */
    case Params::W_FIELD:
      MdvxFieldCode::getEntryByAbbrev("DZDT", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "m/s",
		    "none", codeEntry.code);
      break;

    case Params::P_FIELD:
      MdvxFieldCode::getEntryByAbbrev("PRESA", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "Pa",
		    "none", codeEntry.code);
      break;

    case Params::PB_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "Pa",
		    "none", codeEntry.code);
      break;

    case Params::PHB_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "m2/s2",
		    "none", codeEntry.code);
      break;

    case Params::PH_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "m2/s2",
		    "none", codeEntry.code);
      break;

    case Params::DNW_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "",
		    "none", codeEntry.code);
      break;

    case Params::MUB_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "Pa",
		    "none", codeEntry.code);
      break;

    case Params::MU_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "Pa",
		    "none", codeEntry.code);
      break;

    case Params::REFL_10CM_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "dBZ",
		    "none", codeEntry.code);
      break;

      // raw 3-d fields on edge points

    case Params::U_EDGE_FIELD:

      MdvxFieldCode::getEntryByAbbrev("U GRD", codeEntry);
      if (_params.wind_speed_in_knots) {
	_setFieldName(fhdr, _params._output_fields[out_field].name, "kts",
		      "none", codeEntry.code);
      } else {
	_setFieldName(fhdr,_params._output_fields[out_field].name, "m/s",
		      "none", codeEntry.code);
      }
      break;

    case Params::V_EDGE_FIELD:

      MdvxFieldCode::getEntryByAbbrev("V GRD", codeEntry);
      if (_params.wind_speed_in_knots) {
	_setFieldName(fhdr, _params._output_fields[out_field].name, "kts",
		      "none", codeEntry.code);
      } else {
	_setFieldName(fhdr, _params._output_fields[out_field].name, "m/s",
		      "none", codeEntry.code);
      }
      break;

    case Params::ITFADEF_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "",
		    "none", codeEntry.code);
      break;

      // derived 3-d fields

    case Params::TK_FIELD:
      MdvxFieldCode::getEntryByAbbrev("TMP", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "K",
		    "none", codeEntry.code);
      break;
      
    case Params::TC_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr,_params._output_fields[out_field].name,"C",
     		    "none", codeEntry.code);
      break;
      
    case Params::WSPD_FIELD:
      MdvxFieldCode::getEntryByAbbrev("WIND", codeEntry);
      if (_params.wind_speed_in_knots) {
	_setFieldName(fhdr,_params._output_fields[out_field].name, "kts",
		      "none", codeEntry.code);
      } else {
	_setFieldName(fhdr, _params._output_fields[out_field].name, "m/s",
		      "none", codeEntry.code);
      }
      break;

    case Params::WDIR_FIELD:
      MdvxFieldCode::getEntryByAbbrev("WDIR", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "degT",
		    "none", codeEntry.code);
      break;
      /*
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
      */
    case Params::PRESSURE_FIELD:
      MdvxFieldCode::getEntryByAbbrev("PRES", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "hPa",
		    "none", codeEntry.code);
      break;

    case Params::RH_FIELD:
      MdvxFieldCode::getEntryByAbbrev("R H", codeEntry);
      _setFieldName(fhdr,_params._output_fields[out_field].name,  "%",
		    "none", codeEntry.code);
      break;

    case Params::SPEC_H_FIELD:
      MdvxFieldCode::getEntryByAbbrev("S H", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "kg/kg",
		    "none", codeEntry.code);
      break;

    case Params::DEWPT_FIELD:
      MdvxFieldCode::getEntryByAbbrev("DPT", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "C",
		    "none", codeEntry.code);
      break;
      /*
    case Params::TURB_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "",
		    "none", codeEntry.code);
      break;
      */
    case Params::ICING_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "",
		    "none", codeEntry.code);
      break;

    case Params::CLW_G_FIELD:
      MdvxFieldCode::getEntryByAbbrev("CLW_G", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "g/kg",
		    "none", codeEntry.code);
      break;

    case Params::RNW_G_FIELD:
      MdvxFieldCode::getEntryByAbbrev("RNW_G", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "g/kg",
		    "none", codeEntry.code);
      break;
      /*
    case Params::Q_G_FIELD:
      MdvxFieldCode::getEntryByAbbrev("Q_G", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "g/kg",
		    "none", codeEntry.code);
      break;
      */
    case Params::THETA_FIELD:
      MdvxFieldCode::getEntryByAbbrev("THETA", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "K",
		    "none", codeEntry.code);
      break;
      /*
    case Params::THETAE_FIELD:
      MdvxFieldCode::getEntryByAbbrev("THETAE", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "K",
		    "none", codeEntry.code);
      break;

    case Params::THETAV_FIELD:
      MdvxFieldCode::getEntryByAbbrev("THETAV", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "K",
		    "none", codeEntry.code);
      break;
      */
    case Params::DBZ_3D_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "dbz", "none", codeEntry.code);
      break;

    case Params::HGT_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "gpm", "none", codeEntry.code);
      break;

    case Params::GEO_POT_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m2 s-2", "none", codeEntry.code);
      break;

    case Params::Z_AGL_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m", "none", codeEntry.code);
      break;

    case Params::Q_G_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "g/kg", "none", codeEntry.code);
      break;

      //3D CAPE/CIN/etc for CIP/FIP
    case Params::CIN_3D_FIELD:
      MdvxFieldCode::getEntryByAbbrev("CIN_3D", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "J/kg", "none", codeEntry.code);
      break;

    case Params::CAPE_3D_FIELD:
      MdvxFieldCode::getEntryByAbbrev("CAPE_3D", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "J/kg", "none", codeEntry.code);
      break;

    case Params::LCL_3D_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m", "none", codeEntry.code);
      break;

    case Params::LFC_3D_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m", "none", codeEntry.code);
      break;

    case Params::EL_3D_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m", "none", codeEntry.code);
      break;




      // ITFA debug 3-d fields
      /*
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
      */      
      // raw 2-d fields

    case Params::START_2D_FIELDS:
      break;

    case Params::SOIL_T_1_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "K", "none", codeEntry.code);
      break;

    case Params::SOIL_T_2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "K", "none", codeEntry.code);
      break;

    case Params::SOIL_T_3_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "K", "none", codeEntry.code);
      break;

    case Params::SOIL_T_4_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr,_params._output_fields[out_field].name,
		    "K", "none", codeEntry.code);
      break;

    case Params::SOIL_T_5_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr,_params._output_fields[out_field].name,
		    "K", "none", codeEntry.code);
      break;
      /*
    case Params::SOIL_T_6_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "K", "none", codeEntry.code);
      break;
      */
    case Params::SOIL_M_1_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m^3/m^3", "none", codeEntry.code);
      break;

    case Params::SOIL_M_2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m^3/m^3", "none", codeEntry.code);
      break;

    case Params::SOIL_M_3_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m^3/m^3", "none", codeEntry.code);
      break;

    case Params::SOIL_M_4_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m^3/m^3", "none", codeEntry.code);
      break;

    case Params::SOIL_M_5_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m^3/m^3", "none", codeEntry.code);
      break;

    case Params::SOIL_AM_1_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m^3/m^3", "none", codeEntry.code);
      break;

    case Params::SOIL_AM_2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m^3/m^3", "none", codeEntry.code);
      break;

    case Params::SOIL_AM_3_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m^3/m^3", "none", codeEntry.code);
      break;

    case Params::SOIL_AM_4_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m^3/m^3", "none", codeEntry.code);
      break;

    case Params::SOIL_AM_5_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr,_params._output_fields[out_field].name,
		    "m^3/m^3", "none", codeEntry.code);
      break;


    case Params::LAT_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr,_params._output_fields[out_field].name,
		    "Degrees", "none", codeEntry.code);
      break;	
	    
    case Params::LON_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "Degrees", "none", codeEntry.code);
      break;
	    
    case Params::GROUND_T_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "K", "none", codeEntry.code);
      break;

    case Params::RAINC_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "mm", "none", codeEntry.code);
      break;

    case Params::RAINNC_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr,_params._output_fields[out_field].name,
		    "mm", "none", codeEntry.code);
      break;

    case Params::TERRAIN_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m", "none", codeEntry.code);
      break;
      /*
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
      */
    case Params::LAND_USE_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "1/s", "none", codeEntry.code);
      break;

    case Params::SNOWCOVR_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "", "none", codeEntry.code);
      break;

    case Params::TSEASFC_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "K", "none", codeEntry.code);
      break;

    case Params::PBL_HGT_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m", "none", codeEntry.code);
      break;
      /*
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
      */
    case Params::T2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "K", "none", codeEntry.code);
      break;

    case Params::Q2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr,_params._output_fields[out_field].name,
		    "kg kg{-1}", "none", codeEntry.code);
      break;

    case Params::U10_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m s{-1}", "none", codeEntry.code);
      break;

    case Params::V10_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr,_params._output_fields[out_field].name,
		    "m s{-1}", "none", codeEntry.code);
      break;

    case Params::SNOWH_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m", "none", codeEntry.code);
      break;

    case Params::SFC_PRES_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "Pa", "none", codeEntry.code);
      break;

    case Params::LAND_MASK_FIELD:
       MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
       _setFieldName(fhdr, _params._output_fields[out_field].name,
 		    "", "none", codeEntry.code);
       break;

    case Params::TH2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "K", "none", codeEntry.code);
      break;

    case Params::HFX_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "W m-2", "none", codeEntry.code);
      break;

    case Params::LH_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "W m-2", "none", codeEntry.code);
      break;

    case Params::SNOW_WE_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "kg m-2", "none", codeEntry.code);
      break;

    case Params::SNOW_NC_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "kg m-2", "none", codeEntry.code);
      break;

    case Params::GRAUPEL_NC_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "kg m-2", "none", codeEntry.code);
      break;

    case Params::SOIL_TYPE_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "", "none", codeEntry.code);
      break;
      /*
    case Params::WEASD_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "weasd", "Water equivalent snow depth",
		    "mm", "none", codeEntry.code);
      break;
      */
      // derived 2-d fields

    case Params::FZLEVEL_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "FL", "none", codeEntry.code);
      break;

    case Params::RAIN_TOTAL_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "mm", "none", codeEntry.code);
      break;

    case Params::T2C_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "C", "none", codeEntry.code);
      break;

    case Params::DBZ_2D_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "dbz", "none", codeEntry.code);
      break;
      
    case Params::RH2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "%",
		    "none", codeEntry.code);
      break;

    case Params::SPEC_H_2M_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "kg/kg",
		    "none", codeEntry.code);
      break;


    case Params::LCL_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m", "none", codeEntry.code);
      break;

    case Params::LFC_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m", "none", codeEntry.code);
      break;

    case Params::EL_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "m", "none", codeEntry.code);
      break;

    case Params::CIN_FIELD:
      MdvxFieldCode::getEntryByAbbrev("CIN", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "J/kg", "none", codeEntry.code);
      break;

    case Params::CAPE_FIELD:
      MdvxFieldCode::getEntryByAbbrev("CAPE", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name,
		    "J/kg", "none", codeEntry.code);
      break;

      /*
    case Params::DEWPT2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "DewPt2", "Dew point at 2 meters", "C",
		    "none", codeEntry.code);
      break;
      */
    case Params::WSPD10_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      if (_params.wind_speed_in_knots) {
	_setFieldName(fhdr,_params._output_fields[out_field].name, "kts",
		      "none", codeEntry.code);
      } else {
	_setFieldName(fhdr, _params._output_fields[out_field].name, "m/s",
		      "none", codeEntry.code);
      }
      break;

    case Params::WDIR10_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "degT",
		    "none", codeEntry.code);
      break;
      /*
    case Params::MSLP2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "pressure2", "Mean sea level pressure at 2 meters", "hPa",
		    "none", codeEntry.code);
      break;

    case Params::Q2_G_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, "Q2_G", "2-meter Mixing Ratio in g/kg",
		    "g kg{-1}", "none", codeEntry.code);
      break;

    case Params::THETA2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("THETA2", codeEntry);
      _setFieldName(fhdr, "THETA2", "2 meter Potential Temperature", "K",
		    "none", codeEntry.code);
      break;

    case Params::THETAE2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("THETAE2", codeEntry);
      _setFieldName(fhdr, "THETAE2", "2 meter Equivalent Potential Temperature", "K",
		    "none", codeEntry.code);
      break;

    case Params::THETAV2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("THETAV2", codeEntry);
      _setFieldName(fhdr, "THETAV2", "2 meter Virtual Potential Temperature", "K",
		    "none", codeEntry.code);
      break;
      */

      // GEOGRID 2-d fields
    case Params::LANDUSEF_1_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "category",
		    "none", codeEntry.code);
      break;

    case Params::LANDUSEF_2_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "category",
		    "none", codeEntry.code);
      break;

    case Params::LANDUSEF_6_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, 
		    "category",
		    "none", codeEntry.code);
      break;

    case Params::LANDUSEF_15_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "category",
		    "none", codeEntry.code);
      break;

    case Params::GREENFRAC_7_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "fraction",
		    "none", codeEntry.code);
      break;

    case Params::TWP_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "g/m2",
		    "none", codeEntry.code);
      break;
  
    case Params::VIL_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "none",
                    "none", codeEntry.code);
      break;


    case Params::RWP_FIELD:
      MdvxFieldCode::getEntryByAbbrev("UNDEFINED", codeEntry);
      _setFieldName(fhdr, _params._output_fields[out_field].name, "g/m2",
		    "none", codeEntry.code);
      break;

    case Params::TOTAL_FIELD_COUNT:
      break;

    } // switch
    
    // vlevel header
    if (_params._output_fields[out_field].name > Params::START_2D_FIELDS) 
      {
	if (_params._output_fields[out_field].name > Params::SOIL_AM_5_FIELD) 
	  {
	    vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
	    vhdr.level[0] = 0;
	  }
	else
	  {
	    vhdr.type[0] = Mdvx::VERT_SOIL;
	    switch (_params._output_fields[out_field].name) {

	    case Params::SOIL_M_1_FIELD:
	    case Params::SOIL_T_1_FIELD:
	    case Params::SOIL_AM_1_FIELD:	      
	      vhdr.level[0] = -1*inData.getSoilD1()/1000;
	      break;

	    case Params::SOIL_M_2_FIELD:
	    case Params::SOIL_T_2_FIELD:
	    case Params::SOIL_AM_2_FIELD:	      
	      vhdr.level[0] = -1*inData.getSoilD2()/1000;
	      break;

	    case Params::SOIL_M_3_FIELD:
	    case Params::SOIL_T_3_FIELD:
	    case Params::SOIL_AM_3_FIELD:	      
	      vhdr.level[0] = -1*inData.getSoilD3()/1000;
	      break;

	    case Params::SOIL_M_4_FIELD:
	    case Params::SOIL_T_4_FIELD:
	    case Params::SOIL_AM_4_FIELD:
	      vhdr.level[0] = -1*inData.getSoilD4()/1000;
	      break;

	    case Params::SOIL_M_5_FIELD:
	    case Params::SOIL_T_5_FIELD:
	    case Params::SOIL_AM_5_FIELD: 
	      vhdr.level[0] = -1*inData.getSoilD5()/1000;
	      break;
	    
	    default:
	      cerr << "ERROR: case not handled in vhdr handling of output fields\n";
	      break;
	    }
	  }
      } else {

      switch (_params.output_levels) {
      case Params::NATIVE_VERTICAL_LEVELS:
	for (int iz = 0; iz < fhdr.nz; iz++) {
	  vhdr.type[iz] = Mdvx::VERT_TYPE_WRF_ETA;
	  vhdr.level[iz] = inData.get_halfEta(iz);
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

////////////////////////////////////////////////////
// _setFieldName()
//
// Sets the field name, units etc from the map created from the config file
//

void OutputFile::_setFieldName(Mdvx::field_header_t &fhdr,
			       const Params::output_field_name_t &field_name_enum,
			       const char *units,
			       const char *transform,
			       const int field_code)	
  
{
  int i = static_cast<int>(field_name_enum);

  if ( ( _field_name_map[i].name == NULL) || (_field_name_map[i].long_name == NULL) )
    cerr << "ERROR - fieldnames for field: " << i << " not defined!" << endl;

  _setFieldName(fhdr, _field_name_map[i].name,
	        _field_name_map[i].long_name,
		units, transform, field_code);
}








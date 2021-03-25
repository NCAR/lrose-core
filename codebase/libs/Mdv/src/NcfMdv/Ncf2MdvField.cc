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

/////////////////////////////////////////////////////////////
// Ncf2MdvField.cc
//
// Sue Dettling, Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2008 
//
///////////////////////////////////////////////////////////////
//
// Ncf2MdvField class.
// Translate a CF NetCDF file to Mdvx object
//
///////////////////////////////////////////////////////////////////////

#include <toolsa/TaStr.hh>
#include <toolsa/TaPjg.hh>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/NcfMdv.hh>
#include <Mdv/Ncf2MdvField.hh>

// Constructor

Ncf2MdvField::Ncf2MdvField(bool debug,
                           time_t validTime,
                           int timeIndex,
                           time_t forecastTime,
                           int forecastDelta,
                           Nc3File *ncFile, Nc3Error *ncErr,
                           Nc3Var *var4Data,
                           Nc3Dim *tDim, Nc3Var *tVar,
                           Nc3Dim *zDim, Nc3Var *zVar,
                           Nc3Dim *yDim, Nc3Var *yVar,
                           Nc3Dim *xDim, Nc3Var *xVar,
                           bool readData /* = true */) :
  _debug(debug),
  _ncFile(ncFile),
  _ncErr(ncErr),
  _var4Data(var4Data),
  _readData(readData),
  _tDim(tDim), _tVar(tVar),
  _zDim(zDim), _zVar(zVar),
  _yDim(yDim), _yVar(yVar),
  _xDim(xDim), _xVar(xVar)
  
{

  MEM_zero(_fhdr);
  MEM_zero(_vhdr);
  
  _projType = Mdvx::PROJ_LATLON;
  _projVar = NULL;

  _validTime = validTime;
  _timeIndex = timeIndex;
  _fhdr.forecast_time = forecastTime;
  _fhdr.forecast_delta = forecastDelta;

  _dataType = _var4Data->type();
  _data = NULL;
  
}

//
// Destructor
//

Ncf2MdvField::~Ncf2MdvField()
{
  _clear();
}
  
// Clear the memory

void Ncf2MdvField::_clear()
{
  
}

////////////////////////////////
// create MdvxField
// pointer must be freed by calling routine
// returns NULL on failure

MdvxField *Ncf2MdvField::createMdvxField()

{

  if (_debug) {
    cerr << "Adding data field: " << _var4Data->name() << endl;
    cerr << "             time: " << DateTime::strm(_validTime) << endl;
  }
  
  // set name, units

  _setNamesAndUnits();

  // set projection info
  
  if (_setProjType()) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::createMdvxField");
    TaStr::AddStr(_errStr, "  Cannot find projection type, field:",
                  _var4Data->name());
    return NULL;
  }

  if (_setProjInfo()) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::createMdvxField");
    TaStr::AddStr(_errStr, "  Cannot load projection info, field:",
                  _var4Data->name());
    return NULL;
  }
  _proj.syncToFieldHdr(_fhdr);
  _fhdr.proj_type = _projType;

  // Set grid info

  if (_setGridDimensions()) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::createMdvxField");
    TaStr::AddStr(_errStr, "  Cannot set grid, field: ",
                  _var4Data->name());
    return NULL;
  }

  // set data in grid

  if (_setGridData()) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::createMdvxField");
    TaStr::AddStr(_errStr, "  Cannot set data, field: ",
                  _var4Data->name());
    return NULL;
  }

  // set MDV-specific attributes, which will only exist if the
  // netCDF file was derived from an MDV file

  _setMdvSpecific();

  // create MdvxField object

  if (_readData) {
    MdvxField *fld = new MdvxField(_fhdr, _vhdr, _data);
    // request compression if appropriate
    bool shuffle = false, compressed = false;
    int compressLevel = 0;
    if (_var4Data->get_compression_parameters(shuffle, 
                                              compressed,
                                              compressLevel) == 0) {
      fld->requestCompression(Mdvx::COMPRESSION_GZIP);
    }
    return fld;
  } else {
    MdvxField *fld = new MdvxField(_fhdr, _vhdr, NULL,
                                   false, false, false);
    return fld;
  }

}

///////////////////////////////
// handle error string

void Ncf2MdvField::clearErrStr()
{
  _errStr.clear();
  TaStr::AddStr(_errStr, "=====>>> Ncf2MdvField <<<=====");
  TaStr::AddStr(_errStr, "Time for error: ", DateTime::str());
  TaStr::AddStr(_errStr, "  Field name:", _var4Data->name());
}

/////////////////////////////////////////
// set the projection type
// returns 0 on success, -1 on failure

int Ncf2MdvField::_setProjType()

{

  // init

  _projVar = NULL;

  // find the grid mapping variable name
  
  Nc3Att *gridMapAtt = _var4Data->get_att(NcfMdv::grid_mapping);
  if (gridMapAtt != NULL) {
    
    string projVarName = _asString(gridMapAtt);
    delete gridMapAtt;

    // get the projection variable

    _projVar = _ncFile->get_var(projVarName.c_str());
    if (_projVar == NULL) {
      TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::_setProjType");
      TaStr::AddStr(_errStr, "  Cannot find grid mapping variable: ",
                    projVarName);
      return -1;
    }

  } else {

    // look for var with grid_mapping_name attribute
    
    for (int ivar = 0; ivar < _ncFile->num_vars(); ivar++) {
      Nc3Var* mappingVar = _ncFile->get_var(ivar);
      Nc3Att *mappingNameAtt = mappingVar->get_att(NcfMdv::grid_mapping_name);
      if (mappingNameAtt != NULL) {
        delete mappingNameAtt;
        _projVar = mappingVar;
        break;
      }
    }
    
  }

  if (_projVar == NULL) {
    // no grid mapping, assume latlon
    _projType = Mdvx::PROJ_LATLON;
    return 0;
  }
  

  // get the projection type
  
  Nc3Att *projTypeAtt = _projVar->get_att(NcfMdv::grid_mapping_name);
  if (projTypeAtt == NULL) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::_setProjType");
    TaStr::AddStr(_errStr, "  No grid_mapping_name, var: ",
                  _projVar->name());
    return -1;
  }
  string _projTypeStr = _asString(projTypeAtt);
  delete projTypeAtt;

  if (_projTypeStr.compare(NcfMdv::albers_conical_equal_area) == 0) {
    _projType = Mdvx::PROJ_ALBERS;
  } else if (_projTypeStr.compare(NcfMdv::azimuthal_equidistant) == 0) {
    _projType = Mdvx::PROJ_FLAT;
  } else if (_projTypeStr.compare(NcfMdv::lambert_azimuthal_equal_area) == 0) {
    _projType = Mdvx::PROJ_LAMBERT_AZIM;
  } else if (_projTypeStr.compare(NcfMdv::lambert_conformal_conic) == 0) {
    _projType = Mdvx::PROJ_LAMBERT_CONF;
  } else if (_projTypeStr.compare(NcfMdv::latitude_longitude) == 0) {
    _projType = Mdvx::PROJ_LATLON;
  } else if (_projTypeStr.compare(NcfMdv::polar_radar) == 0) {
    _projType = Mdvx::PROJ_POLAR_RADAR;
  } else if (_projTypeStr.compare(NcfMdv::polar_stereographic) == 0 ||
             _projTypeStr.compare("polar_sterographic") == 0) {
    _projType = Mdvx::PROJ_POLAR_STEREO;
  } else if (_projTypeStr.compare(NcfMdv::rotated_latitude_longitude) == 0) {
    _projType = Mdvx::PROJ_UNKNOWN;
  } else if (_projTypeStr.compare(NcfMdv::stereographic) == 0) {
    _projType = Mdvx::PROJ_OBLIQUE_STEREO;
  } else if (_projTypeStr.compare(NcfMdv::transverse_mercator) == 0) {
    _projType = Mdvx::PROJ_TRANS_MERCATOR;
  } else if (_projTypeStr.compare(NcfMdv::mercator) == 0) {
    _projType = Mdvx::PROJ_MERCATOR;
  } else if (_projTypeStr.compare(NcfMdv::vertical_perspective) == 0) {
    _projType = Mdvx::PROJ_VERT_PERSP;
  } else if (_projTypeStr.compare(NcfMdv::vertical_section) == 0) {
    _projType = Mdvx::PROJ_VSECTION;
  }

  if (_projType == Mdvx::PROJ_UNKNOWN) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::_setProjType");
    TaStr::AddStr(_errStr, "  Unknown projection: ", _projTypeStr);
    return -1;
  }

  return 0;

}

/////////////////////////////////////////
// set the projection info
// returns 0 on success, -1 on failure

int Ncf2MdvField::_setProjInfo()

{
  
  if (_projType == Mdvx::PROJ_LATLON) {
    return 0;
  }

  int iret = 0;

  double falseNorthing = 0.0;
  _setProjParam(NcfMdv::false_northing, falseNorthing);
  
  double falseEasting = 0.0;
  _setProjParam(NcfMdv::false_easting, falseEasting);
      
  switch (_projType) {

    case Mdvx::PROJ_ALBERS: {

      double origin_lon = 0.0;
      iret |= _setProjParam(NcfMdv::longitude_of_central_meridian,
                            origin_lon);

      double origin_lat = 0.0;
      iret |= _setProjParam(NcfMdv::latitude_of_projection_origin,
                            origin_lat);
      
      double lat1 = 0.0, lat2 = 0.0;
      iret |= _setProjParams(NcfMdv::standard_parallel, lat1, lat2);
      
      _proj.initAlbers(origin_lat, origin_lon,
                       lat1, lat2);
      _proj.setOffsetCoords(falseNorthing, falseEasting);
      
      break;

    }

    case Mdvx::PROJ_FLAT: {
      
      double origin_lon = 0.0;
      iret |= _setProjParam(NcfMdv::longitude_of_projection_origin,
                            origin_lon);
      
      double origin_lat = 0.0;
      iret |= _setProjParam(NcfMdv::latitude_of_projection_origin,
                            origin_lat);
     
      double earth_radius;
      if ( !_setProjParam(NcfMdv::earth_radius, earth_radius)) {
        TaPjg::setEarthRadius( earth_radius/1000);
      }
 
      _proj.initAzimEquiDist(origin_lat, origin_lon, 0.0);
      _proj.setOffsetCoords(falseNorthing, falseEasting);
 
      break;

    }

    case Mdvx::PROJ_LAMBERT_AZIM: {

      double origin_lon = 0.0;
      iret |= _setProjParam(NcfMdv::longitude_of_projection_origin,
                            origin_lon);

      double origin_lat = 0.0;
      iret |= _setProjParam(NcfMdv::latitude_of_projection_origin,
                            origin_lat);
      
      _proj.initLambertAzim(origin_lat, origin_lon);
      _proj.setOffsetCoords(falseNorthing, falseEasting);
      
      break;

    }

    case Mdvx::PROJ_LAMBERT_CONF: {

      double origin_lon = 0.0;
      iret |= _setProjParam(NcfMdv::longitude_of_central_meridian,
                            origin_lon);

      double origin_lat = 0.0;
      iret |= _setProjParam(NcfMdv::latitude_of_projection_origin,
                            origin_lat);
      
      double lat1 = 0.0, lat2 = 0.0;
      iret |= _setProjParams(NcfMdv::standard_parallel, lat1, lat2);

      double earth_radius;
      if ( !_setProjParam(NcfMdv::earth_radius, earth_radius)) {
        TaPjg::setEarthRadius( earth_radius/1000);
      }
   
      _proj.initLambertConf(origin_lat, origin_lon,
                            lat1, lat2);
      _proj.setOffsetCoords(falseNorthing, falseEasting);
      
      break;

    }

    case Mdvx::PROJ_LATLON: {

      _proj.initLatlon();

      break;

    }

    case Mdvx::PROJ_POLAR_RADAR: {
      
      double origin_lon = 0.0;
      iret |= _setProjParam(NcfMdv::longitude_of_projection_origin,
                            origin_lon);
      
      double origin_lat = 0.0;
      iret |= _setProjParam(NcfMdv::latitude_of_projection_origin,
                            origin_lat);
      
      _proj.initPolarRadar(origin_lat, origin_lon);
      
      break;

    }

    case Mdvx::PROJ_POLAR_STEREO: {

      double tangent_lon = 0.0;
      iret |= _setProjParam(NcfMdv::straight_vertical_longitude_from_pole,
                            tangent_lon);

      double tangent_lat = 0.0;
      iret |= _setProjParam(NcfMdv::latitude_of_projection_origin,
                            tangent_lat);
      
      double central_scale = 1.0;
      if (_setProjParam(NcfMdv::scale_factor_at_projection_origin,
                        central_scale) != 0) {
        double standard_parallel;
        if (_setProjParam(NcfMdv::standard_parallel,
                          standard_parallel) == 0) {
          central_scale =
            PjgPolarStereoMath::computeCentralScale(standard_parallel);
        }
      }
      
      Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH;
      if (tangent_lat < 0) {
        poleType = Mdvx::POLE_SOUTH;
      }
     
      double earth_radius;
      if ( !_setProjParam(NcfMdv::earth_radius, earth_radius)) {
        TaPjg::setEarthRadius( earth_radius/1000);
      }

      _proj.initPolarStereo(tangent_lon, poleType, central_scale);
      _proj.setOffsetCoords(falseNorthing, falseEasting);
     
      break;

    }

    case Mdvx::PROJ_OBLIQUE_STEREO: {

      double tangent_lon = 0.0;
      iret |= _setProjParam(NcfMdv::longitude_of_projection_origin,
                            tangent_lon);

      double tangent_lat = 0.0;
      iret |= _setProjParam(NcfMdv::latitude_of_projection_origin,
                            tangent_lat);
      
      double scale = 1.0;
      iret |= _setProjParam(NcfMdv::scale_factor_at_projection_origin,
                            scale);

      _proj.initStereographic(tangent_lat, tangent_lon, scale);
      _proj.setOffsetCoords(falseNorthing, falseEasting);
      
      break;

    }

    case Mdvx::PROJ_TRANS_MERCATOR: {

      double origin_lon = 0.0;
      iret |= _setProjParam(NcfMdv::longitude_of_central_meridian, origin_lon);

      double origin_lat = 0.0;
      iret |= _setProjParam(NcfMdv::latitude_of_projection_origin, origin_lat);
      
      double scale = 1.0;
      iret |= _setProjParam(NcfMdv::scale_factor_at_central_meridian, scale);

      double earth_radius;
      if ( !_setProjParam(NcfMdv::earth_radius, earth_radius))
        TaPjg::setEarthRadius( earth_radius/1000);

      _proj.initTransMercator(origin_lat, origin_lon, scale);
      _proj.setOffsetCoords(falseNorthing, falseEasting);
     
      break;

    }

    case Mdvx::PROJ_MERCATOR: {

      double origin_lon = 0.0;
      iret |= _setProjParam(NcfMdv::longitude_of_central_meridian, origin_lon);

      double origin_lat = 0.0;
      iret |= _setProjParam(NcfMdv::latitude_of_projection_origin, origin_lat);

      double earth_radius;
      if ( !_setProjParam(NcfMdv::earth_radius, earth_radius))  
        TaPjg::setEarthRadius( earth_radius/1000);

      _proj.initMercator(origin_lat, origin_lon);
      _proj.setOffsetCoords(falseNorthing, falseEasting);

      break;
    
   }

    case Mdvx::PROJ_VERT_PERSP: {

      double origin_lon = 0.0;
      iret |= _setProjParam(NcfMdv::longitude_of_projection_origin, origin_lon);

      double origin_lat = 0.0;
      iret |= _setProjParam(NcfMdv::latitude_of_projection_origin, origin_lat);
      
      double ppt_ht_m = 0.0;
      iret |= _setProjParam(NcfMdv::perspective_point_height, ppt_ht_m);
      double persp_radius = ppt_ht_m / 1000.0 + Pjg::EradKm;

      _proj.initVertPersp(origin_lat, origin_lon, persp_radius);
      _proj.setOffsetCoords(falseNorthing, falseEasting);
      
      break;

    }

    default: {
    }

  }

  if (iret) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::_setProjInfo");
    TaStr::AddStr(_errStr, "  Cannot set projection: ", _projTypeStr);
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////
// set a particular projection parameter value
// returns 0 on success, -1 on failure

int Ncf2MdvField::_setProjParam(const string &param_name,
                                double &param_val)

{

  Nc3Att *att = _projVar->get_att(param_name.c_str());
  
  if (att == NULL) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::_setProjParam");
    TaStr::AddStr(_errStr, "  Missing projection parameter: ", param_name);
    TaStr::AddStr(_errStr, "  Projection type: ", _projTypeStr);
    delete att;
    return -1;
  }
	
  param_val = att->as_double(0);
  delete att;
  return 0;

}

//////////////////////////////////////////////
// set a pair of projection parameter values
// if only one value is available, both params are set the same
// returns 0 on success, -1 on failure

int Ncf2MdvField::_setProjParams(const string &param_name,
                                 double &param_val_1,
                                 double &param_val_2)
  
{

  Nc3Att *att = _projVar->get_att(param_name.c_str());
  
  if (att == NULL) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::_setProjParams");
    TaStr::AddStr(_errStr, "  Missing projection parameter: ", param_name);
    TaStr::AddStr(_errStr, "  Projection type: ", _projTypeStr);
    delete att;
    return -1;
  }
  
  if (att->num_vals() == 1) {
    param_val_1 = att->as_double(0);
    param_val_2 = att->as_double(0);
  } else if (att->num_vals() == 2) {
    param_val_1 = att->as_double(0);
    param_val_2 = att->as_double(1);
  } else {
    delete att;
    return -1;
  }

  delete att;
  return 0;

}

///////////////////////////////////////////////////////////////
// Set names and units

void Ncf2MdvField::_setNamesAndUnits()
  
{

  string name = _var4Data->name();
  STRncopy(_fhdr.field_name, name.c_str(), MDV_SHORT_FIELD_LEN);

   //
   // TODO:  Check that this is a proper default action for long field name
   //
   if (  name.length() > MDV_SHORT_FIELD_LEN -1 ) {
      // long name is set to untruncated variable name
      STRncopy(_fhdr.field_name_long, name.c_str(), MDV_LONG_FIELD_LEN); 
   } else {
      // long name is set to netcdf long name attribute
      Nc3Att* longNameAtt = _var4Data->get_att(NcfMdv::long_name);
      if (longNameAtt != NULL) {
         STRncopy(_fhdr.field_name_long, _asString(longNameAtt).c_str(), MDV_LONG_FIELD_LEN);
        delete longNameAtt;
     }
   }

  Nc3Att* unitsAtt = _var4Data->get_att(NcfMdv::units);
  if (unitsAtt != NULL) {
    STRncopy(_fhdr.units, _asString(unitsAtt).c_str(), MDV_UNITS_LEN);
    delete unitsAtt;
  }

}

///////////////////////////////////////////////////////////////
// Set dimensions of the grid
//
// Returns 0 on success, -1 on failure

int Ncf2MdvField::_setGridDimensions()
  
{

  // x dimension
  
  size_t nx;
  double minx, dx;
  
  if(_setXYAxis("X", _xVar, "longitude", _xDim, nx, minx, dx)) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::_setGridDimensions");
    TaStr::AddStr(_errStr, "  Cannot set X dimension");
    return -1;
  }

  _fhdr.nx = nx;
  _fhdr.grid_minx = minx;
  _fhdr.grid_dx = dx;
  
  if (_projType == Mdvx::PROJ_LATLON && nx > 1) {
    _fhdr.proj_origin_lon = minx + dx * ((nx - 1.0) / 2.0);
  }

  // y dimension
  
  size_t ny;
  double miny, dy;
  
  if(_setXYAxis("Y", _yVar, "latitude", _yDim, ny, miny, dy)) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::_setGridDimensions");
    TaStr::AddStr(_errStr, "  Cannot set Y dimension");
    return -1;
  }

  _fhdr.ny = ny;
  _fhdr.grid_miny = miny;
  _fhdr.grid_dy = dy;

  if (_projType == Mdvx::PROJ_LATLON && ny > 1) {
    _fhdr.proj_origin_lat = miny + dy * ((ny - 1.0) / 2.0);
  }

  // z dimension

  _setZAxis();

  return 0;

}

///////////////////////////////////////////////////////////////
// Set an X or Y axis
// Returns 0 on success, -1 on failure

int Ncf2MdvField::_setXYAxis(const string &axisName,
                             const Nc3Var *axisVar,
                             const string &latlonStdName,
                             const Nc3Dim *axisDim,
                             size_t &nn,
                             double &minVal,
                             double &dVal)
  
{

  // x dimension
  
  Nc3Att *attxName = axisVar->get_att(NcfMdv::standard_name);
  if (attxName != NULL) {
    string stdName = _asString(attxName);
    delete attxName;
    if (stdName.compare(latlonStdName) == 0) {
      if (_projType != Mdvx::PROJ_LATLON) {
        TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::_setXYAxis");
        TaStr::AddStr(_errStr, "  ", axisName, false);
        TaStr::AddStr(_errStr, " dimension is of type ", latlonStdName);
        TaStr::AddStr(_errStr, "  Yet projection type is : ", _projTypeStr);
        return -1;
      }
    }
  }

  nn = axisDim->size();
  minVal = axisVar->as_double(0);
  if (nn > 1) {
    double maxVal = axisVar->as_double(nn-1);
    dVal = (maxVal - minVal) / (nn - 1.0);
  } else {
    dVal = 1.0;
  }
  
  if (_projType == Mdvx::PROJ_LATLON) {
    // for latlon, we are done, no units conversion required
    return 0;
  }

  // get units

  Nc3Att *attUnits = axisVar->get_att(NcfMdv::units);
  if (attUnits == NULL) {
    return 0;
  }
  string units = _asString(attUnits);
  delete attUnits;

  if (units.compare("km") == 0) {
    // already km
    return 0;
  }

  if (units.find("deg") != string::npos) {
    // probably degrees, as in radar data in polar coords
    return 0;
  }

  // convert to km

  double kmMult = _getKmMult(units);
  minVal *= kmMult;
  dVal *= kmMult;

  return 0;

}

///////////////////////////////////////////////////////////////
// Set Z axis
// Returns 0 on success, -1 on failure

void Ncf2MdvField::_setZAxis()
  
{

  if (_zVar == NULL) {
    _fhdr.nz = 1;
    _fhdr.grid_minz = 0;
    _fhdr.grid_dz = 1;
    _fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    _vhdr.type[0] =  Mdvx::VERT_TYPE_SURFACE;
    return;
  }

  // get attributes

  string standardName;
  Nc3Att *standardNameAtt = _zVar->get_att(NcfMdv::standard_name);
  if (standardNameAtt != NULL) {
    standardName = _asString(standardNameAtt);
    delete standardNameAtt;
  }
  
  string longName;
  Nc3Att *longNameAtt = _zVar->get_att(NcfMdv::long_name);
  if (longNameAtt != NULL) {
    longName = _asString(longNameAtt);
    delete longNameAtt;
  }
  
  string units;
  Nc3Att *unitsAtt = _zVar->get_att(NcfMdv::units);
  if (unitsAtt != NULL) {
    units = _asString(unitsAtt);
    delete unitsAtt;
  }

  // try to set vlevel type

  Mdvx::vlevel_type_t vlevelType = Mdvx::VERT_TYPE_Z;
  if (!longName.compare("surface")) {
    vlevelType = Mdvx::VERT_TYPE_SURFACE;
  } else if (!standardName.compare("atmosphere_sigma_coordinate")) {
    vlevelType = Mdvx::VERT_TYPE_SIGMA_P;
  } else if (!units.compare("mb") ||
             !units.compare("hPa") ||
             !longName.compare("pressure_levels") ||
             !standardName.compare("air_pressure")) {
    vlevelType = Mdvx::VERT_TYPE_PRESSURE;
  } else if (!units.compare("km") ||
             !units.compare("ft") ||
             !standardName.compare("altitude")) {
    vlevelType = Mdvx::VERT_TYPE_Z;
  } else if (!longName.compare("model eta levels")) {
    vlevelType = Mdvx::VERT_TYPE_ETA;
  } else if (!units.compare("degree_Kelvin") ||
             !standardName.compare("isentropic surface")) {
    vlevelType = Mdvx::VERT_TYPE_THETA;
  } else if (!units.compare("degree") ||
             !units.compare("deg") ||
             !standardName.compare("elevation angles")) {
    vlevelType = Mdvx::VERT_TYPE_ELEV;
  } else if (!units.compare("100 ft") ||
             !longName.compare("Flight levels in 100s of feet")) {
    vlevelType = Mdvx::VERT_FLIGHT_LEVEL;
  }
  _fhdr.vlevel_type = vlevelType;
             
  // load up raw coord values

  size_t nz = _zDim->size();
  double minz = _zVar->as_double(0);
  double dz = 1.0;
  if (nz > 1) {
    double maxz = _zVar->as_double(nz-1);
    dz = (maxz - minz) / (nz - 1.0);
  }

  TaArray<double> vlevels_;
  double *vlevels = vlevels_.alloc(nz);
  for (size_t ii = 0; ii < nz; ii++) {
    vlevels[ii] = _zVar->as_double(ii);
  }

  _fhdr.nz = nz;
  _fhdr.grid_minz = minz;
  _fhdr.grid_dz = dz;

  for (size_t ii = 0; ii < nz; ii++) {
    _vhdr.level[ii] = vlevels[ii];
    _vhdr.type[ii] = vlevelType;
  }

  if (vlevelType != Mdvx::VERT_TYPE_Z) {
    return;
  }

  // convert units as required
  
  if (units.compare("km") == 0) {
    // already km
    return;
  }
  
  // convert units to km as required
  
  double mult = _getKmMult(units);
  _fhdr.grid_minz = minz * mult;
  _fhdr.grid_dz = dz * mult;
  for (size_t ii = 0; ii < nz; ii++) {
    _vhdr.level[ii] = vlevels[ii] * mult;
  }

  return;

}

///////////////////////////////////////////////////////////////
// Set grid data
// Returns 0 on success, -1 on failure

int Ncf2MdvField::_setGridData()
  
{

  // compute data size, allocate array

  size_t nPtsVol = _fhdr.nx * _fhdr.ny * _fhdr.nz;

  size_t tDimSize = 1; 
  if (_tVar!= NULL && _tDim != NULL) {
    tDimSize = _tDim->size();
  }
  
  // compute number of points contained in _var4Data

  size_t nPtsTotal =  _fhdr.nx * _fhdr.ny * _fhdr.nz * tDimSize;

  // get scale and offset

  double scale = 1.0;
  Nc3Att *scaleAtt = _var4Data->get_att(NcfMdv::scale_factor); 
  if (scaleAtt != NULL) {
    scale = scaleAtt->as_double(0);
    delete scaleAtt;
  }

  double offset = 0.0;
  Nc3Att *offsetAtt = _var4Data->get_att(NcfMdv::add_offset); 
  if (offsetAtt != NULL) {
    offset = offsetAtt->as_double(0);
    delete offsetAtt;
  }

  // read min and max value if available

  Nc3Att *minValueAtt = _var4Data->get_att(NcfMdv::min_value); 
  if (minValueAtt != NULL) {
    _fhdr.min_value = minValueAtt->as_float(0);
    _fhdr.min_value_orig_vol = _fhdr.min_value;
    delete minValueAtt;
  }

  Nc3Att *maxValueAtt = _var4Data->get_att(NcfMdv::max_value); 
  if (maxValueAtt != NULL) {
    _fhdr.max_value = maxValueAtt->as_float(0);
    _fhdr.max_value_orig_vol = _fhdr.max_value;
    delete maxValueAtt;
  }

  // decode data, fill _data array

  switch (_dataType) {

    case nc3Char:
    case nc3Byte: {

      _fhdr.encoding_type = Mdvx::ENCODING_INT8;
      _fhdr.data_element_nbytes = sizeof(ui08);
      _fhdr.volume_size = nPtsVol * _fhdr.data_element_nbytes;
      
      _data_.free();
      _data = _data_.alloc(_fhdr.volume_size);
      
      // change offset since MDV ui08 data is unsigned while
      // netCDF data ncbyte is signed
      
      bool hasFillValue = false;
      ncbyte fillValue = -128;
      Nc3Att *fillAtt = _var4Data->get_att(NcfMdv::FillValue); 
      if (fillAtt != NULL) {
        fillValue = fillAtt->as_ncbyte(0);
        delete fillAtt;
        hasFillValue = true;
      }
      
      bool hasValidMin = false;
      ncbyte validMin = -128;
      Nc3Att *minAtt = _var4Data->get_att(NcfMdv::valid_min); 
      if (minAtt != NULL) {
        validMin = minAtt->as_ncbyte(0);
        delete minAtt;
        hasValidMin = true;
      }

      bool hasValidMax = false;
      ncbyte validMax = 127;
      Nc3Att *maxAtt = _var4Data->get_att(NcfMdv::valid_max); 
      if (maxAtt != NULL) {
        validMax = maxAtt->as_ncbyte(0);
        delete maxAtt;
        hasValidMax = true;
      }

      // if the min and max are equal, it is unlikely that
      // these are correct
      if (validMin == validMax) {
        hasValidMin = false;
        hasValidMax = false;
      }
      
      if(!hasFillValue) {
        if (hasValidMin && validMin > -128) {
          fillValue = -128;
        } else if (hasValidMax && validMax < 127) {
          fillValue = 127;
        }
      }
      
      _fhdr.missing_data_value = fillValue + 128;
      _fhdr.bad_data_value = _fhdr.missing_data_value;
      _fhdr.scale = scale;
      _fhdr.bias = offset - 128.0 * scale;
      _fhdr.scaling_type = Mdvx::SCALING_SPECIFIED;

      if (_readData) {

        TaArray<ncbyte> inData_;
        ncbyte *inData = inData_.alloc(nPtsTotal);
        for (size_t kk = 0; kk < nPtsTotal; kk++) {
          inData[kk] = _fhdr.missing_data_value;
        }

        Nc3Bool iret = 0;
        size_t ndims = _var4Data->num_dims();
        if (ndims == 2) {
          iret = _var4Data->get(inData, _fhdr.ny, _fhdr.nx);
        } else if (ndims == 3) {
          if (_tVar != NULL) {
            iret = _var4Data->get(inData, tDimSize, _fhdr.ny, _fhdr.nx);
          } else if (_zVar != NULL) {
            iret = _var4Data->get(inData, _fhdr.nz, _fhdr.ny, _fhdr.nx);
          } else {
            TaStr::AddStr(_errStr, "ERROR - ndims 3 but tVar and zVar are NULL");
          }
        } else if (ndims == 4) {
          if (_tVar != NULL && _zVar != NULL) {
            iret = _var4Data->get(inData, tDimSize, _fhdr.nz, _fhdr.ny, _fhdr.nx);
            if (_tVar == NULL) {
              TaStr::AddStr(_errStr, "ERROR - ndims 4 but tVar is NULL");
            } else {
              TaStr::AddStr(_errStr, "ERROR - ndims 4 but zVar is NULL");
            }
          }
        }
        if (iret == 0) {
          // try just (y,x)
          iret = _var4Data->get(inData, _fhdr.ny, _fhdr.nx);
        }
        if (iret == 0) {
          TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::_setGridData");
          TaStr::AddStr(_errStr, "  Reading byte data, field: ", _fhdr.field_name);
          TaStr::AddStr(_errStr, _ncErr->get_errmsg());
          return -1;
        }

        inData = inData + _timeIndex * nPtsVol;
        
        for (size_t ii = 0; ii < nPtsVol; ii++) {
          if ((hasFillValue && inData[ii] == fillValue) ||
              (hasValidMin && inData[ii] < validMin) ||
              (hasValidMax && inData[ii] > validMax)) {
            _data[ii] = _fhdr.missing_data_value;
          } else {
            int val = inData[ii];
            _data[ii] = (ui08) (val + 128);
          }
        }

      } else {

        for (size_t ii = 0; ii < nPtsVol; ii++) {
          _data[ii] = _fhdr.missing_data_value;
        }

      } // if (_readData)

      break;

    } // case nc3Char: case nc3Byte:

    case nc3Short: {

      _fhdr.encoding_type = Mdvx::ENCODING_INT16;
      _fhdr.data_element_nbytes = sizeof(ui16);
      _fhdr.volume_size = nPtsVol * _fhdr.data_element_nbytes;
      
      _data_.free();
      _data = _data_.alloc(_fhdr.volume_size);
      
      // change offset since MDV ui08 data is unsigned while
      // netCDF data ncbyte is signed

      bool hasFillValue = false;
      short fillValue = -32768;
      Nc3Att *fillAtt = _var4Data->get_att(NcfMdv::FillValue); 
      if (fillAtt != NULL) {
        fillValue = fillAtt->as_short(0);
        delete fillAtt;
        hasFillValue = true;
      }

      bool hasValidMin = false;
      short validMin = -32768;
      Nc3Att *minAtt = _var4Data->get_att(NcfMdv::valid_min); 
      if (minAtt != NULL) {
        validMin = minAtt->as_short(0);
        delete minAtt;
        hasValidMin = true;
      }

      bool hasValidMax = false;
      short validMax = 32767;
      Nc3Att *maxAtt = _var4Data->get_att(NcfMdv::valid_max); 
      if (maxAtt != NULL) {
        validMax = maxAtt->as_short(0);
        delete maxAtt;
        hasValidMax = true;
      }

      // if the min and max are equal, it is unlikely that
      // these are correct
      if (validMin == validMax) {
        hasValidMin = false;
        hasValidMax = false;
      }

      if(!hasFillValue) {
        if (hasValidMin && validMin > -32768) {
          fillValue = -32768;
        } else if (hasValidMax && validMax < 32767) {
          fillValue = 32767;
        }
      }
      
      _fhdr.missing_data_value = fillValue + 32768;
      _fhdr.bad_data_value = _fhdr.missing_data_value;
      _fhdr.scale = scale;
      _fhdr.bias = offset - 32768.0 * scale;
      _fhdr.scaling_type = Mdvx::SCALING_SPECIFIED;

      if (_readData) {

        TaArray<short> inData_;
        short *inData = inData_.alloc(nPtsTotal);
        Nc3Bool iret = 0;
        
        size_t ndims = _var4Data->num_dims();
        if (ndims == 2) {
          iret = _var4Data->get(inData, _fhdr.ny, _fhdr.nx);
        } else if (ndims == 3) {
          if (_tVar != NULL) {
            iret = _var4Data->get(inData, tDimSize, _fhdr.ny, _fhdr.nx);
          } else if (_zVar != NULL) {
            iret = _var4Data->get(inData, _fhdr.nz, _fhdr.ny, _fhdr.nx);
          } else {
            TaStr::AddStr(_errStr, "ERROR - ndims 3 but tVar and zVar are NULL");
          }
        } else if (ndims == 4) {
          if (_tVar != NULL && _zVar != NULL) {
            iret = _var4Data->get(inData, tDimSize, _fhdr.nz, _fhdr.ny, _fhdr.nx);
            if (_tVar == NULL) {
              TaStr::AddStr(_errStr, "ERROR - ndims 4 but tVar is NULL");
            } else {
              TaStr::AddStr(_errStr, "ERROR - ndims 4 but zVar is NULL");
            }
          }
        }
        
        if (iret == 0) {
          TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::_setGridData");
          TaStr::AddStr(_errStr, "  Reading short data, field: ", _fhdr.field_name);
          TaStr::AddStr(_errStr, _ncErr->get_errmsg());
          return -1;
        }
        
        // Advance to desired time step or grid in the inData array
        
        inData = inData + _timeIndex * nPtsVol;
        
        ui16 *ui16Data = (ui16 *) _data;
        for (size_t ii = 0; ii < nPtsVol; ii++) {
          if ((hasFillValue && inData[ii] == fillValue) ||
              (hasValidMin && inData[ii] < validMin) ||
              (hasValidMax && inData[ii] > validMax)) {
            ui16Data[ii] = (ui16) _fhdr.missing_data_value;
          } else {
            int val = inData[ii];
            ui16Data[ii] = (ui16) (val + 32768);
          }
        }
        
      } else {
        
        ui16 *ui16Data = (ui16 *) _data;
        for (size_t ii = 0; ii < nPtsVol; ii++) {
          ui16Data[ii] = _fhdr.missing_data_value;
        }

      } // if (_readData)

      break;

    } // case nc3Short:

    case nc3Int: {

      _fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
      _fhdr.data_element_nbytes = sizeof(fl32);
      _fhdr.volume_size = nPtsVol * _fhdr.data_element_nbytes;
      
      _data_.free();
      _data = _data_.alloc(_fhdr.volume_size);
      
      bool hasFillValue = false;
      int fillValue = 0;
      Nc3Att *fillAtt = _var4Data->get_att(NcfMdv::FillValue); 
      if (fillAtt != NULL) {
        fillValue = fillAtt->as_int(0);
        delete fillAtt;
        hasFillValue = true;
      }
      
      bool hasValidMin = false;
      int validMin = -2147483648;
      Nc3Att *minAtt = _var4Data->get_att(NcfMdv::valid_min); 
      if (minAtt != NULL) {
        validMin = minAtt->as_int(0);
        delete minAtt;
        hasValidMin = true;
      }

      bool hasValidMax = false;
      int validMax = 2147483647;
      Nc3Att *maxAtt = _var4Data->get_att(NcfMdv::valid_max); 
      if (maxAtt != NULL) {
        validMax = maxAtt->as_int(0);
        delete maxAtt;
        hasValidMax = true;
      }

      // if the min and max are equal, it is unlikely that
      // these are correct
      if (validMin == validMax) {
        hasValidMin = false;
        hasValidMax = false;
      }

      if(!hasFillValue) {
        if (hasValidMin && validMin > -2147483648) {
          fillValue = -2147483648;
        } else if (hasValidMax && validMax < 2147483647) {
          fillValue = 2147483647;
        }
      }
      
      _fhdr.missing_data_value = fillValue * scale + offset;
      _fhdr.bad_data_value = _fhdr.missing_data_value;
      
      if (_readData) {

        TaArray<int> inData_;
        int *inData = inData_.alloc(nPtsTotal);
        Nc3Bool iret = 0;
        
        size_t ndims = _var4Data->num_dims();
        if (ndims == 2) {
          iret = _var4Data->get(inData, _fhdr.ny, _fhdr.nx);
        } else if (ndims == 3) {
          if (_tVar != NULL) {
            iret = _var4Data->get(inData, tDimSize, _fhdr.ny, _fhdr.nx);
          } else if (_zVar != NULL) {
            iret = _var4Data->get(inData, _fhdr.nz, _fhdr.ny, _fhdr.nx);
          } else {
            TaStr::AddStr(_errStr, "ERROR - ndims 3 but tVar and zVar are NULL");
          }
        } else if (ndims == 4) {
          if (_tVar != NULL && _zVar != NULL) {
            iret = _var4Data->get(inData, tDimSize, _fhdr.nz, _fhdr.ny, _fhdr.nx);
            if (_tVar == NULL) {
              TaStr::AddStr(_errStr, "ERROR - ndims 4 but tVar is NULL");
            } else {
              TaStr::AddStr(_errStr, "ERROR - ndims 4 but zVar is NULL");
            }
          }
        }
        
        if (iret == 0) {
          TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::_setGridData");
          TaStr::AddStr(_errStr, "  Reading int data, field: ", _fhdr.field_name);
          TaStr::AddStr(_errStr, _ncErr->get_errmsg());
          return -1;
        }
        
        // Advance to desired time step or grid in the inData array
        
        inData = inData + _timeIndex * nPtsVol;
        
        fl32 *fl32Data = (fl32 *) _data;
        for (size_t ii = 0; ii < nPtsVol; ii++) {
          if ((hasFillValue && inData[ii] == fillValue) ||
              (hasValidMin && inData[ii] < validMin) ||
              (hasValidMax && inData[ii] > validMax)) {
            fl32Data[ii] = _fhdr.missing_data_value;
          } else {
            fl32Data[ii] = inData[ii] * scale + offset;
          }
        }

      } else {
        
        fl32 *fl32Data = (fl32 *) _data;
        for (size_t ii = 0; ii < nPtsVol; ii++) {
          fl32Data[ii] = _fhdr.missing_data_value;
        }

      } // if (_readData)
      
      break;
      
    } // case nc3Int:

    case nc3Float: {

      _fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
      _fhdr.data_element_nbytes = sizeof(fl32);
      _fhdr.volume_size = nPtsVol * _fhdr.data_element_nbytes;
      
      _data_.free();
      _data = _data_.alloc(_fhdr.volume_size);

      bool hasFillValue = false;
      float fillValue = -9.0e33;
      Nc3Att *fillAtt = _var4Data->get_att(NcfMdv::FillValue); 
      if (fillAtt != NULL) {
        fillValue = fillAtt->as_float(0);
        delete fillAtt;
        hasFillValue = true;
      }

      bool hasValidMin = false;
      float validMin = -9.0e33;
      Nc3Att *minAtt = _var4Data->get_att(NcfMdv::valid_min); 
      if (minAtt != NULL) {
        validMin = minAtt->as_float(0);
        delete minAtt;
        hasValidMin = true;
      }

      bool hasValidMax = false;
      float validMax = 9.0e33;
      Nc3Att *maxAtt = _var4Data->get_att(NcfMdv::valid_max); 
      if (maxAtt != NULL) {
        validMax = maxAtt->as_float(0);
        delete maxAtt;
        hasValidMax = true;
      }

      // if the min and max are equal, it is unlikely that
      // these are correct
      if (validMin == validMax) {
        hasValidMin = false;
        hasValidMax = false;
      }

      if(!hasFillValue) {
        if (hasValidMin && validMin > -9.0e33) {
          fillValue = -9.0e33;
        } else if (hasValidMax && validMax < 9.0e33) {
          fillValue = 9.0e33;
        }
      }
      
      _fhdr.missing_data_value = fillValue * scale + offset;
      _fhdr.bad_data_value = _fhdr.missing_data_value;

      if (_readData) {

        TaArray<float> inData_;
        float *inData = inData_.alloc(nPtsTotal);
        Nc3Bool iret = 0;
        
        size_t ndims = _var4Data->num_dims();
        if (ndims == 2) {
          iret = _var4Data->get(inData, _fhdr.ny, _fhdr.nx);
        } else if (ndims == 3) {
          if (_tVar != NULL) {
            iret = _var4Data->get(inData, tDimSize, _fhdr.ny, _fhdr.nx);
          } else if (_zVar != NULL) {
            iret = _var4Data->get(inData, _fhdr.nz, _fhdr.ny, _fhdr.nx);
          } else {
            TaStr::AddStr(_errStr, "ERROR - ndims 3 but tVar and zVar are NULL");
          }
        } else if (ndims == 4) {
          if (_tVar != NULL && _zVar != NULL) {
            iret = _var4Data->get(inData, tDimSize, _fhdr.nz, _fhdr.ny, _fhdr.nx);
            if (_tVar == NULL) {
              TaStr::AddStr(_errStr, "ERROR - ndims 4 but tVar is NULL");
            } else {
              TaStr::AddStr(_errStr, "ERROR - ndims 4 but zVar is NULL");
            }
          }
        }
        
        if (iret == 0) {
          TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::_setGridData");
          TaStr::AddStr(_errStr, "  Reading float data, field: ", _fhdr.field_name);
          TaStr::AddStr(_errStr, _ncErr->get_errmsg());
          return -1;
        }
        
        // Advance to desired time step or grid in the inData array
        
        inData = inData + _timeIndex * nPtsVol;
        
        fl32 *fl32Data = (fl32 *) _data;
        for (size_t ii = 0; ii < nPtsVol; ii++) {
          if ((hasFillValue && inData[ii] == fillValue) ||
              (hasValidMin && inData[ii] < validMin) ||
              (hasValidMax && inData[ii] > validMax)) {
            fl32Data[ii] = _fhdr.missing_data_value;
          } else {
            fl32Data[ii] = inData[ii] * scale + offset;
          }
        }

      } else {
        
        fl32 *fl32Data = (fl32 *) _data;
        for (size_t ii = 0; ii < nPtsVol; ii++) {
          fl32Data[ii] = _fhdr.missing_data_value;
        }

      } // if (_readData)

      break;

    } // case nc3Float

    case nc3Double: {

      _fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
      _fhdr.data_element_nbytes = sizeof(fl32);
      _fhdr.volume_size = nPtsVol * _fhdr.data_element_nbytes;
      
      _data_.free();
      _data = _data_.alloc(_fhdr.volume_size);
      
      bool hasFillValue = false;
      double fillValue = 0;
      Nc3Att *fillAtt = _var4Data->get_att(NcfMdv::FillValue); 
      if (fillAtt != NULL) {
        fillValue = fillAtt->as_double(0);
        delete fillAtt;
        hasFillValue = true;
      }
      
      bool hasValidMin = false;
      double validMin = -9.0e33;
      Nc3Att *minAtt = _var4Data->get_att(NcfMdv::valid_min); 
      if (minAtt != NULL) {
        validMin = minAtt->as_double(0);
        delete minAtt;
        hasValidMin = true;
      }

      bool hasValidMax = false;
      double validMax = 9.0e33;
      Nc3Att *maxAtt = _var4Data->get_att(NcfMdv::valid_max); 
      if (maxAtt != NULL) {
        validMax = maxAtt->as_double(0);
        delete maxAtt;
        hasValidMax = true;
      }

      // if the min and max are equal, it is unlikely that
      // these are correct
      if (validMin == validMax) {
        hasValidMin = false;
        hasValidMax = false;
      }

      if(!hasFillValue) {
        if (hasValidMin && validMin > -9.0e33) {
          fillValue = -9.0e33;
        } else if (hasValidMax && validMax < 9.0e33) {
          fillValue = 9.0e33;
        }
      }

      _fhdr.missing_data_value = fillValue * scale + offset;
      _fhdr.bad_data_value = _fhdr.missing_data_value;

      if (_readData) {
        
        TaArray<double> inData_;
        double *inData = inData_.alloc(nPtsTotal);
        Nc3Bool iret = 0;
        
        if (_tVar != NULL && _zVar != NULL) {
          iret = _var4Data->get( inData, tDimSize, _fhdr.nz, _fhdr.ny, _fhdr.nx);
        } else if (_tVar != NULL) {
          iret = _var4Data->get(inData, tDimSize, _fhdr.ny, _fhdr.nx);
        } else if (_zVar != NULL) {
          iret = _var4Data->get(inData, _fhdr.nz, _fhdr.ny, _fhdr.nx);
        } else {
          iret = _var4Data->get(inData, _fhdr.ny, _fhdr.nx);
        }
        
        if (iret == 0) {
          TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::_setGridData");
          TaStr::AddStr(_errStr, "  Reading double data, field: ", _fhdr.field_name);
          TaStr::AddStr(_errStr, _ncErr->get_errmsg());
          return -1;
        }
        
        // Advance to desired time step or grid in the inData array
        
        inData = inData + _timeIndex * nPtsVol;
        
        fl32 *fl32Data = (fl32 *) _data;
        for (size_t ii = 0; ii < nPtsVol; ii++) {
          if ((hasFillValue && inData[ii] == fillValue) ||
              (hasValidMin && inData[ii] < validMin) ||
              (hasValidMax && inData[ii] > validMax)) {
            fl32Data[ii] = _fhdr.missing_data_value;
          } else {
            fl32Data[ii] = inData[ii] * scale + offset;
          }
        }
        
      } else {
        
        fl32 *fl32Data = (fl32 *) _data;
        for (size_t ii = 0; ii < nPtsVol; ii++) {
          fl32Data[ii] = _fhdr.missing_data_value;
        }
        
      } // if (_readData)

      break;

    } // case nc3Double

    default: {
      TaStr::AddStr(_errStr, "ERROR - Ncf2MdvField::_setGridData");
      TaStr::AddInt(_errStr, "  Unknown data type: ", (int) _dataType);
      return -1;
    }

  } // switch

  _fhdr.compression_type = Mdvx::COMPRESSION_NONE;

  if (_zVar == NULL) {
    _fhdr.data_dimension = 2;
  } else {
    _fhdr.data_dimension = 3;
  }

  return 0;

}

///////////////////////////////////////////////////////////////
// If the netCDF file was created from MDV, there will be some
// parameters available from the field header

int Ncf2MdvField::_setMdvSpecific()
  
{

  for (int i = 0; i < _var4Data->num_atts(); i++) {
    
    Nc3Att* att = _var4Data->get_att(i);
      
    if (att == NULL) {
      continue;
    }
    
    // ints
    
    _setSi32FromAttr(att, NcfMdv::mdv_field_code, _fhdr.field_code);
    _setSi64FromAttr(att, NcfMdv::mdv_user_time_1, _fhdr.user_time1);
    _setSi64FromAttr(att, NcfMdv::mdv_user_time_2, _fhdr.user_time2);
    _setSi64FromAttr(att, NcfMdv::mdv_user_time_3, _fhdr.user_time3);
    _setSi64FromAttr(att, NcfMdv::mdv_user_time_4, _fhdr.user_time4);
    _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_0, _fhdr.user_data_si32[0]);
    _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_1, _fhdr.user_data_si32[1]);
    _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_2, _fhdr.user_data_si32[2]);
    _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_3, _fhdr.user_data_si32[3]);
    _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_4, _fhdr.user_data_si32[4]);
    _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_5, _fhdr.user_data_si32[5]);
    _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_6, _fhdr.user_data_si32[6]);
    _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_7, _fhdr.user_data_si32[7]);
    _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_8, _fhdr.user_data_si32[8]);
    _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_9, _fhdr.user_data_si32[9]);
    _setFl32FromAttr(att, NcfMdv::mdv_user_data_fl32_0, _fhdr.user_data_fl32[0]);
    _setFl32FromAttr(att, NcfMdv::mdv_user_data_fl32_1, _fhdr.user_data_fl32[1]);
    _setFl32FromAttr(att, NcfMdv::mdv_user_data_fl32_2, _fhdr.user_data_fl32[2]);
    _setFl32FromAttr(att, NcfMdv::mdv_user_data_fl32_3, _fhdr.user_data_fl32[3]);
    _setSi32FromAttr(att, NcfMdv::mdv_transform_type, _fhdr.transform_type);
    _setSi32FromAttr(att, NcfMdv::mdv_native_vlevel_type, _fhdr.native_vlevel_type);
    _setSi32FromAttr(att, NcfMdv::mdv_vlevel_type, _fhdr.vlevel_type);

    if (_projType == Mdvx::PROJ_VSECTION) {
      _setFl64FromAttr(att, NcfMdv::mdv_proj_origin_lat, _fhdr.proj_origin_lat);
      _setFl64FromAttr(att, NcfMdv::mdv_proj_origin_lon, _fhdr.proj_origin_lon);
    }
      
    string transform;
    _setStrFromAttr(att, NcfMdv::mdv_transform, transform);
    STRncopy(_fhdr.transform, transform.c_str(), MDV_TRANSFORM_LEN);
    
    // Caller must delete attribute
    
    delete att;
    
  } // i
  
  // set standard items

  _fhdr.struct_id = Mdvx::FIELD_HEAD_MAGIC_COOKIE_64;
  _fhdr.record_len1 = sizeof(Mdvx::field_header_t) - (2 * sizeof(si32));
  _fhdr.record_len2 = _fhdr.record_len1;

  _vhdr.struct_id = Mdvx::VLEVEL_HEAD_MAGIC_COOKIE_64;
  _vhdr.record_len1 = sizeof(Mdvx::vlevel_header_t) - (2 * sizeof(si32));
  _vhdr.record_len2 = _vhdr.record_len1;

  return 0;

}

////////////////////////////////////////
// set si32 from attribute

void Ncf2MdvField::_setSi32FromAttr(Nc3Att *att, const string &requiredName, si32 &val)

{
  if (att == NULL) {
    return;
  }
  if (requiredName.compare(att->name()) == 0) {
    val = att->as_int(0);
  }
}

////////////////////////////////////////
// set si64 from attribute

void Ncf2MdvField::_setSi64FromAttr(Nc3Att *att, const string &requiredName, si64 &val)

{
  if (att == NULL) {
    return;
  }
  if (requiredName.compare(att->name()) == 0) {
    val = att->as_int64(0);
  }
}

////////////////////////////////////////
// set fl32 from attribute

void Ncf2MdvField::_setFl32FromAttr(Nc3Att *att, const string &requiredName, fl32 &val)

{
  if (att == NULL) {
    return;
  }
  if (requiredName.compare(att->name()) == 0) {
    val = att->as_float(0);
  }
}
    
////////////////////////////////////////
// set fl64 from attribute

void Ncf2MdvField::_setFl64FromAttr(Nc3Att *att, const string &requiredName, fl64 &val)

{
  if (att == NULL) {
    return;
  }
  if (requiredName.compare(att->name()) == 0) {
   val = att->as_double(0);
  }
}
    
////////////////////////////////////////
// set string from attribute

void Ncf2MdvField::_setStrFromAttr(Nc3Att *att, const string &requiredName, string &val)

{
  if (att == NULL) {
    return;
  }
  if (requiredName.compare(att->name()) == 0) {
    val = _asString(att);
  }
}

///////////////////////////////////////////
// get string representation of component

string Ncf2MdvField::_asString(const Nc3TypedComponent *component,
                               int index /* = 0 */) const
  
{
  
  const char* strc = component->as_string(index);
  string strs(strc);
  delete[] strc;
  return strs;

}

///////////////////////////////////////////
// get km converter

double Ncf2MdvField::_getKmMult(const string &units) const
  
{
  
  // get multiplier to convert to km

  string copy;
  for (size_t ii = 0; ii < units.size(); ii++) {
    copy.append(1, tolower(units[ii]));
  }

  double mult = 1.0;
  if (copy.compare("m") == 0 || copy.compare("meters") == 0) {
    mult = 1.0e-3;
  } else if (copy.compare("cm") == 0 || copy.compare("centimeters") == 0) {
    mult = 1.0e-5;
  } else if (copy.compare("mm") == 0 || copy.compare("millimeters") == 0) {
    mult = 1.0e-6;
  } else if (copy.compare("ft") == 0 || copy.compare("feet") == 0) {
    mult = 0.0003048;
  }

  return mult;

}



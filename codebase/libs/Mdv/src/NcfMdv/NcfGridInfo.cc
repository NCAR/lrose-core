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
// NcfGridInfo.cc
// 
// Sue Dettling, RAP, NCAR
// PO Box 3000, Boulder, CO, USA
// April 2008
//
/////////////////////////////////////////////////////////////////////
// 
// NcfGridInfo object will bridge the Mdv projection info with the
// netCDF dimensions, coordinate variables, and auxilliary 
// variables (if relevant).
//
//////////////////////////////////////////////////////////////

#include <Mdv/NcfGridInfo.hh>
#include <Mdv/NcfMdv.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/TaPjg.hh>

NcfGridInfo::NcfGridInfo ( Mdvx::field_header_t fHdr) :
        _fHdr(fHdr)
{
  
  // Initialize projection object and coordinate

  _proj.init(fHdr);
  _coord = _proj.getCoord();
  _isXSect = false;
  _outputLatlonArrays = true;

  // Initialize netCDF object pointers

  _xDim = NULL;
  _yDim = NULL;
  _xDim = NULL;
  _yDim = NULL;

  _projVar = NULL;
  _xVar = NULL;
  _yVar = NULL;
  _latVar = NULL;
  _lonVar = NULL;
  _altVar = NULL;

  // initialize coordinate x, y, lat, and lon arrays

  _xData = NULL;
  _yData = NULL;
  _lonData = NULL;
  _latData = NULL;
  
}

NcfGridInfo::~NcfGridInfo()
{

  // Memory cleanup

  _clear();
  
}

void NcfGridInfo::_clear()
{

  // Memory cleanup

  if (_xData != NULL) {
    delete[] _xData;
    _xData = NULL;
  }
  
  if (_yData != NULL ) {
    delete[] _yData;
    _yData = NULL;
  }

  if(_lonData != NULL) {
    delete[] _lonData;
    _lonData = NULL;
  }
  
  if(_latData != NULL) {
    delete[] _latData;
    _latData = NULL;
  }
  
}

// equality  -- Assume the objects are equal if their coordinate structures
// are equal.  This requires that the structure be initialized to all zeros
// before setting any of the structure's fields.

bool NcfGridInfo::operator==(const NcfGridInfo &other) const
{
  
  if (_proj != other._proj) {
    return false;
  }

  return true;

}
  
//  computeCoordinateVars():
//  Compute projection x and y arrays. If data is not in lat lon projection
//  compute the auxiliary 2D lat lon  coordinate variables. See Section 5 of CF-1.0 
//  Coordnate Systems for discussion on auxiliary coordinate variables

int NcfGridInfo::computeCoordinateVars()
{

  // Compute the x and y coordinates

  int nx = _coord.nx;
  int ny = _coord.ny;

  // Allocate memory for data arrays

  _clear();

  _xData = new float[nx];
  _yData = new float[ny];
  _lonData = new float[nx * ny];
  _latData = new float[nx * ny];
     
  // Fill in x, y coordinate variable arrays

  float minx = _coord.minx;
  float dx = _coord.dx;
  float miny = _coord.miny;
  float dy = _coord.dy;

  for (int i = 0; i < _coord.nx; i++) {
    _xData[i] = minx + i * dx;
  }
  
  for (int j = 0; j < _coord.ny; j++) {
    _yData[j] = miny + j * dy;
  }
  
  // Compute the auxiliary arrays

  for (int j = 0; j < _coord.ny; j++)  {
    for (int i = 0; i < _coord.nx; i++) {
      double lat, lon;
      _proj.xy2latlon( _xData[i],  _yData[j], lat, lon);
      _latData[ j * nx + i ] = lat;
      _lonData[ j * nx + i] = lon;
    }
  }

  return 0;

}
  

////////////////////////////////////////////////////////////////////
// For an Xsection, set the CoordinateVars from the sample points

int NcfGridInfo::setCoordinateVarsFromSamplePoints(vector<Mdvx::vsect_samplept_t> pts)
{

  // Allocate memory for data arrays

  _clear();

  _xData = new float[pts.size()];
  _yData = new float[1];
  _lonData = new float[pts.size()];
  _latData = new float[pts.size()];
  
  double minx = _fHdr.grid_minx;
  double dx = _fHdr.grid_dx;

  for (int i = 0; i < (int) pts.size(); i++) {
    _xData[i] = minx + i * dx;
    _latData[i]  = pts[i].lat;
    _lonData[i]  = pts[i].lon;
    //Eventually, we want samplePoints to have a height as well:
    //_altData[i] = pts[i].alt;
  }
  
  double miny = _fHdr.grid_miny;
  for (int j = 0; j < 1; j++) {
    //for now: yDim = 1 and ydata = 0, i.e., we are getting a 2d slice back
    //Eventually, we want to return a 3d Xsection, i.e. a rectangular tube 
    //around a flight path. In this case, y will be 'horizontal distance from path'
    _yData[j] = miny;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// add xy dimension for this grid
// returns 0 on success, -1 on failure

int NcfGridInfo::addXyDim(int gridNum, Nc3File *ncFile, string &errStr)

{

  char xDimName[4],  yDimName[4];
  
  sprintf(xDimName, "x%d", gridNum);
  sprintf(yDimName, "y%d", gridNum);
  
  if (!(_xDim = ncFile->add_dim(xDimName, _coord.nx))) {
    return -1;
  }
  if (!(_yDim = ncFile->add_dim(yDimName, _coord.ny))) {
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////////////////////
// add nc projection variable for this grid

int NcfGridInfo::addProjVar(int gridNum, Nc3File *ncFile, string &errStr)

{

  int iret = 0;

  // Add variable to NcFile

  char projVarName[16];
  sprintf(projVarName, "%s_%d", NcfMdv::grid_mapping, gridNum);
    
  _projVar = ncFile->add_var(projVarName, nc3Int);
  if (_projVar == NULL) {
    TaStr::AddStr(errStr, "ERROR: NcfGridInfo::addProjVar");
    TaStr::AddStr(errStr, "  Cannot add projection: ", projVarName);
    return -1;
  }
    
  // Add attributes
  
  const Mdvx::coord_t &mdvxCoord = _proj.getCoord();
  Mdvx::projection_type_t projType = _proj.getProjType();

  switch (projType) {
      
    case Mdvx::PROJ_FLAT: {
      iret |= !_projVar->add_att(NcfMdv::grid_mapping_name, NcfMdv::azimuthal_equidistant);
      iret |= !_projVar->add_att(NcfMdv::longitude_of_projection_origin,
                                 mdvxCoord.proj_origin_lon);
      iret |= !_projVar->add_att(NcfMdv::latitude_of_projection_origin,
                                 mdvxCoord.proj_origin_lat);
      iret |= !_projVar->add_att(NcfMdv::false_easting, mdvxCoord.false_easting);
      iret |= !_projVar->add_att(NcfMdv::false_northing, mdvxCoord.false_northing);
      
      double earthRadiusMeters  = TaPjg::getEarthRadius() * 1000;
      iret  |= !_projVar->add_att(NcfMdv::earth_radius, earthRadiusMeters);
     
      break;
    }
      
    case Mdvx::PROJ_LATLON: {
      iret |= !_projVar->add_att(NcfMdv::grid_mapping_name, NcfMdv::latitude_longitude);
      break;
    }
      
    case Mdvx::PROJ_LAMBERT_CONF: {
      iret |= !_projVar->add_att(NcfMdv::grid_mapping_name, NcfMdv::lambert_conformal_conic);
      iret |= !_projVar->add_att(NcfMdv::longitude_of_central_meridian,
                                 mdvxCoord.proj_origin_lon);
      iret |= !_projVar->add_att(NcfMdv::latitude_of_projection_origin,
                                 mdvxCoord.proj_origin_lat);
      float parallels[2];
      parallels[0] = mdvxCoord.proj_params.lc2.lat1;
      parallels[1] = mdvxCoord.proj_params.lc2.lat2;
      if (mdvxCoord.proj_params.lc2.lat1 == mdvxCoord.proj_params.lc2.lat2) {
        iret |= !_projVar->add_att(NcfMdv::standard_parallel, 1, parallels);
      } else {
        iret |= !_projVar->add_att(NcfMdv::standard_parallel, 2, parallels);
      }
      iret |= !_projVar->add_att(NcfMdv::false_easting, mdvxCoord.false_easting);
      iret |= !_projVar->add_att(NcfMdv::false_northing, mdvxCoord.false_northing);

      double earthRadiusMeters  = TaPjg::getEarthRadius() * 1000;
      iret  |= !_projVar->add_att(NcfMdv::earth_radius, earthRadiusMeters);

      break;
    }
      
    case Mdvx::PROJ_POLAR_RADAR: {
      iret |= !_projVar->add_att(NcfMdv::grid_mapping_name, NcfMdv::polar_radar);
      iret |= !_projVar->add_att(NcfMdv::longitude_of_projection_origin,
                                 mdvxCoord.proj_origin_lon);
      iret |= !_projVar->add_att(NcfMdv::latitude_of_projection_origin,
                                 mdvxCoord.proj_origin_lat);
      break;
    }
      
    case Mdvx::PROJ_POLAR_STEREO: {
      iret |= !_projVar->add_att(NcfMdv::grid_mapping_name, NcfMdv::polar_stereographic);
      iret |= !_projVar->add_att(NcfMdv::straight_vertical_longitude_from_pole,
                                 mdvxCoord.proj_params.ps.tan_lon);
      if (mdvxCoord.proj_params.ps.pole_type == 0) {
        iret |= !_projVar->add_att(NcfMdv::latitude_of_projection_origin, 90.0);
      } else {
        iret |= !_projVar->add_att(NcfMdv::latitude_of_projection_origin, -90.0);
      }
      iret |= !_projVar->add_att(NcfMdv::scale_factor_at_projection_origin,
                                 mdvxCoord.proj_params.ps.central_scale);
      iret |= !_projVar->add_att(NcfMdv::false_easting, mdvxCoord.false_easting);
      iret |= !_projVar->add_att(NcfMdv::false_northing, mdvxCoord.false_northing);

      double earthRadiusMeters  = TaPjg::getEarthRadius() * 1000;
      iret  |= !_projVar->add_att(NcfMdv::earth_radius, earthRadiusMeters);

      break;
    }
      
    case Mdvx::PROJ_OBLIQUE_STEREO: {
      iret |= !_projVar->add_att(NcfMdv::grid_mapping_name, NcfMdv::stereographic);
      iret |= !_projVar->add_att(NcfMdv::longitude_of_projection_origin,
                                 mdvxCoord.proj_params.os.tan_lon);
      iret |= !_projVar->add_att(NcfMdv::latitude_of_projection_origin,
                                 mdvxCoord.proj_params.os.tan_lat);
      iret |= !_projVar->add_att(NcfMdv::scale_factor_at_projection_origin, 1.0);
      iret |= !_projVar->add_att(NcfMdv::false_easting, mdvxCoord.false_easting);
      iret |= !_projVar->add_att(NcfMdv::false_northing, mdvxCoord.false_northing);
      break;
    }
      
    case Mdvx::PROJ_MERCATOR: {
      iret |= !_projVar->add_att(NcfMdv::grid_mapping_name, NcfMdv::mercator);
      iret |= !_projVar->add_att(NcfMdv::longitude_of_central_meridian,
                                 mdvxCoord.proj_origin_lon);
      iret |= !_projVar->add_att(NcfMdv::latitude_of_projection_origin,
                                 mdvxCoord.proj_origin_lat);
      iret |= !_projVar->add_att(NcfMdv::false_easting, mdvxCoord.false_easting);
      iret |= !_projVar->add_att(NcfMdv::false_northing, mdvxCoord.false_northing);

      double earthRadiusMeters  = TaPjg::getEarthRadius() * 1000;
      iret  |= !_projVar->add_att(NcfMdv::earth_radius, earthRadiusMeters);

      break;
    }
      
    case Mdvx::PROJ_TRANS_MERCATOR: {
      iret |= !_projVar->add_att(NcfMdv::grid_mapping_name, NcfMdv::transverse_mercator);
      iret |= !_projVar->add_att(NcfMdv::longitude_of_central_meridian,
                                 mdvxCoord.proj_origin_lon);
      iret |= !_projVar->add_att(NcfMdv::latitude_of_projection_origin,
                                 mdvxCoord.proj_origin_lat);
      iret |= !_projVar->add_att(NcfMdv::scale_factor_at_central_meridian,
                                 mdvxCoord.proj_params.tmerc.central_scale);
      iret |= !_projVar->add_att(NcfMdv::false_easting, mdvxCoord.false_easting);
      iret |= !_projVar->add_att(NcfMdv::false_northing, mdvxCoord.false_northing);

      double earthRadiusMeters  = TaPjg::getEarthRadius() * 1000;
      iret  |= !_projVar->add_att(NcfMdv::earth_radius, earthRadiusMeters);

      break;
    }
      
    case Mdvx::PROJ_ALBERS: {
      iret |= !_projVar->add_att(NcfMdv::grid_mapping_name, NcfMdv::albers_conical_equal_area);
      iret |= !_projVar->add_att(NcfMdv::longitude_of_central_meridian,
                                 mdvxCoord.proj_origin_lon);
      iret |= !_projVar->add_att(NcfMdv::latitude_of_projection_origin,
                                 mdvxCoord.proj_origin_lat);
      float parallels[2];
      parallels[0] = mdvxCoord.proj_params.albers.lat1;
      parallels[1] = mdvxCoord.proj_params.albers.lat2;
      if (mdvxCoord.proj_params.albers.lat1 == mdvxCoord.proj_params.albers.lat2) {
        iret |= !_projVar->add_att(NcfMdv::standard_parallel, 1, parallels);
      } else {
        iret |= !_projVar->add_att(NcfMdv::standard_parallel, 2, parallels);
      }
      iret |= !_projVar->add_att(NcfMdv::false_easting, mdvxCoord.false_easting);
      iret |= !_projVar->add_att(NcfMdv::false_northing, mdvxCoord.false_northing);
      break;
    }
      
    case Mdvx::PROJ_LAMBERT_AZIM: {
      iret |= !_projVar->add_att(NcfMdv::grid_mapping_name, NcfMdv::lambert_azimuthal_equal_area);
      iret |= !_projVar->add_att(NcfMdv::longitude_of_projection_origin,
                                 mdvxCoord.proj_origin_lon);
      iret |= !_projVar->add_att(NcfMdv::latitude_of_projection_origin,
                                 mdvxCoord.proj_origin_lat);
      iret |= !_projVar->add_att(NcfMdv::false_easting, mdvxCoord.false_easting);
      iret |= !_projVar->add_att(NcfMdv::false_northing, mdvxCoord.false_northing);
      break;
    }
      
    case Mdvx::PROJ_VERT_PERSP: {
      iret |= !_projVar->add_att(NcfMdv::grid_mapping_name, NcfMdv::vertical_perspective);
      iret |= !_projVar->add_att(NcfMdv::longitude_of_projection_origin,
                                 mdvxCoord.proj_origin_lon);
      iret |= !_projVar->add_att(NcfMdv::latitude_of_projection_origin,
                                 mdvxCoord.proj_origin_lat);
      double pptHtm = (mdvxCoord.proj_params.vp.persp_radius - Pjg::EradKm) * 1000.0;
      iret |= !_projVar->add_att(NcfMdv::perspective_point_height, pptHtm);
      iret |= !_projVar->add_att(NcfMdv::false_easting, mdvxCoord.false_easting);
      iret |= !_projVar->add_att(NcfMdv::false_northing, mdvxCoord.false_northing);
      break;
    }
      
    case Mdvx::PROJ_VSECTION: {
      iret |= !_projVar->add_att(NcfMdv::grid_mapping_name,
                                 NcfMdv::vertical_section);
      break;
    }
      
    default: {
      iret |= !_projVar->add_att(NcfMdv::grid_mapping_name, "unknown");
      break;
    }
      
  } // switch
  
  return (iret? -1 : 0);

}

////////////////////////////////////////////////////////////////
// add nc coordinate variables for this grid

int NcfGridInfo::addCoordVars(int gridNum, bool outputLatlonArrays,
                              Nc3File *ncFile, string &errStr)

{
  
  int iret = 0;
  _outputLatlonArrays = outputLatlonArrays;

  // Add x and y variables to NcFile
  
  char xVarName[32],  yVarName[32];
  
  sprintf(xVarName, "x%d", gridNum);
  sprintf(yVarName, "y%d", gridNum);
  
  if ((_xVar = ncFile->add_var(xVarName, nc3Float, _xDim)) == NULL) {
    TaStr::AddStr(errStr, "Mdv2NcfTrans::NcfGridInfo::addCoordVars");
    TaStr::AddStr(errStr, "  Cannot add xVar");
    return -1;
  }
  if ((_yVar = ncFile->add_var(yVarName, nc3Float, _yDim)) == NULL) {
    TaStr::AddStr(errStr, "Mdv2NcfTrans::NcfGridInfo::addCoordVars");
    TaStr::AddStr(errStr, "  Cannot add yVar");
    return -1;
  }
  
  // Add attributes of coordinate variables
  
  Mdvx::projection_type_t projType = _proj.getProjType();
  if (projType == Mdvx::PROJ_LATLON) {
    
    iret |= !_xVar->add_att(NcfMdv::standard_name, NcfMdv::longitude);
    iret |= !_xVar->add_att(NcfMdv::long_name, NcfMdv::longitude);
    iret |= !_xVar->add_att(NcfMdv::units, NcfMdv::degrees_east);
    iret |= !_yVar->add_att(NcfMdv::standard_name, NcfMdv::latitude);
    iret |= !_yVar->add_att(NcfMdv::long_name, NcfMdv::latitude);
    iret |= !_yVar->add_att(NcfMdv::units, NcfMdv::degrees_north);
    
  } else {
    
    iret |= !_xVar->add_att(NcfMdv::standard_name, NcfMdv::projection_x_coordinate);
    iret |= !_xVar->add_att(NcfMdv::units, "km");
    iret |= !_yVar->add_att(NcfMdv::standard_name, NcfMdv::projection_y_coordinate);
    iret |= !_yVar->add_att(NcfMdv::units, "km");
    
    // Add auxiliary variables to NcFile if grids are not lat lon grids
    // and user specifies their inclusion.
    
    if (_outputLatlonArrays) {
      
      char latVarName[32],  lonVarName[32];
      sprintf(latVarName, "lat%d", gridNum);
      sprintf(lonVarName, "lon%d", gridNum);
      
      if ((_latVar = ncFile->add_var(latVarName, nc3Float,
                                     _yDim, _xDim)) == NULL) {
        TaStr::AddStr(errStr, "Mdv2NcfTrans::NcfGridInfo::addCoordVars");
        TaStr::AddStr(errStr, "  Cannot add latVar");
        return -1;
      }
      if ((_lonVar = ncFile->add_var(lonVarName, nc3Float,
                                     _yDim, _xDim)) == NULL) {
        TaStr::AddStr(errStr, "Mdv2NcfTrans::NcfGridInfo::addCoordVars");
        TaStr::AddStr(errStr, "  Cannot add lonVar");
        return -1;
      }
      
      iret |= !_latVar->add_att(NcfMdv::standard_name, NcfMdv::latitude);
      iret |= !_latVar->add_att(NcfMdv::units, NcfMdv::degrees_north);
      iret |= !_lonVar->add_att(NcfMdv::standard_name, NcfMdv::longitude);
      iret |= !_lonVar->add_att(NcfMdv::units, NcfMdv::degrees_east);
      
    } // if (_outputLatlonArrays)
    
  } // end else (not a PROJ_LATLON)
    
  iret |= !_xVar->add_att(NcfMdv::axis, "X");
  iret |= !_yVar->add_att(NcfMdv::axis, "Y");

  _isXSect = false;
  return (iret? -1 : 0);

}

////////////////////////////////////////////////////////////////
// add nc vert section coordinate variables for this grid

int NcfGridInfo::addVsectCoordVars(int gridNum,
                                   Nc3File *ncFile, string &errStr)

{
  
  int iret = 0;
  
  char xVarName[32],  yVarName[32];
  sprintf(xVarName, "x%d", gridNum);
  sprintf(yVarName, "y%d", gridNum);
  
  if ((_xVar = ncFile->add_var(xVarName, nc3Float, _xDim)) == NULL) {
    TaStr::AddStr(errStr, "Mdv2NcfTrans::NcfGridInfo::addVsectCoordVars");
    TaStr::AddStr(errStr, "  Cannot add xVar");
    return -1;
  }
  
  if ((_yVar = ncFile->add_var(yVarName, nc3Float, _yDim)) == NULL) {
    TaStr::AddStr(errStr, "Mdv2NcfTrans::NcfGridInfo::addVsectCoordVars");
    TaStr::AddStr(errStr, "  Cannot add yVar");
    return -1;
  }
  
  iret |= !_xVar->add_att(NcfMdv::standard_name, NcfMdv::projection_x_coordinate);
  iret |= !_xVar->add_att(NcfMdv::units, "km");
  iret |= (!_xVar->add_att(NcfMdv::long_name,
                           "Distance along vertical section"));
  iret |= !_xVar->add_att(NcfMdv::axis, "X");
  
  iret |= !_yVar->add_att(NcfMdv::standard_name, NcfMdv::projection_y_coordinate);
  iret |= !_yVar->add_att(NcfMdv::units, "km");
  iret |= (!_yVar->add_att (NcfMdv::long_name,
                            "Distance orthogonal to vertical section"));
  iret |= !_yVar->add_att(NcfMdv::axis, "Y");

  // Add lat, lon, alt variables
  
  char latVarName[8],  lonVarName[8], altVarName[8];
  
  sprintf(latVarName, "lat%d", gridNum);
  sprintf(lonVarName, "lon%d", gridNum);
  sprintf(altVarName, "alt%d", gridNum);
  
  if ((_latVar = ncFile->add_var(latVarName, nc3Float, _xDim)) == NULL) {
    TaStr::AddStr(errStr, "Mdv2NcfTrans::NcfGridInfo::addVsectCoordVars");
    TaStr::AddStr(errStr, "  Cannot add latVar");
    return -1;
  }
  if ((_lonVar = ncFile->add_var(lonVarName, nc3Float, _xDim)) == NULL) {
    TaStr::AddStr(errStr, "Mdv2NcfTrans::NcfGridInfo::addVsectCoordVars");
    TaStr::AddStr(errStr, "  Cannot add lonVar");
    return -1;
  }
  if ((_altVar = ncFile->add_var(altVarName, nc3Float, _xDim)) == NULL) {
    TaStr::AddStr(errStr, "Mdv2NcfTrans::NcfGridInfo::addVsectCoordVars");
    TaStr::AddStr(errStr, "  Cannot add altVar");
    return -1;
  }
  
  iret |= !_latVar->add_att(NcfMdv::standard_name, NcfMdv::latitude);
  iret |= !_latVar->add_att(NcfMdv::comment, "latitude at sampled point");
  iret |= !_latVar->add_att(NcfMdv::units, NcfMdv::degrees_north);
  
  iret |= !_lonVar->add_att(NcfMdv::standard_name, NcfMdv::longitude);
  iret |= !_lonVar->add_att(NcfMdv::comment, "longitude at sampled point");
  iret |= !_lonVar->add_att(NcfMdv::units, NcfMdv::degrees_east);
  
  iret |= !_altVar->add_att(NcfMdv::standard_name, "altitude");
  iret |= !_altVar->add_att(NcfMdv::comment, "altitude at sampled point");
  iret |= !_altVar->add_att(NcfMdv::units, "km");
  
  _isXSect = true;
  return (iret? -1 : 0);

}

///////////////////////////////////////////////////////////
// Write the coordinate data
//
// Returns 0 on success, -1 on error

int NcfGridInfo::writeCoordDataToFile(Nc3File *ncFile, string &errStr)
  
{
  
  // Put the coordinate variable data

  if (_xVar != NULL && _xData != NULL) {
    if (!_xVar->put( _xData, _coord.nx)) {
      TaStr::AddStr(errStr, "ERROR - NcfGridInfo::writeCoordDataToFile");
      TaStr::AddStr(errStr, "  Cannot put xData");
      return -1;
    }
  }

  if (_yVar != NULL && _yData != NULL) {
    if (!_yVar->put( _yData, _coord.ny)) {
      TaStr::AddStr(errStr, "ERROR - NcfGridInfo::writeCoordDataToFile");
      TaStr::AddStr(errStr, "  Cannot put yData");
      return -1;
    }
  }
  
  // If we are converting a Xsection, also add lat, long points,
  // Note that projection type for a cross section is not PROJ_LATLON
  // so adding lat lon here is mutually exclusive to adding them above.

  Mdvx::projection_type_t projType = _proj.getProjType();

  if (_isXSect) {

    if (_lonData != NULL && _lonVar != NULL) {
      if (!_lonVar->put(_lonData, _coord.nx)) {
        TaStr::AddStr(errStr, "ERROR - NcfGridInfo::writeCoordDataToFile");
        TaStr::AddStr(errStr, "  Cannot put vsect lonData");
        return -1;
      }
    }

    if (_latData != NULL && _latVar != NULL) {
      if (!_latVar->put(_latData, _coord.nx)) {
        TaStr::AddStr(errStr, "ERROR - NcfGridInfo::writeCoordDataToFile");
        TaStr::AddStr(errStr, "  Cannot put vsect latData");
        return -1;
      }
    }

  } else if (projType != Mdvx::PROJ_LATLON && _outputLatlonArrays) {

    // Put latlon arrays

    if (_lonData != NULL && _lonVar != NULL) {
      if (!_lonVar->put(_lonData, _coord.ny, _coord.nx)) {
        TaStr::AddStr(errStr, "ERROR - NcfGridInfo::writeCoordDataToFile");
        TaStr::AddStr(errStr, "  Cannot put xy lonData");
        return -1;
      }
    }

    if (_latData != NULL && _latVar != NULL) {
      if (!_latVar->put(_latData, _coord.ny, _coord.nx)) {
        TaStr::AddStr(errStr, "ERROR - NcfGridInfo::writeCoordDataToFile");
        TaStr::AddStr(errStr, "  Cannot put xy latData");
        return -1;
      }
    }
    
  }

  return 0;

}  


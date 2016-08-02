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
///////////////////////////////////////////////////
// GDS - Grid Description Section
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
// 
// $Id: GDS.cc,v 1.17 2016/06/21 21:03:52 jcraig Exp $
//
//////////////////////////////////////////////////
#include <euclid/PjgLc1Calc.hh>
#include <euclid/PjgGrib.hh>
#include <grib2/GDS.hh>
#include <grib2/LatLonProj.hh>
#include <grib2/PolarStereoProj.hh>
#include <grib2/LambertConfProj.hh>
#include <grib2/GausLatLonProj.hh>
#include <grib2/MercatorProj.hh>
#include <grib2/SpaceViewProj.hh>

using namespace std;

namespace Grib2 {

const int GDS::EQUIDISTANT_CYL_PROJ_ID = 0;
const int GDS::MERCATOR_PROJ_ID = 10;
const int GDS::POLAR_STEREOGRAPHIC_PROJ_ID = 20;
const int GDS::LAMBERT_CONFORMAL_PROJ_ID = 30;
const int GDS::GAUSSIAN_LAT_LON_PROJ_ID = 40;
const int GDS::SPACE_VIEW_PROJ_ID = 90;
const fl32 GDS::DEGREES_SCALE_FACTOR = 0.000001;
const fl32 GDS::GRID_SCALE_FACTOR =    0.000001;


GDS::GDS() :
  GribSection()
{
  _sectionLen = 0;
  _sectionNum = 3;
  _gridDefSource = 0;
  _numDataPoints = 0;
  _listSize = 0;
  _quasi_regularListInterp = 0;
  _gridTemplateNum = -1;
  _projection = NULL;
}

GDS::GDS(si32 numberDataPoints, si32 gridDefNum, GribProj *projectionTemplate) :
  GribSection()
{
  _sectionLen = 0;
  _sectionNum = 3;
  _gridDefSource = 0;
  _numDataPoints = numberDataPoints;
  _listSize = 0;
  _quasi_regularListInterp = 0;
  _gridTemplateNum = gridDefNum;

  switch (_gridTemplateNum) {
    case EQUIDISTANT_CYL_PROJ_ID:
    case POLAR_STEREOGRAPHIC_PROJ_ID:
    case LAMBERT_CONFORMAL_PROJ_ID:
    case GAUSSIAN_LAT_LON_PROJ_ID:
    case MERCATOR_PROJ_ID:
    case SPACE_VIEW_PROJ_ID:
      break;
    default:
      _projection = NULL;
      cerr << "ERROR: GDS()" << endl;
      cerr << "Grid TemplateNum (projection) - " << _gridTemplateNum << " not implemented" << endl;
      return; 
  }
  _projection = projectionTemplate;
  _sectionLen = _projection->getTemplateSize();

}


GDS::~GDS() 
{
  if (_projection != NULL)
    delete _projection;
}



void GDS::print(FILE *stream) const
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Grid Description Section:\n");
  fprintf(stream, "----------------------------------------------------\n");
  fprintf(stream, "GDS length %d\n", _sectionLen);
  fprintf(stream, "GDS section number %d\n", _sectionNum);
  if(_gridDefSource != 0) {
    fprintf(stream, "Grid Definition source - %d - translates to:\n", _gridDefSource);
    switch (_gridDefSource) {
    case 0:
      fprintf(stream, "     Specified in Code table 3.1\n");
      break;
    case 1:
      fprintf(stream, "     Predetermined grid definition, defined by originating centre\n");
      break;
    case 255:
      fprintf(stream, "     A grid definition does not apply to this product\n");
      break;
    }
  }

  fprintf(stream, "Number of data points %d\n", _numDataPoints);

  fprintf(stream, "List length, in bytes, of optional way of defining grid size %d\n", _listSize);
  fprintf(stream, "Definition of appended list defining number of grid points %d\n", _quasi_regularListInterp);
  switch (_quasi_regularListInterp) {
    case 0:
      fprintf(stream, "     There is no appended list\n");
      break;
    case 1:
      fprintf(stream, "     Numbers define number of points corresponding to\n");
      fprintf(stream, "     full coordinate circles (i.e. parallels), coordinate\n");
      fprintf(stream, "     values on each circle are multiple of the circle mesh,\n");
      fprintf(stream, "     and extreme coordinate values given in grid definition\n"); 
      fprintf(stream, "     in all rows (i.e. extreme longitudes) may not be reached\n");
      break;
    case 2:
      fprintf(stream, "     Numbers define number of points corresponding to\n");
      fprintf(stream, "     coordinate lines delimited by extreme coordinate\n");
      fprintf(stream, "     values given in grid definition (i.e. extreme\n");
      fprintf(stream, "     longitudes) which are present in each row\n");
      break;
    case  255:
      fprintf(stream, "     Missing\n");
      break;
  }

  fprintf(stream, "Grid  Template Number - table value - %d translates to:\n", _gridTemplateNum);
  switch (_gridTemplateNum) {
    case EQUIDISTANT_CYL_PROJ_ID:
      fprintf(stream, "     Latitude/longitude\n");
      break;
    case 1:
      fprintf(stream, "     Rotated latitude/longitude\n");
      break;
    case 2:
      fprintf(stream, "     Stretched latitude/longitude\n");
      break;
    case 3:
      fprintf(stream, "     Stretched and rotated latitude/longitude\n");
      break;
    case 10:
      fprintf(stream, "     Mercator\n");
      break;
    case 20:
      fprintf(stream, "     Polar stereographic\n");
      break;
    case 30:
      fprintf(stream, "     Lambert Conformal\n");
      break;
    case 40:
      fprintf(stream, "     Gaussian latitude/longitude\n");
      break;
    case 41:
      fprintf(stream, "     Rotated Gaussian latitude/longitude\n");
      break;
    case 42:
      fprintf(stream, "     Stretched Gaussian latitude/longitude\n");
      break;
    case 43:
      fprintf(stream, "     Stretched and rotated Gaussian latitude/longitude\n");
      break;
    case 50:
      fprintf(stream, "     Spherical harmonic coefficients\n");
      break;
    case 51:
      fprintf(stream, "     Rotated spherical harmonic coefficients\n");
      break;
    case 52:
      fprintf(stream, "     Stretched spherical harmonic coefficients\n");
      break;
    case 53:
      fprintf(stream, "     Stretched and rotated spherical harmonic coefficients\n");
      break;
    case 90:
      fprintf(stream, "     Space view perspective orthographic\n");
      break;
    case 100:
      fprintf(stream, "     Triangular grid based on an icosahedron\n");
      break;
    case 110:
      fprintf(stream, "     Equatorial azimuthal equidistant projection\n");
      break;
    case 120:
      fprintf(stream, "     Azimuth-range projection\n");
      break;
    case 65535:
      fprintf(stream, "     Missing\n");
      break;
  }
  fprintf(stream, "\n");
  _projection->print(stream);

}


int GDS::unpack(ui08 *gdsPtr)
{

   //
   // Length in bytes of the section
   //
   _sectionLen = _upkUnsigned4 (gdsPtr[0], gdsPtr[1], gdsPtr[2], gdsPtr[3]);

   _sectionNum = (int) gdsPtr[4];

   if (_sectionNum != 3) {
     cerr << "ERROR: GDS::unpack()" << endl;
     cerr << "Detecting incorrect section number, should be 3 but found section "
           << _sectionNum << endl;
     return( GRIB_FAILURE );
   }

   _gridDefSource = (int) gdsPtr[5];

   _numDataPoints = _upkUnsigned4 (gdsPtr[6], gdsPtr[7], gdsPtr[8], gdsPtr[9]);

   if(_gridDefSource != 0) {
     cerr << "ERROR: GDS::unpack()" << endl;
     cerr << "Grid definition source - " << _gridDefSource << " is unknown and not implemented" << endl;
     return (GRIB_FAILURE);
   }

   _listSize = (int) gdsPtr[10];

   _quasi_regularListInterp = (int) gdsPtr[11];

   _gridTemplateNum = _upkUnsigned2 (gdsPtr[12], gdsPtr[13]);

   // Gaussian quasi_regular grid is the only quasi_regular grid currently implemented
   if((_quasi_regularListInterp != 0 || _listSize != 0) && _gridTemplateNum != GAUSSIAN_LAT_LON_PROJ_ID) {
     cerr << "ERROR: GDS::unpack()" << endl;
     cerr << "List of numbers defining number of points is not implemented" << endl;
     return (GRIB_FAILURE);
   }
   if((_quasi_regularListInterp != 0 && _quasi_regularListInterp != 1) ||
      (_listSize != 0 && _listSize != 1 && _listSize != 2 && _listSize != 4)) {
     cerr << "ERROR: GDS::unpack()" << endl;
     cerr << "List of numbers size and interpretation is confusing." << endl;
     return (GRIB_FAILURE);
   }

   switch (_gridTemplateNum) {
     case EQUIDISTANT_CYL_PROJ_ID:
       _projection = new LatLonProj();
       break;
     case MERCATOR_PROJ_ID:
       _projection = new MercatorProj();
       break;
     case POLAR_STEREOGRAPHIC_PROJ_ID:
       _projection = new PolarStereoProj();
       break;
     case LAMBERT_CONFORMAL_PROJ_ID:
       _projection = new LambertConfProj();
       break;
     case GAUSSIAN_LAT_LON_PROJ_ID:
       _projection = new GausLatLonProj();
       break;
     case SPACE_VIEW_PROJ_ID:
       _projection = new SpaceViewProj();
       break;
     default:
       cerr << "ERROR: GDS::unpack()" << endl;
       cerr << "Grid TemplateNum (projection) - " << _gridTemplateNum << " not implemented" << endl;
       return (GRIB_FAILURE);

   }
   _projection->unpack(&gdsPtr[14]);

  return( GRIB_SUCCESS );
}


int GDS::pack(ui08 *gdsPtr)
{

  _pkUnsigned4(_sectionLen, &(gdsPtr[0]));

  gdsPtr[4] = (ui08) _sectionNum;

  if(_gridDefSource != 0) {
    cerr << "ERROR: GDS::pack()" << endl;
    cerr << "Using anything but pre-specified grid definitions is not implemented" << endl;
    return (GRIB_FAILURE);
  }

  gdsPtr[5] = (ui08) _gridDefSource;

  _pkUnsigned4(_numDataPoints, &(gdsPtr[6]));

  if(_quasi_regularListInterp != 0 || _listSize != 0) {
    cerr << "ERROR: GDS::pack()" << endl;
    cerr << "List of numbers defining number of points is not implemented" << endl;
    return (GRIB_FAILURE);
  }

  gdsPtr[10] = (ui08) _listSize;

  gdsPtr[11] = (ui08) _quasi_regularListInterp;

  _pkUnsigned2(_gridTemplateNum, &(gdsPtr[12]));

  // Pack based on projection type
   switch (_gridTemplateNum) {
     case EQUIDISTANT_CYL_PROJ_ID:
     case MERCATOR_PROJ_ID:
     case POLAR_STEREOGRAPHIC_PROJ_ID:
     case LAMBERT_CONFORMAL_PROJ_ID:
     case GAUSSIAN_LAT_LON_PROJ_ID:
     case SPACE_VIEW_PROJ_ID:
       return _projection->pack(&gdsPtr[14]);

     default:
       cerr << "ERROR: GDS::pack()" << endl;
       cerr << "Grid TemplateNum (projection) - " << _gridTemplateNum << " not implemented" << endl;
       return (GRIB_FAILURE);

   }

}

} // namespace Grib2


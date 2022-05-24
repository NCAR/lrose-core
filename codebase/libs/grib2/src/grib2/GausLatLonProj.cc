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
// GausLatLonProj - Gaussian Latitude/Longitude 
//              projection definition 
//
// Note:  pack and unpack routines are static
//        methods in <grib/GribSection.hh>
//////////////////////////////////////////////////

#include <iostream>
#include <math.h>

#include <grib2/GausLatLonProj.hh>
#include <grib2/GDS.hh>
#include <grib2/constants.h>

using namespace std;

namespace Grib2 {

const int GausLatLonProj::LAT_LON_SIZE = 72;

GausLatLonProj::GausLatLonProj() 
: GribProj()
{
  _pointsList = NULL;
}


GausLatLonProj::~GausLatLonProj() 
{
  if(_pointsList)
    delete[] _pointsList;
}


int GausLatLonProj::pack (ui08 *projPtr) 
{

  projPtr[0] = (ui08) _earthShape;

  projPtr[1] = (ui08) _radiusScaleFactor;

  GribSection::_pkUnsigned4(_radiusScaleValue, &(projPtr[2]));

  projPtr[6] = (ui08) _majorAxisScaleFactor;

  GribSection::_pkUnsigned4(_majorAxisScaleValue, &(projPtr[7]));

  projPtr[11] = (ui08) _minorAxisScaleFactor;

  GribSection::_pkUnsigned4(_minorAxisScaleValue, &(projPtr[12]));

  GribSection::_pkUnsigned4(_ni, &(projPtr[16]));

  GribSection::_pkUnsigned4(_nj, &(projPtr[20]));

  GribSection::_pkUnsigned4((int)(_basicAngleProdDomain / GDS::DEGREES_SCALE_FACTOR), &(projPtr[24]));

  GribSection::_pkUnsigned4((int)(_basicAngleSubdivisions / GDS::DEGREES_SCALE_FACTOR), &(projPtr[28]));

  GribSection::_pkSigned4((int)(_la1 / GDS::DEGREES_SCALE_FACTOR), &(projPtr[32]));

  GribSection::_pkSigned4((int)(_lo1 / GDS::DEGREES_SCALE_FACTOR), &(projPtr[36]));

  projPtr[40] = (ui08) _resolutionFlag;

  GribSection::_pkSigned4((int)(_la2 / GDS::DEGREES_SCALE_FACTOR), &(projPtr[41]));

  GribSection::_pkSigned4((int)(_lo2 / GDS::DEGREES_SCALE_FACTOR), &(projPtr[45]));

  GribSection::_pkUnsigned4((int)(_di / GDS::DEGREES_SCALE_FACTOR), &(projPtr[49]));

  GribSection::_pkUnsigned4(_nParalells, &(projPtr[53]));

  projPtr[57] = (ui08) _scanModeFlag;

  return( GRIB_SUCCESS );
}

int GausLatLonProj::unpack (ui08 *projPtr) 
{

  // Shape of the earth (see Code Table 3.2)
  _earthShape = (si32) projPtr[0]; 

  // Scale factor of radius of spherical earth
  _radiusScaleFactor = (si32) projPtr[1]; 

  //Scaled value of radius of spherical earth
  _radiusScaleValue 
            = GribSection::_upkUnsigned4 (projPtr[2], projPtr[3], projPtr[4], projPtr[5]);

  // Scale factor of major axis of oblate spheroid earth
  _majorAxisScaleFactor = (si32) projPtr[6]; 

  // Scaled value of major axis of oblate spheroid earth
  _majorAxisScaleValue =
           GribSection::_upkUnsigned4 (projPtr[7], projPtr[8], projPtr[9], projPtr[10]);

  // Scale factor of minor axis of oblate spheroid earth
  _minorAxisScaleFactor = (si32) projPtr[11]; 

  //Scaled value of minor axis of oblate spheroid earth
  _minorAxisScaleValue = 
           GribSection::_upkUnsigned4 (projPtr[12], projPtr[13], projPtr[14], projPtr[15]);

  // number of points along a parallel (undefined - all bits set to 1)
  _ni = GribSection::_upkUnsigned4 (projPtr[16], projPtr[17], projPtr[18], projPtr[19]);

  // number of points along  longitude meridian (undefined - all bits set to 1)
  _nj = GribSection::_upkUnsigned4 (projPtr[20], projPtr[21], projPtr[22], projPtr[23]);

  // Basic angle of the initial production domain (see Note 1)
  _basicAngleProdDomain = GDS::DEGREES_SCALE_FACTOR *
           (fl32) GribSection::_upkUnsigned4 (projPtr[24], projPtr[25], projPtr[26], projPtr[27]);

  // Subdivisions of basic angle used to define extreme longitudes and latitudes, 
  // and direction increments(see Note 1)
  _basicAngleSubdivisions = GDS::DEGREES_SCALE_FACTOR *
           (fl32) GribSection::_upkUnsigned4 (projPtr[28], projPtr[29], projPtr[30], projPtr[31]);

  float units = GDS::DEGREES_SCALE_FACTOR;
  if(_basicAngleProdDomain != 0.0)
    units = _basicAngleProdDomain / _basicAngleSubdivisions;

  // Latitude of first grid point (leftmost bit set for south latitude)
  _la1 = units * (fl32) GribSection::_upkSigned4(projPtr[32], projPtr[33], projPtr[34], projPtr[35]);

  // Longitude of first grid point (leftmost bit set for west longitude)
  _lo1 = units * (fl32) GribSection::_upkSigned4(projPtr[36], projPtr[37], projPtr[38], projPtr[39]);

  // Resolution and component flags
  _resolutionFlag = projPtr[40];

  // Latitude of last grid point (leftmost bit set for south latitude
  _la2 = units * (fl32) GribSection::_upkSigned4(projPtr[41], projPtr[42], projPtr[43],  projPtr[44]);

  // Longitude of last grid point (leftmost bit set for west longitude
  _lo2 = units * (fl32) GribSection::_upkSigned4(projPtr[45], projPtr[46], projPtr[47],  projPtr[48]);

  // Longitudinal Direction Increment  (undefined - all bits set to 1)
  _di = (fl32) GribSection::_upkUnsigned4(projPtr[49], projPtr[50], projPtr[51], projPtr[52]);
  if(_di != GribSection::S4MISSING)
     _di *= units;

  // Number of paralells between a pole and equater
  _nParalells = GribSection::_upkUnsigned4(projPtr[53], projPtr[54], projPtr[55], projPtr[56]);

  // Scanning mode flags
  _scanModeFlag = projPtr[57];
  
  if((si32)_nj != _nParalells * 2)
  {
     cerr << "ERROR: GausLatLonProj::unpack()" << endl;
     cerr << "Quasi-regular Lat/Lon sub grid is unimplemented" << endl;
     return (GRIB_FAILURE);
  }

  if(_nj == GribSection::U4MISSING)
  {
     cerr << "ERROR: GausLatLonProj::unpack()" << endl;
     cerr << "Quasi-regular Lat/Lon grid is unimplemented" << endl;
     return (GRIB_FAILURE);
  }

  _maxNi = _ni;
  if(_ni == GribSection::U4MISSING || _di == GribSection::S4MISSING)
  {
    _maxNi = 0;
    int numDataPoints = GribSection::_upkUnsigned4 (projPtr[-8], projPtr[-7], projPtr[-6], projPtr[-5]);
    int totalPoints = 0;
    int intSize = (int) projPtr[-4];
    _pointsList = new si32[_nj];
    int loc = 58;
    for(ui32 a = 0; a < _nj; a++) {
      if(intSize == 1)
	_pointsList[a] = (int) projPtr[loc];
      else if(intSize == 2)
	_pointsList[a] = GribSection::_upkUnsigned2(projPtr[loc], projPtr[loc+1]);
      else if(intSize == 4)
	_pointsList[a] = GribSection::_upkUnsigned4(projPtr[loc], projPtr[loc+1], projPtr[loc+2], projPtr[loc+3]);
      totalPoints += _pointsList[a];
      if(_pointsList[a] > (si32)_maxNi)
	_maxNi = _pointsList[a];
      loc += intSize;
    }
    if(totalPoints != numDataPoints) {
      cerr << "ERROR: GausLatLonProj::unpack()" << endl;
      cerr << "Quasi-regular Lat/Lon grid reading failure, list numbers does not mach gds:numPoints" << endl;
      return (GRIB_FAILURE);
    }
  }
  /*
  fl32 *lats;
  getGaussianLats(&lats);

  fl32 *lons;
  for(int i = 0; i < _nParalells*2; i++) {
    cout << lats[i] << " : ";

    int nlon = getQuasiLons(&lons, i);
    for(int j = 0; j < nlon; j++) {
      cout << lons[j] << " ";
    }
    cout << endl;
    delete[] lons;
  }

  delete[] lats;
  */
  return( GRIB_SUCCESS );
}


void GausLatLonProj::print(FILE *stream) const
{
  fprintf(stream, "Gaussian Latitude/longitude projection:\n");

  switch (_earthShape) {
    case 0:
      fprintf(stream, "Earth assumed spherical with radius = 6367.4700 km\n");
      break;
    case 1:
      fprintf(stream, "Earth assumed spherical with radius specified by data producer\n");
      break;
    case 2:
      fprintf(stream, "Earth assumed oblate spheroid with size as determined by IAU in 1965\n");
      fprintf(stream, "(major axis = 6378.160 km, minor axis = 6356.775 km, f = 1/297.0)\n");
	break;
    case 3:
      fprintf(stream, "Earth assumed oblate spheroid with major and minor axes specified by data producer\n");
      break;
    case 4:
      fprintf(stream, "Earth assumed oblate spheroid with size as determined by IAG-GRS80 model\n");
      fprintf(stream, "(major axis = 6378.1370 km, minor axis = 6356.752314 km, f = 1/298.257222101)\n");
	break;
    case 5:
      fprintf(stream, "Earth assumed represented by WGS84 (as used by ICAO since 1998)(Uses IAG-GRS80 as a basis)\n");
      break;
    case 6:
      fprintf(stream, "Earth assumed spherical with radius = 6371.2290 km\n");
      break;
    case 255:
      fprintf(stream, "Earth Shape flag Missing\n");
      break;
    default:
      if (_earthShape >= 7 && _earthShape <= 191)
         fprintf(stream, "Earth shape in reserved area, value found is %d\n", _earthShape);
      else if (_earthShape >= 192 && _earthShape <= 254)
         fprintf(stream, "Earth shape in local reserved area, value found is %d\n", _earthShape);

  }
  fprintf(stream, "Scale factor of radius of spherical earth %d\n", _radiusScaleFactor);
  fprintf(stream, "Scaled value of radius of spherical earth %d\n", _radiusScaleValue);
  fprintf(stream, "Scale factor of major axis of oblate spheroid earth %d\n", _majorAxisScaleFactor);
  fprintf(stream, "Scaled value of major axis of oblate spheroid earth %d\n", _majorAxisScaleValue);
  fprintf(stream, "Scale factor of minor axis of oblate spheroid earth %d\n", _minorAxisScaleFactor);
  fprintf(stream, "Scaled value of minor axis of oblate spheroid earth %d\n", _minorAxisScaleValue);
  fprintf(stream, "Number of points along latitude circle %d\n", _ni);
  fprintf(stream, "Number of points along longitude meridian %d\n", _nj);
  fprintf(stream, "Basic angle of the initial production domain %f\n", _basicAngleProdDomain);
  fprintf(stream, "Subdivisions of basic angle used to define extreme longitudes and latitudes, %f\n",_basicAngleSubdivisions);
  fprintf(stream, "Latitude of first grid point %f\n", _la1);
  fprintf(stream, "Longitude of first grid point %f\n", _lo1);
  fprintf(stream, "Resolution flag byte %d\n", _resolutionFlag);
  if ((_resolutionFlag & 32) == 32)
    fprintf(stream, "    i direction increments given\n");
  else
    fprintf(stream, "    i direction increments not given\n");

  if ((_resolutionFlag & 16) == 16)
    fprintf(stream, "    j direction increments given\n");
  else
    fprintf(stream, "    j direction increments not given\n");

  if ((_resolutionFlag & 8) == 8) {
    fprintf(stream, "    u- and v- components of vector quantities resolved relative to the defined\n");
    fprintf(stream, "    grid in the direction of increasing x and y (or i and j) coordinates respectively\n");
  }
  else {
    fprintf(stream, "    u- and v- components of vector quantities resolved relative to easterly\n");
    fprintf(stream, "     and northerly directions\n");
  }

  fprintf(stream, "Latitude of last grid point %f\n", _la2);
  fprintf(stream, "Longitude of last grid point %f\n", _lo2);
  fprintf(stream, "Longitudinal Direction Increment %f\n", _di);
  fprintf(stream, "Number of paralells between a pole and equater %d\n", _nParalells);
  fprintf(stream, "Scanning mode flags %d\n", _scanModeFlag);

  if ((_scanModeFlag & 16) == 0) {
    fprintf(stream, "    All rows scan in the same direction\n");
  }
  else {
    fprintf(stream, "    Adjacent rows scans in the opposite direction\n");
  }

  if ((_scanModeFlag & 32) == 0) {
    fprintf(stream, "    Adjacent points in i (x) direction are consecutive\n");
  }
  else {
    fprintf(stream, "    Adjacent points in j (y) direction are consecutive\n");
  }

  if ((_scanModeFlag & 64) == 0) {
    fprintf(stream, "    Points of first row or column scan in the -j (-y) direction\n");
  }
  else {
    fprintf(stream, "    Points of first row or column scan in the +j (+y) direction\n");
  }

  if ((_scanModeFlag & 128) == 0) {
    fprintf(stream, "    Points of first row or column scan in the +i (+x) direction\n");
  }
  else {
    fprintf(stream, "    Points of first row or column scan in the -i (-x) direction\n");
  }

}

/*  

 */
int GausLatLonProj::getQuasiLons(fl32 **lons, int rowIndex)
{
  int nlon = -1;
  if(_pointsList == NULL && _ni == GribSection::U4MISSING)
    return nlon;
  if(_pointsList == NULL)
    nlon = _ni;
  else
    nlon = _pointsList[rowIndex];

  double d = _lo2 + (_lo2 / (double)(_maxNi-1)) - _lo1;

  (*lons) = new fl32[nlon];

  if(d < 0.0) {
    d += 360.;
    _lo1 -= 360.;
  }

  double dj = d / (double)nlon;

  for(int i = 0; i < nlon; i++) 
  {
    (*lons)[i] = _lo1 + (i * dj);
  }

  return nlon;
}

/*  
 *  Adapted from gauss2lats.m program from Tom Holt.
 *  Adapted from NCAR fortran program by Tom Holt
 *  http://www.mathworks.com/matlabcentral/fileexchange/2586-gauss2lats/content/gauss2lats.m
 *
 */
void GausLatLonProj::getGaussianLats(fl32 **lats)
{
  int nlat = _nParalells * 2;
  (*lats) = new fl32[nlat];

  double acon = 180.0/M_PI;

  // convergence criterion for iteration of cos latitude
  float xlim = 1.0E-7;

  double *cosc = new double[nlat];
  double *sinc = new double[nlat];
  double *colat = new double[nlat];

  // initialise arrays
  for(int i = 0; i < nlat; i++)
  {
    cosc[i] = 0.;
    sinc[i] = 0.;
    colat[i] = 0.;
  }

  // the number of zeros between pole and equator
  int nzero = _nParalells;

  // set first guess for cos(colat)
  for(int i = 1; i <= nzero; i++)
    cosc[i-1]=sin((i-0.5)*M_PI/nlat+M_PI*0.5);


  // constants for determining the derivative of the polynomial
  int fi = nlat;
  double fi1 = fi + 1.0;
  double a = fi*fi1/sqrt(4.0*fi1*fi1-1.0);
  double b = fi1*fi/sqrt(4.0*fi*fi-1.0);

  // loop over latitudes, iterating the search for each root
  for(int i = 1; i <= nzero; i++)
  {
    // determine the value of the ordinary Legendre polynomial for the current guess root
    double g = _gord(nlat, cosc[i-1]);
    // determine the derivative of the polynomial at this point
    double gm = _gord(nlat-1, cosc[i-1]);
    double gp = _gord(nlat+1, cosc[i-1]);
    double gt = (cosc[i-1]*cosc[i-1]-1.0)/(a*gp-b*gm);
    // update the estimate of the root
    double delta = g * gt;
    cosc[i-1] = cosc[i-1] - delta;
 
    // if convergence criterion has not been met, keep trying
    while(fabs(delta) > xlim)
    { 
        g = _gord(nlat,cosc[i-1]);
        gm = _gord(nlat-1,cosc[i-1]);
        gp = _gord(nlat+1,cosc[i-1]);
        gt = (cosc[i-1]*cosc[i-1]-1.0)/(a*gp-b*gm);
        delta = g * gt;
        cosc[i-1] = cosc[i-1] - delta;   
    }
  }

  // determine the colatitudes and sin(colat) and weights over sin**2
  for(int i = 1; i <= nzero; i++)
  {
    colat[i-1] = acos(cosc[i-1]);
    sinc[i-1] = sin(colat[i-1]);
  }


  // determine the southern hemisphere values by symmetry
  for(int i = nzero+1; i <= nlat; i++)
  {
    cosc[i-1] = -cosc[nlat-i];
    colat[i-1] = M_PI - colat[nlat-i];
    sinc[i-1] = sinc[nlat-i];
  }

  int reverse = 0;
  fl32 la1 = _la1;
  fl32 la2 = _la2;
  if(_la1 < _la2) {
    reverse = 1;
    la1 = _la2;
    la2 = _la1;
  }

  // calculate latitudes and latitude spacing
  ui32 count = 0;
  //double ylat = 90.;
  for(int i = 1; i <= nlat; i++)
  {
    double lat = acos(sinc[i-1])*acon;
    if(i > nzero) lat *= -1.0;
    if(lat < la1 + .001 && lat > la2 - .001) {
      (*lats)[count] = lat;
      count++;
    }

    //dlat[i-1] = xlat[i-1]-ylat;
    //ylat = xlat[i-1];
  }

  if(_nj != count) {
    (*lats) = NULL;
  }

  if(reverse) {
    for(ui32 i = 0; i < count/2; i++) {
      double tmp = (*lats)[i];
      (*lats)[i] = (*lats)[count-i];
      (*lats)[count-i] = tmp;
    }
  }

  delete[] cosc;
  delete[] sinc;
  delete[] colat;

}


/*
 * This function calculates the value of an ordinary Legendre polynomial at a latitude.
 *          inputs are:
 *                n = the degree of the polynomial
 *                x = cos(colatitude)
 *         outputs are:
 *              ggg = the value of the Legendre polynomial of degree n at 
 *                    latitude asin(x)
 */
double GausLatLonProj::_gord(int n, double x)
{

  // determine the colatitude
  double colat=acos(x);

  double c1 = sqrt(2.0);

  for(int i=1; i <= n; i++)
    c1 = c1*sqrt(1.0-1.0/(4*i*i));

  double fn = n;
  double ang = fn * colat;
  double s1 = 0.0;
  double c4 = 1.0;
  double a = -1.0;
  double b = 0.0;

  for(int k=0; k <= n; k += 2)
  {
    if(k == n) 
      c4 = 0.5*c4;
    
    s1 = s1+c4*cos(ang);
    a = a+2.0;
    b = b+1.0;
    double fk = k;
    ang = colat*(fn-fk-2.0);
    c4 = (a*(fn-b+1.0)/(b*(fn+fn-a)))*c4;
  }
  double ggg = s1*c1;
  return ggg;
}

} // namespace Grib2


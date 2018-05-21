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
// gds - Grid Description Section
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
// 
//////////////////////////////////////////////////

#include <grib/constants.h>
#include <grib/gds_.hh>

const double gds::DEGREES_SCALE_FACTOR = 0.001;
const double gds::GRID_SCALE_FACTOR = 0.001;
using namespace std;

gds::gds() :  GribSection(), _expectedSize(EXPECTED_SIZE) 
{

}

gds::~gds() 
{
}

void gds::setProjection(const gds_t &new_projection)
{
  _data_struct = new_projection;
}


bool gds::isRegular() const
{
  return(_data_struct.numVertical != 0 || _data_struct.verticalOrPoints == NO_VERTICAL_POINTS);
}


void gds::print(FILE *stream) const
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Grib Grid Description Section (gds):\n");
  fprintf(stream, "--------------------------\n");
  fprintf(stream, "GDS length %d\n", _nBytes);
  fprintf(stream, "Number of vertical coordinate parameters %d\n", _data_struct.numVertical);
  fprintf(stream, "Vertical coord params, or numbers of pts per row %d\n", _data_struct.verticalOrPoints);
  fprintf(stream, "Projection type Id %d\n", _data_struct.gridType);
  fprintf(stream, "Data ordering %d\n", _data_struct.dataOrder);
  fprintf(stream, "Grid orientation %d\n", _data_struct.gridOrientation);
  fprintf(stream, "nx %d\n", _data_struct.nx);
  fprintf(stream, "ny %d\n", _data_struct.ny);
  fprintf(stream, "dx %f\n", _data_struct.dx);
  fprintf(stream, "dy %f\n", _data_struct.dy);
  fprintf(stream, "originLat %f\n", _data_struct.originLat);
  fprintf(stream, "originLon %f\n", _data_struct.originLon);
  fprintf(stream, "lov %f\n", _data_struct.lov);
  fprintf(stream, "latin1 %f\n", _data_struct.latin1);
  fprintf(stream, "latin2 %f\n", _data_struct.latin2);
  fprintf(stream, "lastLat %f\n", _data_struct.lastLat);
  fprintf(stream, "lastLon %f\n", _data_struct.lastLon);

  printQuasiList(stream);
}


void gds::printQuasiList(FILE *stream) const
{
  if (isRegular())
    return;

  // print the list length 
  int listLength = 0; 
  if (_data_struct.numVertical != 0) 
    listLength = _data_struct.numVertical; 
  else 
    listLength = _data_struct.ny; 
  fprintf(stream, "Size of points list: %d\n", listLength);

  // print the list itself 
  for (size_t listIndex = 0; listIndex < _numPtsPerRow.size(); listIndex++) { 
    fprintf(stream, "%d ", _numPtsPerRow[listIndex]); 
  }
  fprintf(stream, "\n");
}


int gds::getNumGridPoints() const
{
  if (isRegular()) {
    // regular grid
    return _data_struct.nx * _data_struct.ny;
  }

  // add up the points on each row
  int totalGridPoints = 0;
  for (vector<int>::const_iterator itr = _numPtsPerRow.begin(); itr != _numPtsPerRow.end(); itr++) {
    totalGridPoints += *itr;
  }

  return totalGridPoints;
}


int gds::unpackPtsPerRow(ui08 *gdsPtr)
// This routine does NOT handle those situations where we have
// a list of vertical coordinate parameters.
{
  // clear any previous list
  _numPtsPerRow.clear();

  if (_data_struct.numVertical != 0 || _data_struct.verticalOrPoints == NO_VERTICAL_POINTS)
    return GRIB_SUCCESS;

  // retrieve the list length 
  int listLength = _data_struct.ny; 
 
  // retrieve the list itself 
  for (int listIndex = 0; listIndex < listLength; listIndex++) { 
    _numPtsPerRow.push_back(_upkSigned2( 
      gdsPtr[_data_struct.verticalOrPoints + 2 * listIndex + 1], 
      gdsPtr[_data_struct.verticalOrPoints + 2 * listIndex])); 
  }

  return( GRIB_SUCCESS );
}


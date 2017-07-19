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

#ifndef __GDS_EQUIDISTANT_HH__
#define __GDS_EQUIDISTANT_HH__

#include <grib/constants.h>
#include <grib/gds_.hh>

//=============================================================================
//   Equidistant (Lat/Lon) Projection class
//=============================================================================

using namespace std;

class gds_equidistant : public gds {
public:
  gds_equidistant() : gds() {
    _nBytes = NUM_SECTION_BYTES;
  }
  //virtual ~gds_equidistant() { }

  virtual int unpack( ui08 *gdsPtr ) {
    // reset private variables
    _data_struct.gridType = EQUIDISTANT_CYL_PROJ_ID;
    _data_struct.numVertical = 0;
    _data_struct.verticalOrPoints = 0;
    _data_struct.nx = 0;
    _data_struct.ny = 0;
    _data_struct.resMode = 0;
    _data_struct.lov = 0.0;
    _data_struct.dx = 0.0;
    _data_struct.dy = 0.0;
    _data_struct.scanMode = 0;
    _data_struct.originLat = 0.0;
    _data_struct.originLon = 0.0;
    _data_struct.latin1 = 0.0;
    _data_struct.latin2 = 0.0;
    _data_struct.resolutionFlag = 0;
    _data_struct.dataOrder = DO_XY;
    _data_struct.gridOrientation = GO_SN_WE;
    _numPtsPerRow.clear();
   
    //
    // Length in bytes of the section
    //
    _nBytes = _upkUnsigned3( gdsPtr[0], gdsPtr[1], gdsPtr[2] );
    
    //
    // Check that section size is an expected value. GMC has encountered RUC files
    // with corrupt records. This check is useful in detecting bad records.
    //
    if(_nBytes > _expectedSize) {
      cout << "ERROR: Possible corrupt record. GDS size in bytes is " << 
	_nBytes << endl << "expected size in bytes is " << _expectedSize << endl;
      cout << "If GDS size is correct use setExpectedSize or GribRecord::setPdsExpectedSize method to pass test." 
	   << endl;
      return( GRIB_FAILURE );
    }
    
    // get the coordinate parameters
    _data_struct.numVertical = (int)gdsPtr[3];
    _data_struct.verticalOrPoints = (int)gdsPtr[4];
    
    //
    // Get the projection type
    //
    _data_struct.gridType = (grid_type_t)gdsPtr[5];
    
    if (_data_struct.gridType != EQUIDISTANT_CYL_PROJ_ID) {
      cout << "Error: Attempting to unpack GDS section with Equidistant (lat/lon) virtual function ";
      cout << "but GDS section gridType is not EQUIDISTANT_CYL_PROJ_ID" << endl;
      return (GRIB_FAILURE);
    }
   
    _data_struct.nx = _upkUnsigned2(gdsPtr[6], gdsPtr[7]);  // along latitude circle
    _data_struct.ny = _upkUnsigned2(gdsPtr[8], gdsPtr[9]);  // along longitude meridian
   
    // Latitude of first grid point (leftmost bit set for south latitude
    _data_struct.originLat = DEGREES_SCALE_FACTOR * (double) _upkSigned3(gdsPtr[10], gdsPtr[11], gdsPtr[12]);
    // Longitude of first grid point (leftmost bit set for west longitude
    _data_struct.originLon = DEGREES_SCALE_FACTOR * (double) _upkSigned3(gdsPtr[13], gdsPtr[14], gdsPtr[15]);
      
    // Resolution and component flags
    _data_struct.resolutionFlag = gdsPtr[16];

    // Latitude of last grid point (leftmost bit set for south latitude
    _data_struct.lastLat = DEGREES_SCALE_FACTOR * (double) _upkSigned3(gdsPtr[17], gdsPtr[18], gdsPtr[19]);
    // Longitude of last grid point (leftmost bit set for west longitude
    _data_struct.lastLon = DEGREES_SCALE_FACTOR * (double) _upkSigned3(gdsPtr[20], gdsPtr[21], gdsPtr[22]);

    // Longitudinal Direction Increment  (undefined - all bits set to 1)
    _data_struct.dx = GRID_SCALE_FACTOR * (double) _upkUnsigned2(gdsPtr[23], gdsPtr[24]);

    // Latitudinal Direction Increment (undefined - all bits set to 1)t
    _data_struct.dy = GRID_SCALE_FACTOR * (double) _upkUnsigned2(gdsPtr[25], gdsPtr[26]);
  
    // Scanning mode flags
    _data_struct.scanMode = gdsPtr[27];

    //
    // data ordering
    if ((_data_struct.scanMode & 32) == 0) {
      _data_struct.dataOrder = DO_XY;
    }
    else {
      _data_struct.dataOrder = DO_YX;
    }

    //
    // grid orientation
    if ((_data_struct.scanMode & 192) == 64) {
      _data_struct.gridOrientation = GO_SN_WE;
    }
    else if ((_data_struct.scanMode & 192) == 0) {
      _data_struct.gridOrientation = GO_NS_WE;
    }
    else if ((_data_struct.scanMode & 192) == 192) {
      _data_struct.gridOrientation = GO_SN_EW;
    }
    else if ((_data_struct.scanMode & 192) == 128) {
      _data_struct.gridOrientation = GO_NS_EW;
    }

    unpackPtsPerRow(gdsPtr);

    return( GRIB_SUCCESS );
  }


  virtual int pack( ui08 *gdsPtr ) {
    // Length in bytes of the section
    _pkUnsigned3(_nBytes, &(gdsPtr[0]));
    
    // vertical coordinate parameters
    gdsPtr[3] = (ui08)_data_struct.numVertical;
    gdsPtr[4] = (ui08)_data_struct.verticalOrPoints;
    
    // Projection type
    gdsPtr[5] = (ui08)_data_struct.gridType;

    if (_data_struct.gridType != EQUIDISTANT_CYL_PROJ_ID) {
      cout << "Error: Attempting to unpack GDS section with Equidistant (lat/lon) virtual function ";
      cout << "but GDS section gridType is not EQUIDISTANT_CYL_PROJ_ID" << endl;
      return (GRIB_FAILURE);
    }

    // Number of points in X and Y
    
    _pkUnsigned2(_data_struct.nx, &(gdsPtr[6]));
    _pkUnsigned2(_data_struct.ny, &(gdsPtr[8]));
    
    // first point latitude and longitude
    
    _pkSigned3((int)(_data_struct.originLon / DEGREES_SCALE_FACTOR), &(gdsPtr[10]));
    _pkSigned3((int)(_data_struct.originLat / DEGREES_SCALE_FACTOR), &(gdsPtr[13]));
    
    
    //
    gdsPtr[16] = (ui08)_data_struct.resolutionFlag;
    
    // Latitude of last grid point (leftmost bit set for south latitude)
    _pkSigned3((int)(_data_struct.lastLon / DEGREES_SCALE_FACTOR), &(gdsPtr[17]));

    // Longitude of last grid point (leftmost bit set for west longitude)
    _pkSigned3((int)(_data_struct.lastLat  / DEGREES_SCALE_FACTOR), &(gdsPtr[20]));

    // Longitudinal Direction Increment  (undefined - all bits set to 1)
    _pkUnsigned2((int)(_data_struct.dx / DEGREES_SCALE_FACTOR), &(gdsPtr[23]));

    // Latitudinal Direction Increment (undefined - all bits set to 1)
    _pkUnsigned2((int)(_data_struct.dy / DEGREES_SCALE_FACTOR), &(gdsPtr[25]));

    // Scanning mode flags    
    _data_struct.scanMode = 0;
    
    if(_data_struct.dataOrder == DO_XY) {
      _data_struct.scanMode = 0;
    }
    else {
      _data_struct.scanMode = 32;
    }

    if(_data_struct.gridOrientation == GO_SN_WE) {
      _data_struct.scanMode += 64;
    }
    else if(_data_struct.gridOrientation == GO_NS_WE) {
      _data_struct.scanMode += 0;
    }
    else if(_data_struct.gridOrientation == GO_SN_EW) {
      _data_struct.scanMode += 192;
    }
    else if(_data_struct.gridOrientation == GO_NS_EW) {
      _data_struct.scanMode += 128;
    }

    gdsPtr[27] = _data_struct.scanMode;

    gdsPtr[28] = 0;
    gdsPtr[29] = 0;
    gdsPtr[30] = 0;
    gdsPtr[31] = 0;

    
    return GRIB_SUCCESS;
  }


  //virtual void print(FILE *stream) const {  }

private:


};


#endif  // __GDS_EQUIDISTANT_HH__

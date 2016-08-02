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
/////////////////////////////////////////////////////////////////////
// MrmsReader top-level application class header.
//
// Reads MRMS (Multi-Radar Multi-Sensor) binary data.
//
////////////////////////////////////////////////////////////////////

#ifndef MRMS_READER
#define MRMS_READER

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

class MrmsReader {
 public:

  // Constructors
  MrmsReader();
  MrmsReader(bool debug);
  MrmsReader(bool debug, int swap_flag);

  // Destructor
  ~MrmsReader();

  int readFile(char *file);

  int getNx() { return _nx; };
  int getNy() { return _ny; };
  int getNz() { return _nz; };
  float getDx() { return _dx; };
  float getDy() { return _dy; };

  long getTime() { return _epoch_seconds; };
  float getMissingVal() { return (float)_missing_val; };
  float getMinX() { return _nw_lon; };
  float getMinY();

  float *getData() { return _data; };
  char *getVarName() { return &_varname[0]; };
  char *getVarUnits() { return &_varunit[0]; };
  float *getZhgt() { return _zhgt; };
      
private:

  void _clear();

  void _reOrderNS_2_SN (float *data, int numX, int numY);

  template < class Data_Type >
  void byteswap( Data_Type &data );
  template < class Data_Type >
  void byteswap( Data_Type *data_array, int num_elements );

  bool _debug;         // Debug Flag
  char _varname[20];   // Name of variable
  char _varunit[6];    // Units of variable
  int _nradars;        // Number of radars used in product
  vector<string> _radarnam;   // List of radars used in product
  int _var_scale;      // Value used to scale the variable data
  int _missing_val;    // Value to indicate missing data
  float _nw_lon;       // Longitude of NW grid cell (center of cell)
  float _nw_lat;       // Latitude of NW grid cell (center of cell)
  int _nx;             // Number of columns in field
  int _ny;             // Number of rows in field
  float _dx;           // Size of grid cell (degrees longitude)
  float _dy;           // Size of grid cell (degrees latitude)
  float *_zhgt;        // Height of vertical levels
  int _nz;             // Number of vertical levels
  long _epoch_seconds; // valid time (seconds since Jan. 1, 1970 00:00:00 UTC)
  int _swap_flag;

  float *_data;       // Variable data (un-scaled) in 1D array

};


#endif

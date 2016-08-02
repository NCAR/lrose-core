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
//
// Usgs2Spdb.hh
//
// Jason Craig, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 2006
//
///////////////////////////////////////////////////////////////
//
// Usgs2Spdb reads USGS data from argc or an ASCII file, converts them to
// usgsData_t format (rapformats library) and stores them in SPDB.
//
////////////////////////////////////////////////////////////////

#ifndef Usgs2Spdb_H
#define Usg2Spdb_H

#include <rapformats/UsgsData.hh>
#include <Spdb/DsSpdb.hh>
#include "Params.hh"
using namespace std;

class Usgs2Spdb {
  
public:

  // constructor

  Usgs2Spdb(Params *tdrpParams);

  // destructor
  
  ~Usgs2Spdb();

  int processFile(const char *file_path);

  int saveVolcano(char *sent, char *title, char *sender, char *lat, char *lon, char *alt, char *color, char *time, char *id);

  int saveEarthquake(char *sent, char *title, char *id, char *sender, char *version, char *magnitude, char *magnitudeType,
		   char *time, char *lat, char *lon, char *depth, char *horizontalError, char *verticalError,
		   char *stations, char *phases, char *distance, char *RMSError, char *azimuthalGap);

private:

  int _parseTime(char *time);

  void _saveData(UsgsData::usgsData_t data, int time, si32 dataType);

  int _writeData();

  DsSpdb _spdb;
  Params *_params;
};

#endif

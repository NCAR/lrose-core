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
//////////////////////////////////////////////////////////////////////////
// 
// Class for getting location of TDWR station
// 
// Mike Dixon, EOL, NCAR, Boulder, CO, 80307, USA
// Nov 2013
//
//////////////////////////////////////////////////////////////////////////
// Lat/Lon from:
//
//  http://www.wispa.org/tdwr-locations-and-frequencies
//
// Ground heights from google:
//
//  http://www.daftlogic.com/sandbox-google-maps-find-altitude.htm
//
// Radar height = ground height plus a nominal 30 meters.
//////////////////////////////////////////////////////////////////////////

#include <Radx/TdwrLoc.hh>
#include <Radx/RadxPath.hh>
#include <cmath>
using namespace std;

const TdwrLoc::locInfo_t TdwrLoc::_locInfo[_nStations] = {

  { "ADW",  38.695280,  -76.845000,     72,    102,   5.635 },
  { "ATL",  33.646193,  -084.262233,   295,    325,   5.615 },
  { "BNA",  35.979079,  -086.661691,   217,    247,   5.605 },
  { "BOS",  42.158060,  -070.933890,    46,     76,   5.610 },
  { "BWI",  39.090560,  -076.630000,    58,     88,   5.645 },
  { "CLE",  41.289372,  -082.007419,   250,    280,   5.645 },
  { "CLT",  35.337269,  -080.885006,   229,    259,   5.608 },
  { "CMH",  40.006110,  -082.715560,   316,    346,   5.605 },
  { "CVG",  38.897780,  -084.580280,   284,    314,   5.610 },
  { "DAL",  32.924940,  -096.968473,   167,    197,   5.608 },
  { "DAY",  40.021376,  -084.123077,   283,    313,   5.640 },
  { "DCA",  38.758853,  -076.961837,    77,    107,   5.615 },
  { "DEN",  39.727220,  -104.526390,  1719,   1749,   5.615 },
  { "DFW",  33.064286,  -096.915554,   169,    199,   5.640 },
  { "DTW",  42.111110,  -083.515000,   201,    231,   5.615 },
  { "EWR",  40.593397,  -074.270164,     6,     36,   5.620 },
  { "FLL",  26.142601,  -080.343820,     2,     32,   5.645 },
  { "HOU",  29.515852,  -095.241692,    11,     41,   5.645 },
  { "IAD",  39.083667,  -077.529224,   109,    139,   5.605 },
  { "IAH",  30.064720,  -095.567500,    49,     79,   5.605 },
  { "ICT",  37.506844,  -097.437228,   387,    417,   5.603 },
  { "IND",  39.636556,  -086.435286,   226,    256,   5.605 },
  { "JFK",  40.588633,  -073.880303,     4,     34,   5.647 },
  { "LAS",  36.144000,  -115.007000,   606,    636,   5.645 },
  { "MCI",  39.498610,  -094.741670,   313,    343,   5.605 },
  { "MCO",  28.343125,  -081.324674,    23,     53,   5.640 },
  { "MDW",  41.651400,  -087.729400,   200,    230,   5.645 },
  { "MEM",  34.896044,  -089.992727,   113,    143,   5.610 },
  { "MIA",  25.757083,  -080.491076,     3,     33,   5.605 },
  { "MKE",  42.819440,  -088.046110,   249,    279,   5.603 },
  { "MSP",  44.870902,  -092.932257,   316,    336,   5.610 },
  { "MSY",  30.021389,  -090.402919,     2,     32,   5.645 },
  { "OKC",  35.276110,  -097.510000,   364,    394,   5.603 },
  { "ORD",  41.796589,  -087.857628,   198,    228,   5.615 },
  { "PBI",  26.687812,  -080.272931,     8,     38,   5.630 },
  { "PHL",  39.950061,  -075.069979,    11,     41,   5.610 },
  { "PHX",  33.420352,  -112.163180,   314,    344,   5.610 },
  { "PIT",  40.501066,  -080.486586,   387,    417,   5.615 },
  { "RDU",  36.001401,  -078.697942,   122,    152,   5.647 },
  { "SDF",  38.045810,  -085.610641,   189,    219,   5.646 },
  { "SJU",  18.473940,  -066.178910,    25,     55,   5.610 },
  { "SLC",  40.967222,  -111.929722,  1286,   1316,   5.610 },
  { "STL",  38.804691,  -090.488558,   169,    199,   5.610 },
  { "TPA",  27.858670,  -082.517550,     3,     33,   5.620 },
  { "TUL",  36.070184,  -095.826313,   217,    247,   5.605 }

};

///////////////////////////////////////////////////////////////
// constructor

TdwrLoc::TdwrLoc()

{
  _name.clear();
  _latitudeDeg = 0.0;
  _longitudeDeg = 0.0;
  _groundHtM = 0.0;
  _radarHtM = 0.0;
  _freqGhz = 0.0;
}

///////////////////////////////////////////////////////////////
// destructor

TdwrLoc::~TdwrLoc()

{

}

///////////////////////////////////////////////////////////////
// load up location from the station name

int TdwrLoc::loadLocationFromName(const string &name)

{
  
  for (int ii = 0; ii < _nStations; ii++) {
    string tableName = _locInfo[ii].name;
    if (tableName.compare(name) == 0) {
      _load(ii);
      return 0;
    }
  }

  return -1;

}

///////////////////////////////////////////////////////////////
// load up location from the file path

int TdwrLoc::loadLocationFromFilePath(const string &path)

{
  
  RadxPath fpath(path);
  string fileName = fpath.getFile();

  for (int ii = 0; ii < _nStations; ii++) {
    string tableName = _locInfo[ii].name;
    if (fileName.find(tableName) != string::npos) {
      _load(ii);
      return 0;
    }
  }

  return -1;

}

///////////////////////////////////////////////////////////////
// load up location based on index

void TdwrLoc::_load(int index)

{

  if (index >= _nStations) {
    return;
  }
  
  const locInfo_t &loc = _locInfo[index];

  _name = loc.name;
  _latitudeDeg = loc.latitudeDeg;
  _longitudeDeg = loc.longitudeDeg;
  _groundHtM = loc.groundHtM;
  _radarHtM = loc.radarHtM;
  _freqGhz = loc.freqGhz;

}


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
// StationLoc.h
//
// StationLoc object
//
// James Yu, III, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2000
//
///////////////////////////////////////////////////////////////
//
// StationLoc finds out the closest point from the mouse click point 
//
///////////////////////////////////////////////////////////////

#ifndef StationLoc_HH
#define StationLoc_HH

#include <string>
#include <vector>

using namespace std;

class StationLoc
{
 
public:

  //////////////////////////////////////////
  /// This class, info, defines four public variables. 
  /// These variables, two for double and two for string, are for 
  /// storing the data which was read from input data file into 
  /// the system main memory. 

  class Info{
  public:
    double ilon, ilat, ialt;
    string iname, itype;
  };

  StationLoc();    // Constructor
  ~StationLoc();   // Destructor

  void clear();    // clear out info, ready for next read
  
  // read in the data file - this must be called before using
  // other functions

  int ReadData(const char *FilePath);
  
  // Print the station data to standard output

  void PrintAll();
  
  // Find out the nearest airport name after getting the mouse click point 
  // If no station within max_distance, returns empty string

  string FindClosest(double getlat,
		     double getlon,
		     double max_distance = 100) const;
  
  // Given station name, fill out lat, lon, alt and type
  // Returns 0 on success, -1 on failure.

  int FindPosition(const string &name,
		   double &lat,
		   double &lon,
		   double &alt,
		   string &type);

  // Given a station name, fill out lat,lon,alt,type for all matches.
  // Return number of matches on success, -1 on failure.

  int FindAllPosition(const string &name,
		      vector <double> &lats,
		      vector <double> &lons,
		      vector <double> &alts,
		      vector <string> &types);


private:

  int ReadDataHttp(const char *url);

  vector<Info> _info;
  vector<Info>::iterator infoi;

};

#endif


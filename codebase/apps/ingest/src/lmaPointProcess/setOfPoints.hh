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
//
// Object that keeps a record of a set of points. NOTE : it does
// NOT store all the points - just the most recent. Also
// unix times are used, but they have a fractional part
// (to allow calculation of the duration). Niles Oien Jan 2004.
//

#ifndef setOfPoints_H
#define setOfPoints_H

#include "Params.hh"
#include <Spdb/DsSpdb.hh>
#include <vector>

using namespace std;

class setOfPoints {

public:

  //
  // Constructor.
  //
  setOfPoints(Params *TDRP_Params,
	      double lat,
	      double lon,
	      double alt,
	      double t);
  //
  // Method to add a point. Returns 0 if the point
  // was added, 1 if the point was not but it is possible that
  // another one could be (by looking at the time) and -1 if
  // the point was not added and in fact there is no way that
  // another point can be added at this time.
  //
  int addPoint(double lat,
	       double lon,
	       double alt,
	       double t);
  //
  // Save out the data for the set of points to an SPDB database.
  // Returns 0L or the time of the last write.
  //
  time_t addChunk(DsSpdb *Out, DsSpdb *OutLtg);

  //
  // Return the number of points.
  //
  int getSize();
  //
  // Return the duration.
  //
  double getDuration();
  //
  // Destructor
  //
  ~setOfPoints();

  private :

  //
  // The most recent point added, ie. the last.
  //  
  double _lastLat, _lastLon, _lastAlt;
  double _lastT;
  //
  // The first point added.
  //
  double _firstLat, _firstLon, _firstAlt;
  double _firstT;
  //
  int _numPoints;
  Params *_params;
  //
  // The altitudes.
  //
  vector <double> _altVector;
  //
  double _maxAlt, _minAlt;
  //
};

#endif


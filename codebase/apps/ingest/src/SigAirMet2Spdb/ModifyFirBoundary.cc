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
 // ModifyFirBoundary.cc
 //
 // SigAirMet2Spdb object -- functions for modifying FIR boundaries and
//                           for SIGMET polygons
 //
 // Deirdre Garvey, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
 //
 // Spring 2003
 //
 ///////////////////////////////////////////////////////////////

 #include <string>
 #include <vector>
 #include <cerrno>
 #include <toolsa/toolsa_macros.h>
 #include <toolsa/globals.h>
 #include <toolsa/pjg_flat.h>
 #include <euclid/geometry.h>
 #include "SigAirMet2Spdb.hh"

/////////////////////////////////////////////////
// search for FIR modifications
//

bool SigAirMet2Spdb::_searchForModifyFir
  (const size_t &startIdx,
   const size_t &endIdx,
   const vector <string> &toks,
   const vector <double> &firLats,
   const vector <double> &firLons,
   vector <double> &outlats,
   vector <double> &outlons)

{

  // set defaults

  outlats.clear();
  outlons.clear();

  bool found=false;
  string func_name="searchForModifyFir";

  // Search for things which can modify the FIR:
  //  - a line with 2+ stations or latlons as points and a
  //      direction relative to that line -- use _findPoints()
  //  - a lat and a direction relative to it, can be in combination with
  //      a lon and a direction, both can occur 1 or more times
  //      (e.g., 2 lats, and/or 2 lons) -- need new to generate
  //      these bounding lines
  //  - a portion of the FIR, e.g., 1/4 SE FIR would be the southeast quadrant
  //      of the FIR -- need new to generate these bounding lines
  
  // search for a LINE with stations or latlons as endpoints and a direction
  // e.g. N OF A LINE NZNR-OHURA
  

  // First check for 'AND'.  An 'AND' probably indicates that two lines are
  // being used to segment the FIR, and we will need to handle them both seperately.
  size_t andIdx=0;
  for (size_t ii = startIdx; ii < endIdx; ii++) {
    if (toks[ii] == "AND"){
      andIdx = ii;
      break;
    }
  }
  if (toks[andIdx] == "AND")
    {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Found 'AND' @ " << andIdx << endl;
      }

      if (_searchForModifyFirWith2PtsLineAndDir(startIdx, andIdx, toks,
                                              firLats, firLons, 
						outlats, outlons)){
	// succesfully modified the FIR one time, try again
	vector <double> newLats = outlats;
	vector <double> newLons = outlons;
	if (_searchForModifyFirWith2PtsLineAndDir(andIdx, endIdx, toks,
                                              newLats, newLons, 
						  outlats, outlons))
	  //succesfully handled both lines
	  return true;
	
	// put the output from first handling back into outlats
	outlats = newLats;
	outlons = newLons;	
	return true;
      }
    }
   


  found=_searchForModifyFirWith2PtsLineAndDir(startIdx, endIdx, toks,
                                              firLats, firLons, 
                                              outlats, outlons);
  
  if (found) {
    return true;
  }
  
  // search for a direction and a lat or lon line
  // e.g., S OF N04 AND E OF E10430

  found=_searchForModifyFirWithLatLonLineAndDir(startIdx, endIdx, toks,
                                                firLats, firLons, 
						outlats, outlons);

  if (found) {
    return true;
  }
  
  // not successful

  return false;

}

/////////////////////////////////////////////////
// Search for a line with 2+ stations or latlons
// as points and a direction relative to that line
//
// Return true if able to modify FIR, false otherwise
//
// Load the new boundaries in outlats, outlons

bool SigAirMet2Spdb::_searchForModifyFirWith2PtsLineAndDir
  (const size_t &startIdx,
   const size_t &endIdx,
   const vector <string> &toks,
   const vector <double> &firLats,
   const vector <double> &firLons,
   vector <double> &outlats,
   vector <double> &outlons)

{

  // set defaults

  outlats.clear();
  outlons.clear();

  string func_name="searchForModifyFirWith2PtsLineAndDir";
  bool found=false;

  // search for a LINE with stations or latlons as endpoints and a direction
  // e.g. N OF A LINE NZNR-OHURA

  bool foundLine=false;
  bool foundDir=false;
  string dir;
  for (size_t ii=startIdx; ii< endIdx; ii++) {
    string tok=toks[ii];
    if (_isDirTok(tok)) {
      foundDir=true;
      dir=tok;
    }
    if (_isLineTok(tok)) {
      foundLine=true;
      break;
    }
  }

  if (!foundDir || !foundLine) {
    found=false;
  }

  // Find the line endpoints

  if (foundLine && foundDir) {
    vector <bool> sources;
    vector <double> adjustLats;
    vector <double> adjustLons;
    bool skipIcaoStation=true;
    int minNpoints=2;
    size_t usedStartIdx=0;
    size_t usedEndIdx=0;
    int nPoints=_findPoints(toks, startIdx, endIdx, skipIcaoStation, 
			    minNpoints, adjustLats, adjustLons, sources,
			    usedStartIdx, usedEndIdx);

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << func_name << ": dir: " << dir 
           << ", foundLine: " << foundLine 
           << ", nPoints: " << nPoints << endl;
    }
    
    if (nPoints < minNpoints) {

      found=false;

    } else {
      
      // Set a new centroid based on the FIR boundaries
      
      _setCentroidFromFir();

      // Convert the direction to an angle

      double angle;
      if (!_dir2ReverseAngle(dir, angle)) {
	return false;
      }

      // Construct the great circle call. Use only the first and last endpoints
      // of the line (in case there are intermediate points)
      
      vector <double> tmplats=firLats;
      vector <double> tmplons=firLons;
      GC_PolyTrunc gc(tmplats, tmplons);
      double useLat1, useLon1, useLat2, useLon2;
      useLat1=adjustLats[0];
      useLon1=adjustLons[0];
      useLat2=adjustLats[adjustLats.size() - 1];
      useLon2=adjustLons[adjustLons.size() - 1];

      if (_params.debug) {
	cerr << func_name << ": current nPoints for gc: " << gc.getNpoints() 
             << endl;
	gc.print();
	cerr << func_name << ": calling GreatCircleTrunc with: " 
             << useLat1 << ", " << useLon1 << ", " <<  useLat2 
             << ", " << useLon2 << ", dir: " << angle << endl;
      }

      gc.GreatCircleTrunc(useLat1, useLon1, useLat2, useLon2, angle);
      
      if (_params.debug) {
	cerr << func_name << ": after GreatCircleTrunc, nPoints: " 
             << gc.getNpoints() << endl;
	gc.print();
      }

      // Create the return lat,lon vectors

      size_t npts=_buildLatLonVectorsFromGCPolyTrunc(gc, outlats, outlons);

      if (npts < 3) {
	found=false;
      } else {
	found=true;
      }

    } //endelse nPoints < minNpoints
  } //endif foundline and foundDir

  return found;
}


/////////////////////////////////////////////////
// Search for a lat and a direction relative to it.
// Can be in combination with a lon and a direction.
// Both can occur 1 or more times (e.g., 2 lats, and/or 2 lons)
//   -- need new to generate these bounding lines
//
// Return true if able to modify FIR, false otherwise
//
// Load the new boundaries in outlats, outlons

bool SigAirMet2Spdb::_searchForModifyFirWithLatLonLineAndDir
  (const size_t &startIdx,
   const size_t &endIdx,
   const vector <string> &toks,
   const vector <double> &firLats,
   const vector <double> &firLons,
   vector <double> &outlats,
   vector <double> &outlons)

{

  // set defaults

  outlats.clear();
  outlons.clear();

  bool found=false;
  string func_name="searchForModifyFirWithLatLonLineAndDir";

  // search for a direction and a lat or lon line
  // e.g., S OF N04 AND E OF E10430

  bool foundOne=true;
  size_t useStart=startIdx;
  size_t useEnd=endIdx;
  size_t usedStart=0;
  size_t usedEnd=0;
  // set the initial FIR boundary to the entire FIR
  vector <double> useFirLats;
  vector <double> useFirLons;
  useFirLats.clear();
  useFirLons.clear();
  useFirLats=firLats;
  useFirLons=firLons;
  string dir;
  GC_PolyTrunc gc(useFirLats, useFirLons);

  while (foundOne) {
    bool isLat;
    double value;
    foundOne=_searchForDirAndLatOrLon(useStart, useEnd, toks, dir, isLat, 
				      value, usedStart, usedEnd);

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << func_name << ": found a dir and lat or lon: "
           << foundOne << ", dir: " << dir
           << ", isLat: " << isLat << ", value: " << value 
           << ", useStart: " << useStart << ", useEnd: " << useEnd << endl;
    }

    if (foundOne) {

      // reset the next start to use the end of the 
      // previously found direction +lat/lon

      useStart=usedEnd;
	
      // Set a new centroid based on the FIR boundaries

      _setCentroidFromFir();

      // Convert the direction to an angle

      double angle;
      if (!_dir2ReverseAngle(dir, angle)) {
	return false;
      }

      // Get the min/max lat and lon

      double minLat, maxLat, minLon, maxLon;
      if (!_getMinMaxLatLon(firLats, firLons,
			    minLat, maxLat, minLon, maxLon)) {
	return false;
      }

      // Construct the great circle call

      double useLat1, useLon1, useLat2, useLon2;
      if (isLat) {
	useLat1=value;
	useLat2=value;
	useLon1=minLon;
	useLon2=maxLon;
      } else {
	useLat1=minLat;
	useLat2=maxLat;
	useLon1=value;
	useLon2=value;
      }

      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << func_name << ": current nPoints for gc: " 
             << gc.getNpoints() << endl;
	gc.print();
	cerr << func_name << ": calling GreatCircleTrunc with: " 
             << useLat1 << ", " << useLon1 << ", " <<  useLat2 
             << ", " << useLon2 << ", dir: " << angle << endl;
      }
      
      gc.GreatCircleTrunc(useLat1, useLon1, useLat2, useLon2, angle);

      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << func_name << ": after GreatCircleTrunc, nPoints: " 
             << gc.getNpoints() << endl;
	gc.print();
      }

      if (gc.getNpoints() <= 3) {
	foundOne=false;
	break;
      }
      found=true;
    } // endif foundOne
  } // endwhile

  if (found) {
    // assign the useFirLats,Lons to the return outlats,outlons
    size_t npts=_buildLatLonVectorsFromGCPolyTrunc(gc, outlats, outlons);
    if (_params.debug) {
      cerr << func_name << ": found npts: " << npts << endl;
    }
    if (npts < 3) {
      found=false;
    }
  }

  return found;
}


/////////////////////////////////////////////////
// Build new polygon lat,lon vectors from GC_PolyTruc
//
// Returns the number of points
// Returns the new firlats and firlons vectors based on the
// contents of the GC_PolyTrunc item

size_t SigAirMet2Spdb::_buildLatLonVectorsFromGCPolyTrunc
  (GC_PolyTrunc &gc,
   vector <double> &lats,
   vector <double> &lons)
{
  size_t nPoints = gc.getNpoints();
  for (size_t ii=0; ii < nPoints; ii++) {
    lats.push_back(gc.getLat(ii));
    lons.push_back(gc.getLon(ii));
  }
  return nPoints;
}


/////////////////////////////////////////////////
// search for a direction then a lat or lon line
//
// Returns true if found a direction and lat or lon line
// Returns the direction and whether is a lat or line line
// and the value (lat or lon)

bool SigAirMet2Spdb::_searchForDirAndLatOrLon
  (const size_t &startIdx,
   const size_t &endIdx,
   const vector <string> &toks,
   string &dir,
   bool &isLat,
   double &value,
   size_t &usedStartIdx,
   size_t &usedEndIdx)
{
 
  // Set return defaults

  isLat=false;
  value=-9999;
  dir="NULL";

  // Set defaults

  string func_name="searchForDirAndLatOrLon";

  // Search for a direction then the lat or lon

  bool foundDir=false;
  size_t dirIdx=0;
  for (size_t ii=startIdx; ii< endIdx; ii++) {
    string tok=toks[ii];
    if (_isDirTok(tok)) {
      if ((ii+1 < endIdx) && (toks[ii+1] == "OF")) {
	foundDir=true;
	dir=tok;
	dirIdx=ii;
	break;
      }
    }
  }

  if (!foundDir) {
    return false;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << func_name << ": Found a direction: " << dir << endl;
  }

  // set defaults

  usedStartIdx=dirIdx;

  size_t useStartIdx=dirIdx+2;
  string latlonstr="";
  int max_search_toks=1;
  bool foundNS=false;
  bool foundEW=false;
  size_t nsPos;
  size_t ewPos;

  // Look for a lat or lon, expect in the form N/S/E/W plus a value
  // Are we near the end of the input vector

  if ((useStartIdx + max_search_toks) > toks.size()) {
    max_search_toks=toks.size() - useStartIdx;
  }
  if (max_search_toks < 1) {
    return false;
  }
  
  // Look for the start of a lat or lon in the first token

  string useTok=toks[useStartIdx];
  if (_findNSEWchars(useTok, foundNS, nsPos, foundEW, ewPos)) {
    latlonstr=useTok;
    usedEndIdx=useStartIdx;
  }

  // Is the first token a number and the second token a N/S/E/W?

  if (!foundNS && !foundEW && (_hasDigit(useTok))) {
    if (_findNSEWchars(toks[useStartIdx+1], foundNS, nsPos, foundEW, ewPos)) {
      latlonstr=useTok;
      latlonstr+=toks[useStartIdx+1];
      usedEndIdx=useStartIdx+1;
    }
  }
    
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << func_name << ": latlonstr: " << latlonstr 
         << ", foundNS: " << foundNS << ", foundEW: " << foundEW << endl;
  }

  if (!foundNS && !foundEW) {
    return false;
  }

  // Okay, we have a possible lat or lon string. 
  // Is it a string that happens to have N or S in it?

  if (!_isDirTok(latlonstr)) {
    return false;
  }

  // Parse for lat or lon
  
  bool found=_hasLatOrLon(latlonstr, isLat, value);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << func_name << ": found: " << found 
         << ", isLat: " << isLat << ", value: " << value 
         << ", dir: " << dir << ", usedStartIdx: " << usedStartIdx 
         << ", usedEndIdx: " << usedEndIdx << endl;
  }

  return found;

}

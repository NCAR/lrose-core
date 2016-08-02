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


#include "setOfPointsMgr.hh"
#include <toolsa/umisc.h>
#include <Spdb/DsSpdb.hh>
#include <rapformats/GenPt.hh>

//
// Constructor. Reads the stations and pushes
// them back into the correct vector.
//
setOfPointsMgr::setOfPointsMgr(Params *TDRP_params){
  _params = TDRP_params;
  _setsOfPoints.clear();
  _lastPrintT=0.0;
  _lastWriteT=0;
  Out.setPutMode(Spdb::putModeAdd);
  OutLtg.setPutMode(Spdb::putModeAdd);
}
//
//
// Method to add a point. Determines if we are done and if
// we should save.
//
void setOfPointsMgr::addPoint(double lat,
			      double lon,
			      double alt,
			      double t){
  //
  // Do some debugging, if needed.
  //
  if (_params->debug){
    if (t-_lastPrintT > 5){
      cerr << "At " << utimstr((time_t)t) << " ";
      cerr  << _setsOfPoints.size() << " bundles are active." << endl;
      _lastPrintT = t;
    }
  }
  //
  // See if we can add this to an existing string.
  //
  for (unsigned i=0; i < _setsOfPoints.size(); i++){
    //
    int retVal = _setsOfPoints[i]->addPoint(lat,lon, alt,t);
    //
    if (retVal == 0) return; // Added point to an existing set. Done.
    //
    if (retVal == -1){
      //
      // This one is done - take it off the list after getting a printout.
      //
      time_t temp = _setsOfPoints[i]->addChunk(&Out, &OutLtg);
      if (temp >  _lastWriteT) _lastWriteT = temp;
      delete _setsOfPoints[i];
      _setsOfPoints.erase( _setsOfPoints.begin() + i);
    }
  }
  //
  // OK - if we got to here, we did not
  // add this point to an existing bundle.
  // Create a new bundle.
  //
  setOfPoints *S = new setOfPoints(_params, lat, lon, alt, t);
  //
  _setsOfPoints.push_back( S );
  //
  return;
  //
}
//
// Destructor. Cleans out the vector.
//
setOfPointsMgr::~setOfPointsMgr(){

  for (unsigned i=0; i < _setsOfPoints.size(); i++){
    delete _setsOfPoints[i];
  }
  _setsOfPoints.clear();

}
//
// Method to write out a dummy point.
//
void setOfPointsMgr::writeDummy(){
  //
  //
  // Only do this in REALTIME mode.
  //
  if (_params->mode != Params::REALTIME) return;
  //
  // Only do this if ten seconds has elapsed since the last save.
  //
  if ( time(NULL) - _lastWriteT < 10) return;
  //
  // Save this out as a GenPoint. Set the number of LMA points
  // to 0 so it's clearly bogus.
  //
  GenPt G;
  G.setName("Dummy ltg point");
  G.setId( 0 );

  time_t saveTime = time(NULL);
  int dataType = 123;
  G.setTime( saveTime );
  G.setLat( -90.0 );
  G.setLon( 180.0 );
 
  G.setNLevels( 1 );
  //
  G.addVal( double(0) );
  G.addFieldInfo( "numLmaEntries", "count");
  //
  G.addVal( -90.0 );
  G.addFieldInfo( "firstLat", "deg");
  G.addVal( 180.0 );
  G.addFieldInfo( "firstLon", "deg");
  G.addVal( saveTime );
  G.addFieldInfo( "firstT", "sec");
  G.addVal( 0.0 );
  G.addFieldInfo( "firstAlt", "m");
  //
  G.addVal( -90.0 );
  G.addFieldInfo( "lastLat", "deg");
  G.addVal( 180.0 );
  G.addFieldInfo( "lastLon", "deg");
  G.addVal( saveTime );
  G.addFieldInfo( "lastT", "sec");
  G.addVal( 0.0 );
  G.addFieldInfo( "lastAlt", "m");
  //
  G.addVal( 0.0 );
  G.addFieldInfo( "minAltitude", "m");
  G.addVal( 0.0 );
  G.addFieldInfo( "maxAltitude", "m");
  G.addVal( 0.0 );
  G.addFieldInfo( "meanAltitude", "m");
  G.addVal( 0.0 );
  G.addFieldInfo( "sdAltitude", "m");
  //
  G.addVal( 0.0 );
  G.addFieldInfo( "duration", "sec");
  //  
  G.addVal( 0.0 );
  G.addFieldInfo( "horDist", "Km");
  //
  G.addVal( 0.0 );
  G.addFieldInfo( "flashLength", "Km");
  //
  if (!(G.check())){
    fprintf(stderr, "GenPt check failed.\n");
    exit(-1);
  }
  
  if ( G.assemble() != 0 ) {
    fprintf(stderr, "Failed to assemble GenPt.\n");
    exit(-1);
  }
  //
  // Write the point to the database
  //
  Out.addPutChunk( 0, G.getTime(),
		   G.getTime() + _params->Expiry,
		   G.getBufLen(), G.getBufPtr() );
  //
  _lastWriteT = saveTime;
  return;
  //
}

bool setOfPointsMgr::writeChunks()
{
  if (Out.put(_params->output_url, 
	      SPDB_GENERIC_POINT_ID, 
	      SPDB_GENERIC_POINT_LABEL) != 0) 
  {
    cerr << "Error writing to spdb database." << endl;
  }

  Out.clearPutChunks();

  if (_params->saveTraditional)
  {
    OutLtg.put( _params->traditional_ltg_url,
		SPDB_LTG_ID,
		SPDB_LTG_LABEL);
    OutLtg.clearPutChunks();
  }

}

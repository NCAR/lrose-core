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
// Class that reads the model metars for a given station
// that has had an actual metar come in, and interpolates
// the model data into a METAR at the same time as the
// current metar so that the correction factors can be
// based on this metar (returned in BaseModelMetar).
//
//
#include <list>
#include <string>
//
#include <vector>
//
#include <Spdb/DsSpdb.hh>
#include <rapformats/station_reports.h> 
#include <physics/thermo.h>
#include <physics/physics.h>
#include <toolsa/udatetime.h>
//
#include "ModelMetars.hh"
#include "Params.hh"
using namespace std;
//
// File scope. The routine that interpolates model metars to
// the time of the actual metar.
//
int InterpolateMetar(ModelMetars::entry_t Start, 
		      ModelMetars::entry_t End, 
		      time_t CurrentMetarTime,
		      Params *P, 
		      station_report_t *BaseModelMetar);
//
// Minor subroutine.
//
fl32 DoInterp(fl32 StartVal, fl32 EndVal, double t);
//
///////////////////////////////////////////////////////////
//
// Constructor makes a copy of the TDRP parameters.
//
ModelMetars::ModelMetars( const Params *params){
  _params = (Params *) params;
}

//////////////////////////////////////////////////////////
//
// Main method - get list of entries.
// Returns 0 if OK, -2 if found no data
//
int ModelMetars::getNextTimes(int dataType, // From the four character station ID, hashed.
			      time_t CurrentMetarTime, // Time of the current metar.
			      list<ModelMetars::entry_t> &UnprocessedEntries){

  //
  // Get all the data from the input in this interval
  // for this station.
  //
  if (_params->Debug){
    cerr << "Getting model spdb data from " << _params->ModelUrl << endl;
    cerr << "between " << utimstr(CurrentMetarTime) << endl;;
    cerr << "and " << utimstr(CurrentMetarTime + 3600*_params->MaxLeadTime) << endl;
    cerr << "for station " << Spdb::dehashInt32To4Chars(dataType) << endl;
  }


  DsSpdb ModelSpdb;
  if (ModelSpdb.getInterval(_params->ModelUrl,
			    CurrentMetarTime,
			    CurrentMetarTime + 3600*_params->MaxLeadTime,
			    dataType)) {
    cerr << "Failed to get spdb data from " << _params->ModelUrl;
    cerr << " between " << utimstr(CurrentMetarTime);
    cerr << " and " << utimstr(CurrentMetarTime + 3600*_params->MaxLeadTime) << endl;
    cerr << ModelSpdb.getErrorStr() << endl;
    return -1;
  }
  //
  // First, put all the model METARS into the
  // UnprocessedEntries list.
  //
  const vector<Spdb::chunk_t> &chunks = ModelSpdb.getChunks();

  if (_params->Debug){
    cerr << ModelSpdb.getNChunks() << " input model chunks found at " << _params->ModelUrl << endl;
  }
  //
  // Return -2 if we found nothing.
  //
  if (ModelSpdb.getNChunks() < 2) return -2; // We need at least two to go on.
  //
  // Useful at this point to make a note
  // of the min and max times.
  //   
  int first = 1;
  time_t minTime=0, maxTime=0; // Set to 0 to avoid spurious compiler warnings.

  //
  // Add all entries to UnprocessedEntries unless the valid time is before CurrentMetarTime
  //
  for (int ichunk = 0; ichunk < ModelSpdb.getNChunks(); ichunk++){   
    
    if (chunks[ichunk].valid_time < CurrentMetarTime){
      continue;
    }

    ModelMetars::entry_t E;
    E.dataTime = chunks[ichunk].valid_time;
    E.dataType = chunks[ichunk].data_type;
    E.dataType2= chunks[ichunk].data_type2;
     

    UnprocessedEntries.push_back( E );

    if (first){
      first = 0;
      minTime = chunks[ichunk].valid_time;
      maxTime = minTime;
    } else {
      if (maxTime < chunks[ichunk].valid_time) maxTime = chunks[ichunk].valid_time;
      if (minTime > chunks[ichunk].valid_time) minTime = chunks[ichunk].valid_time;
    }

  }
  //
  if (_params->Debug){
    cerr << "Got model spdb data from " << _params->ModelUrl << endl;
    cerr << "  between "
	 << utimstr(CurrentMetarTime) << endl;
    cerr << "  and "
	 << utimstr(CurrentMetarTime + 3600*_params->MaxLeadTime) << endl;
    cerr << "  for station " << Spdb::dehashInt32To4Chars(dataType) << endl;
    cerr << "Model data times range : " << utimstr(minTime);
    cerr << "  to " << utimstr(maxTime) << endl;
    cerr << "Number of entries : " << ModelSpdb.getNChunks() << endl;

    if (_params->Debug) {
      int q=0;
      list<ModelMetars::entry_t>::iterator l;
      for( l=UnprocessedEntries.begin(); l != UnprocessedEntries.end(); l++ ) {
	q++;
	cerr << q << " " << utimstr(l->dataTime)
	     << " " << l->dataType2 << endl;
      }
    }
  }


  //
  // Get rid of duplicate entries at the same time - take the
  // one with the least data type 2 (lead time).
  //


  list<ModelMetars::entry_t>::iterator i,j; 
  
  int NumLeft = UnprocessedEntries.size();
  int ProcNum = 0;

  for( i=UnprocessedEntries.begin(); i != UnprocessedEntries.end(); i++ ) {
    //
    // Delete forecast if it is initialized after the actual METAR time. (playback)
    //
    if ((i->dataTime - i->dataType2) > CurrentMetarTime){
      i = UnprocessedEntries.erase(i);
      i--;
      if (i == UnprocessedEntries.end()) 
	break;
    }
  }


  do{
    int deletions = 1;
    do {
      
      if (UnprocessedEntries.size() < 2) break; // Only one entry, do not process.
      
      i=UnprocessedEntries.begin();

      for(int k=0; k<ProcNum; k++){
	i++;
      }

      deletions = 0;

      for(j=i, j++; j != UnprocessedEntries.end(); j++){

	if (i->dataTime == j->dataTime){ 
	  //
	  // Duplicate entry, delete the one with
	  // the greater lead time.
	  //
	  if (j->dataType2 <= i->dataType2){
	    i=UnprocessedEntries.erase(i);
	  } else {
	    j=UnprocessedEntries.erase(j);
	  }
	  deletions=1;
	  NumLeft--;
	  break; // Discontinue loop.
	}      
	
      }
      
      
    }while(deletions);

    ProcNum++;

  }while(ProcNum < NumLeft-1);

  // End of new code.

  if (_params->Debug){
    cerr << "After removing duplicate entries, " << UnprocessedEntries.size();
    cerr << " entries remain." << endl;
    
    if (_params->Debug) {
      int q=0;
      list<ModelMetars::entry_t>::iterator l;
      
      for( l=UnprocessedEntries.begin(); l != UnprocessedEntries.end(); l++ ) {
	q++;
	cerr << q << " " << utimstr(l->dataTime)
	     << " " << l->dataType2 << endl;
      }
    }

  }


  //
  // Delete all entries for which the output already exists.
  //
  //
  for( i=UnprocessedEntries.begin(); i != UnprocessedEntries.end(); i++ ) {
    //
    // Delete it if there is already an entry in the output for it.
    //
    int LeadTime = i->dataTime - CurrentMetarTime;
    DsSpdb OutputSpdb;
    OutputSpdb.getExact(_params->OutUrl,
			i->dataTime,
			i->dataType,
			LeadTime);
    
    if (OutputSpdb.getNChunks() != 0){ 
      // Already in the output, delete it from the list we have to process.
      i = UnprocessedEntries.erase(i);
      i--;
      if (i == UnprocessedEntries.end())
	break;
    }
  }

  return 0;
}


/////////////////////////////////////////////////////////
//
// Destructor
//
ModelMetars::~ModelMetars(){

}

/////////////////////////////////////////////////////////
//
//
//
int InterpolateMetar(ModelMetars::entry_t Start, 
		      ModelMetars::entry_t End, 
		      time_t CurrentMetarTime,
		      Params *P, 
		      station_report_t *BaseModelMetar){
  //
  // Read in the two model metars.
  //
  DsSpdb StartSpdb, EndSpdb;
  StartSpdb.getExact(P->ModelUrl,
		     Start.dataTime,
		     Start.dataType,
		     Start.dataType2);
  EndSpdb.getExact(P->ModelUrl,
		   End.dataTime,
		   End.dataType,
		   End.dataType2);
  //
  // Check that the read went OK. It should have, because
  // we checked that there was something in the model
  // database at these times.
  //
  if (
      (StartSpdb.getNChunks() == 0) ||
      (EndSpdb.getNChunks() == 0)
      ){ 
    if (StartSpdb.getNChunks() == 0) cerr << "Start failed." << endl;
    if (EndSpdb.getNChunks() == 0) cerr << "End failed." << endl;

    cerr << "Failed to read model data from " << P->ModelUrl << endl;
    cerr << "Start : " << endl;
    cerr << utimstr(Start.dataTime) << endl;
    cerr << Spdb::dehashInt32To4Chars( Start.dataType ) << endl;
    cerr << Start.dataType2 << endl << endl;

    cerr << "End : " << endl;
    cerr << utimstr(End.dataTime) << endl;
    cerr << Spdb::dehashInt32To4Chars( End.dataType ) << endl;
    cerr << End.dataType2 << endl << endl;

	cerr << "Skipping this metar." << endl;

    return(-1);
  }
  //
  // Unfold these into station report structs.
  //
  station_report_t *StartReport, *EndReport;
  StartReport = (station_report_t *) StartSpdb.getChunks()[0].data;
  station_report_from_be( StartReport );

  EndReport = (station_report_t *) EndSpdb.getChunks()[0].data;
  station_report_from_be( EndReport );
  //
  // Get the linear interpolation fraction.
  //
  double t = (CurrentMetarTime - Start.dataTime) / (End.dataTime - Start.dataTime);
  if (P->Debug){
    cerr << "Linear interpolation fraction is " << t << endl;
  }
  //
  // Do simple interpolation on the straightforward fields.
  //
  BaseModelMetar->pres = DoInterp(StartReport->pres,EndReport->pres,t);
  BaseModelMetar->temp = DoInterp(StartReport->temp,EndReport->temp,t);
  BaseModelMetar->relhum = DoInterp(StartReport->relhum,EndReport->relhum,t);
  BaseModelMetar->ceiling = DoInterp(StartReport->ceiling,EndReport->ceiling,t);
  BaseModelMetar->visibility = DoInterp(StartReport->visibility,EndReport->visibility,t);
  //
  // Get the dew point from physics rather than interpolation for
  // internal consistency.
  //
  BaseModelMetar->dew_point = PHYrhdp(BaseModelMetar->temp,
				      BaseModelMetar->relhum);
  //
  // Get the wind dir by doing the interpolation in U and V and then
  // going back to direction.
  //
  if (
      (StartReport->windspd == STATION_NAN) ||
      (EndReport->windspd == STATION_NAN) ||
      (StartReport->winddir == STATION_NAN) ||
      (EndReport->winddir == STATION_NAN) 
      ){
    BaseModelMetar->winddir = STATION_NAN;
    BaseModelMetar->windspd = STATION_NAN;
  } else {
    fl32 StartU, StartV, EndU, EndV, U,V;
    StartU =  PHYwind_u(StartReport->windspd, StartReport->winddir);
    StartV =  PHYwind_v(StartReport->windspd, StartReport->winddir);
    EndU =    PHYwind_u(EndReport->windspd, EndReport->winddir);
    EndV =    PHYwind_v(EndReport->windspd, EndReport->winddir);    
    //
    // Interpolate in U and V.
    //
    U = (1.0-t)*StartU + t*EndU;
    V = (1.0-t)*StartV + t*EndV;
    //
    // Go back to speed and direction.
    //
    BaseModelMetar->winddir = PHYwind_dir(U,V);
    BaseModelMetar->windspd = PHYwind_speed(U,V);
    BaseModelMetar->time = CurrentMetarTime;
  }

  return 0;

}
////////////////////////////////////////////////////////////
//
// Small routine to do an interpolation, checking to see if the
// values are STATION_NAN first and returning appropriately. 
//
fl32 DoInterp(fl32 StartVal, fl32 EndVal, double t){
	
  if (
      (StartVal == STATION_NAN) ||
      (EndVal   == STATION_NAN)
      ){
    return STATION_NAN;
  }

  return (1.0-t)*StartVal + t*EndVal;

}









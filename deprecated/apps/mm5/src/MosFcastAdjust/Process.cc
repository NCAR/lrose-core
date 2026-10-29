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

#include "Process.hh"
#include "ModelMetars.hh"

#include <Spdb/DsSpdb.hh>
#include <rapformats/station_reports.h>
#include <physics/physics.h>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <dsserver/DmapAccess.hh>
 
#include <cstdlib>  
#include <cstdio>
#include <math.h>
//
// File scope - the routine that calculates the
// correction offsets/co-efficients and stores them
// in the METAR struct, CorrectionMetar.
//

void CorrectReport(const Params *P, 
		   const station_report_t &CorrectionMetar, 
		   station_report_t &ModelReport);


fl32 ApplyAdjustment(time_t tDiff, fl32 Model, fl32 Corrector,
		     int AdjustSec, Params::adjust_t Adjust);

void Process(Params *P, list<SpdbTrigger::entry_t> &UnprocessedEntries){

  DsSpdb MetarSpdb;

  list<SpdbTrigger::entry_t>::iterator i;
  for( i=UnprocessedEntries.begin(); i != UnprocessedEntries.end(); i++ ) {

    PMU_auto_register("Processing entries");

    //
    // Get the SPDB station report data
    //

    if (P->Debug){
      cerr << endl;
      cerr << "Getting data at " << utimstr(i->dataTime);
      cerr << " types " << i->dataType << " " << i->dataType2;
      cerr << " from " << P->TriggerUrl;
      cerr << " for station " << Spdb::dehashInt32To4Chars( i->dataType ) << "..."; 
    }
    MetarSpdb.getExact(P->TriggerUrl,
		    i->dataTime,
		    i->dataType,
		    i->dataType2);

    vector<Spdb::chunk_t> mchunks = MetarSpdb.getChunks();
    if (mchunks.size() < 1) {
      cerr << "ERROR - MosFcastAdjust::Process" << endl;
      cerr << "  No chunks found" << endl;
      cerr << "  URL " << P->TriggerUrl << endl;
      continue;
    }
    if (mchunks[0].len < sizeof(station_report_t)) {
      cerr << "ERROR - MosFcastAdjust::Process" << endl;
      cerr << "  URL " << P->TriggerUrl << endl;
      cerr << "  Chunk not large enough for station_report" << endl;
      cerr << "  Chunk len: " << mchunks[0].len << endl;
      continue;
    }
    if (P->Debug) cerr << " done." << endl;

    //
    // Disassemble it into a station report.
    //
    station_report_t metarReport;
    memcpy(&metarReport, mchunks[0].data, sizeof(station_report_t));
    station_report_from_be(&metarReport);

    if (P->Debug) {
	  cerr << "================================" << endl;
	  cerr << "Metar report:" << endl;
	  print_station_report(stderr, "", &metarReport);
	  cerr << "================================" << endl;
	}

    //
    // Get the model entries.
    //
    list<ModelMetars::entry_t> UnprocessedModelEntries;

    ModelMetars MM( P );
    

    int ModelSearch = MM.getNextTimes(i->dataType,
				      i->dataTime,
				      UnprocessedModelEntries);
    if (ModelSearch == -2){
      if (P->Debug) cerr << "No model data found." << endl;
      continue;
    }
    if (ModelSearch != 0){
      if (P->Debug) cerr << "Model get next times failed." << endl;
      continue;
    }
 
  
    //
    // Loop through model data applying the corrections and
    // writing the data out.
    //

    DsSpdb Output;
    Output.clearPutChunks();
    Output.setLeadTimeStorage(Spdb::LEAD_TIME_IN_DATA_TYPE2);

    list<ModelMetars::entry_t>::iterator j;
    for( j=UnprocessedModelEntries.begin();
	 j != UnprocessedModelEntries.end(); j++ ) {

      DsSpdb ModelSpdb;
      if (ModelSpdb.getExact(P->ModelUrl,
			     j->dataTime,
			     j->dataType,
			     j->dataType2)) {
	cerr << "ERROR - MosFcastAdjust::Process" << endl;
	cerr << "  " << ModelSpdb.getErrStr() << endl;
      }
      
      vector<Spdb::chunk_t> chunks = ModelSpdb.getChunks();
      if (chunks.size() < 1) {
	cerr << "ERROR - MosFcastAdjust::Process" << endl;
	cerr << "  No chunks found" << endl;
	continue;
      }
      if (chunks[0].len < sizeof(station_report_t)) {
	cerr << "ERROR - MosFcastAdjust::Process" << endl;
	cerr << "  Chunk not large enough for station_report" << endl;
	cerr << "  Chunk len: " << chunks[0].len << endl;
	continue;
      }
      
      // copy in report and byte swap
      station_report_t correctedReport;
      memcpy(&correctedReport, chunks[0].data, sizeof(station_report_t));
      station_report_from_be( &correctedReport );
      correctedReport.msg_id = SENSOR_REPORT;


      CorrectReport(P, metarReport, correctedReport);
      
      int LeadTime = correctedReport.time - metarReport.time;

      if (LeadTime >= 0){ // Only write this out if it has a positive lead time.
	//
	// Niles : Added this next bit. Work out the model run
	// time and put it in a user specific place in the output
	// struct.
	//
	int ModelRunTime = correctedReport.time - j->dataType2;
	correctedReport.shared.user_data.int1 = ModelRunTime;
	correctedReport.accum_start_time = ModelRunTime;

	if (P->Debug){
	  cerr << endl;
	  cerr << "Model valid time : "
	       << utimstr(correctedReport.time) << endl;
	  cerr << "Lead time, sec. : " << j->dataType2 << endl;
	  cerr << "Model run time : "
	       << utimstr(correctedReport.shared.user_data.int1)
	       << endl << endl;
	}

	//
	// Niles - added this check so we would not get one
	// wind field without the other.
	//
	if (correctedReport.winddir == STATION_NAN) 
	  correctedReport.windspd = STATION_NAN;
	//
	if (correctedReport.windspd == STATION_NAN)
	  correctedReport.winddir = STATION_NAN;
	//

	if (P->Debug) {
	  cerr << "================================" << endl;
	  cerr << "Adjusted report:" << endl;
	  print_station_report(stderr, "", &correctedReport);
	}
	
	station_report_to_be( &correctedReport ); 

	Output.addPutChunk(j->dataType,
			   j->dataTime,
			   j->dataTime + P->TimeBeforeExpire,
			   sizeof( station_report_t ),
			   &correctedReport,
			   LeadTime);
	
      }

    }

    Output.put(P->OutUrl,
	       SPDB_STATION_REPORT_ID,
	       SPDB_STATION_REPORT_LABEL);
    
  } // End of loop through input metar data.


}
///////////////////////////////////////////////////////
//
// Routine that actually does the correction.
//
void CorrectReport(const Params *P, 
		   const station_report_t &CorrectionMetar, 
		   station_report_t &ModelReport){



  if ((ModelReport.temp != STATION_NAN) && (CorrectionMetar.temp != STATION_NAN)){
      ModelReport.temp = ApplyAdjustment(ModelReport.time - CorrectionMetar.time,
					  ModelReport.temp, 
					  CorrectionMetar.temp,
					  P->TempAdjustSec,
					  P->AdjustmentType);
  }

  if ((ModelReport.pres != STATION_NAN) && (CorrectionMetar.pres != STATION_NAN)){
      ModelReport.pres = ApplyAdjustment(ModelReport.time - CorrectionMetar.time,
					  ModelReport.pres, 
					  CorrectionMetar.pres,
					  P->PressureAdjustSec,
					  P->AdjustmentType);
  }

  //if ((!(ModelReport.ceiling == STATION_NAN)) && (!(ModelReport.ceiling == P->ClearCeiling))) {
  if ((ModelReport.ceiling != STATION_NAN) && (CorrectionMetar.ceiling != STATION_NAN)){
      if (CorrectionMetar.ceiling > P->ClearCeiling){
        ModelReport.ceiling = ApplyAdjustment(ModelReport.time - CorrectionMetar.time,
					     ModelReport.ceiling, 
					     P->ClearCeiling,
					     P->CeilingAdjustSec,
					     P->AdjustmentType);
  } else {
      ModelReport.ceiling = ApplyAdjustment(ModelReport.time - CorrectionMetar.time,
					     ModelReport.ceiling, 
					     CorrectionMetar.ceiling,
					     P->CeilingAdjustSec,
					     P->AdjustmentType);
    }
  }

  //if ((!(ModelReport.visibility == STATION_NAN)) && (!(ModelReport.visibility == P->ClearVisibility))) {
  if ((ModelReport.visibility != STATION_NAN) && (CorrectionMetar.visibility != STATION_NAN)){
      ModelReport.visibility = ApplyAdjustment(ModelReport.time - CorrectionMetar.time,
					        ModelReport.visibility, 
					        CorrectionMetar.visibility,
					        P->VisibilityAdjustSec,
					        P->AdjustmentType);
  }

  if ((ModelReport.relhum != STATION_NAN) && (CorrectionMetar.relhum != STATION_NAN)){
      ModelReport.relhum = ApplyAdjustment(ModelReport.time - CorrectionMetar.time,
					    ModelReport.relhum, 
					    CorrectionMetar.relhum,
					    P->HumidityAdjustSec,
					    P->AdjustmentType);
  }

  if ((ModelReport.windspd != STATION_NAN) && (CorrectionMetar.windspd != STATION_NAN)){
      ModelReport.windspd = ApplyAdjustment(ModelReport.time - CorrectionMetar.time,
					     ModelReport.windspd, 
					     CorrectionMetar.windspd,
					     P->WindSpeedAdjustSec,
					     P->AdjustmentType);
  }
  
  if ((ModelReport.winddir != STATION_NAN) && (CorrectionMetar.winddir != STATION_NAN)){
      ModelReport.winddir = ApplyAdjustment(ModelReport.time - CorrectionMetar.time,
					     ModelReport.winddir, 
					     CorrectionMetar.winddir,
					     P->WindDirAdjustSec,
					     P->AdjustmentType);
  }
  //
  // Limit directon to 0..360
  //
  if (ModelReport.winddir <   0.0) ModelReport.winddir = ModelReport.winddir + 360.0;
  if (ModelReport.winddir > 360.0) ModelReport.winddir = ModelReport.winddir - 360.0;




}
///////////////////////////////////////////////////////
//
// Small routine to actually apply an individual adjustment.
//
fl32 ApplyAdjustment(time_t tDiff, fl32 Model, fl32 Corrector,
		     int AdjustSec, Params::adjust_t Adjust){




  if ( 
      (Model == STATION_NAN) ||
      (Corrector == STATION_NAN)
      ){
    return STATION_NAN;
  }

  fl32 metarWeight;
  if (Adjust == Params::ADJUST_LINEAR){
    if (tDiff > AdjustSec) {
      metarWeight = 0;
    } else {
      metarWeight = (-1.0 / AdjustSec) * (tDiff) + 1;
    }
    return (1 - metarWeight) * Model + metarWeight * Corrector;
  }

  if (Adjust == Params::ADJUST_SIGMOID){
    if (tDiff > AdjustSec) {
      metarWeight = 0;
    } else {
      metarWeight = 1 /(1 + exp(12 * (tDiff - (AdjustSec/2) ) / AdjustSec) );
    }
    return (1 - metarWeight) * Model + metarWeight * Corrector;
    
  }

  return STATION_NAN;


}


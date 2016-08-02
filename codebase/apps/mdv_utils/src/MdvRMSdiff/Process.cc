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

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxTimeList.hh>
#include <Mdv/Mdvx.hh>

#include <iostream>
#include <Mdv/MdvxField.hh>
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <Mdv/MdvxProj.hh>

using namespace std;

//
// Constructor
//
Process::Process(){
  return;;
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *P, time_t T){

  if (P->Debug){
    cerr << "Data at " << utimstr(T) << endl;
  }


  //
  // Set up for the Truth data.
  //
  DsMdvx Truth;

  Truth.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  Truth.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  Truth.setReadCompressionType(Mdvx::COMPRESSION_NONE);


  if (P->RemapGrid){
    switch ( P->grid_projection){

    case Params::FLAT:
      Truth.setReadRemapFlat(P->grid_nx, P->grid_ny,
			 P->grid_minx, P->grid_miny,
			 P->grid_dx, P->grid_dy,
			 P->grid_origin_lat, P->grid_origin_lon,
			 P->grid_rotation);

      break;
                                   
    case Params::LATLON:
      Truth.setReadRemapLatlon(P->grid_nx, P->grid_ny,
			   P->grid_minx, P->grid_miny,
			   P->grid_dx, P->grid_dy);

      break;            

    case Params::LAMBERT:
      Truth.setReadRemapLc2(P->grid_nx, P->grid_ny,
			P->grid_minx, P->grid_miny,
			P->grid_dx, P->grid_dy,
			P->grid_origin_lat, 
			P->grid_origin_lon,
			P->grid_lat1,  P->grid_lat2);
      
      break;
      
    default:
      cerr << "Unsupported projection." << endl;
      return -1;
      break;
      
    }               
  }

  Truth.addReadField( P->TruthFieldName ); 
  if (Truth.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->TriggerUrl  << endl;
    return -1;
  }     

 
  //
  // Get the desired field.
  //
  MdvxField *TruthField;

  TruthField = Truth.getFieldByName( P->TruthFieldName );

  if (TruthField == NULL){
    cerr << "Truth field " << P->TruthFieldName << " not found." << endl;
    return -1;
  }


  Mdvx::field_header_t TruthFhdr = TruthField->getFieldHeader();
  Mdvx::master_header_t TruthMhdr = Truth.getMasterHeader();



  date_time_t dt;
  dt.unix_time = TruthMhdr.time_centroid;
  uconvert_from_utime( &dt );

  cerr << dt.year << " ";
  cerr << dt.month << " ";
  cerr << dt.day << " ";
  cerr << dt.hour << " ";
  cerr << dt.min << " ";
  cerr << dt.sec << " ";


  fl32 *TruthData = (fl32 *) TruthField->getVol();

  // Loop through the data and get RMS diff. For Truth data,
  // first apply the function that takes DBZ and turns it into QR.
  // According to Andrew the function is QR = 10.**((DBZ-43.1)/17.5)
  //
  for (int i=0; i < TruthFhdr.nx * TruthFhdr.ny * TruthFhdr.nz; i++){
    //
    // For missing data, set QR to 0.0
    //
    if (
	(TruthData[i] == TruthFhdr.bad_data_value) ||
	(TruthData[i] == TruthFhdr.missing_data_value)
	){
      TruthData[i] = 0.0;
    } else {
      //
      // Cap DBZ at 55 dbz at Andrew's suggestion, then apply function.
      //
      if (TruthData[i] > 55.0) TruthData[i] = 55.0;
      double exponent = (TruthData[i] - 43.1)/17.5;
      TruthData[i] = pow(10.0, exponent);
      //
    }
  }

  // Truthdata are now QR not DBZ

  //
  // Loop through the forecast lead times.
  //

  for (int ifcst=0; ifcst < P->forecastLead_n; ifcst++){

    DsMdvx M;

    string url( P->ForecastUrl );
    time_t start_time = TruthMhdr.time_centroid - 7200;
    time_t end_time = TruthMhdr.time_centroid;

    M.setTimeListModeGen(  url, start_time, end_time );
    M.compileTimeList();

    const vector<time_t> &genTimes = M.getGenTimes();

    for ( unsigned it=0; it < genTimes.size(); it++ ){
      cerr << it << " GEN TIME : " << utimstr(genTimes[it]) << endl;
    }

    exit(0);

    //
    // Read in the forecast field.
    //
    DsMdvx Forecast;
    //
    Forecast.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, 
			 P->ForecastUrl, 
			 P->lookBack, 
			 T - P->_forecastLead[ifcst],
			 P->_forecastLead[ifcst] );
    //
    Forecast.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    Forecast.setReadCompressionType(Mdvx::COMPRESSION_NONE);
    //
    // Only take one level from the forecast.
    //
    Forecast.setReadPlaneNumLimits( P->forecastLevel,
				    P->forecastLevel );
    
    if (P->RemapGrid){
      switch ( P->grid_projection){
	
      case Params::FLAT:
	Forecast.setReadRemapFlat(P->grid_nx, P->grid_ny,
				  P->grid_minx, P->grid_miny,
				  P->grid_dx, P->grid_dy,
				  P->grid_origin_lat, P->grid_origin_lon,
				  P->grid_rotation);

	break;
                                   
      case Params::LATLON:
	Forecast.setReadRemapLatlon(P->grid_nx, P->grid_ny,
				    P->grid_minx, P->grid_miny,
				    P->grid_dx, P->grid_dy);
	
	break;            

      case Params::LAMBERT:
	Forecast.setReadRemapLc2(P->grid_nx, P->grid_ny,
				 P->grid_minx, P->grid_miny,
				 P->grid_dx, P->grid_dy,
				 P->grid_origin_lat, 
				 P->grid_origin_lon,
				 P->grid_lat1,  P->grid_lat2);
      
	break;
      
      default:
	cerr << "Unsupported projection." << endl;
	return -1;
	break;
	
      }               
    }
    
    Forecast.addReadField( P->ForecastFieldName );
    if (Forecast.readVolume()){
      cerr << "Read failed at " << utimstr(T) << " from ";
      cerr << P->ForecastUrl  << endl;
      cerr << Forecast.getErrStr() << endl;
      return -1;
    }     

    if (P->Debug){
      cerr << endl << "Comparing " << Truth.getPathInUse() << " to" << endl;
      cerr << Forecast.getPathInUse() << endl << endl;
    }


    MdvxField  *ForecastField;
    ForecastField = Forecast.getFieldByName( P->ForecastFieldName );
    
    if (ForecastField == NULL){
      cerr << "Forecast field " << P->ForecastFieldName << " not found." << endl;
      return -1;
    }
    
    Mdvx::master_header_t ForecastMhdr = Forecast.getMasterHeader();
    Mdvx::field_header_t ForecastFhdr = ForecastField->getFieldHeader();



    if (
	(TruthFhdr.nx != ForecastFhdr.nx) ||
	(TruthFhdr.ny != ForecastFhdr.ny) ||
	(TruthFhdr.nz != ForecastFhdr.nz)
	){
      cerr << "Grids differ, I cannot cope - set grid remapping in param file.";
      cerr << endl;
      cerr << TruthFhdr.nx << TruthFhdr.ny << TruthFhdr.nz << endl;
      cerr << ForecastFhdr.nx << ForecastFhdr.ny << ForecastFhdr.nz << endl;
      exit(-1);
    }
    
    fl32 *ForecastData = (fl32 *) ForecastField->getVol();

    //
    // Loop through the data and get RMS diff. Store this as
    // an MDV field.
    //
    fl32 *trthMinusFcst = (fl32 *) malloc(TruthFhdr.nx * TruthFhdr.ny * TruthFhdr.nz 
				    * sizeof(fl32));

    if (trthMinusFcst == NULL){
      cerr << "Malloc failed." << endl;
      exit(-1);
    }

    int numPoints = 0;
    double sumSqrdDiff = 0.0;
    for (int i=0; i < TruthFhdr.nx * TruthFhdr.ny * TruthFhdr.nz; i++){
      //
      // Initialize output to missing.
      //
      trthMinusFcst[i] = ForecastFhdr.missing_data_value;
      //
      // See if we have both truth and forecast here.
      //
      if (
	  (TruthData[i] != TruthFhdr.bad_data_value) &&
	  (TruthData[i] != TruthFhdr.missing_data_value) &&
	  (ForecastData[i] != ForecastFhdr.bad_data_value) &&
	  (ForecastData[i] != ForecastFhdr.missing_data_value)
	  ){
	//
	double diff = TruthData[i] - ForecastData[i];
	sumSqrdDiff += diff*diff;
	numPoints++;
	trthMinusFcst[i] = diff;
      }
    }

    double rmsDiff = 0.0;
    if (numPoints > 0.0)
      rmsDiff = sqrt( sumSqrdDiff/numPoints );

    // Output data.

    cerr << P->_forecastLead[ifcst] << " ";
    cerr << TruthMhdr.time_centroid - ForecastMhdr.time_centroid - P->_forecastLead[ifcst] << " ";
    cerr << numPoints << " ";
    cerr << rmsDiff << " ";

    //
    // Write MDV data.
    //
    DsMdvx outputMdv;
    outputMdv.clearWrite();

    outputMdv.setMasterHeader( ForecastMhdr );

    Mdvx::vlevel_header_t ForecastVhdr = ForecastField->getVlevelHeader();
    
 
    MdvxField *fld = new MdvxField( ForecastFhdr, ForecastVhdr, trthMinusFcst );
    fld->setFieldName( "Diff" );
    fld->setUnits( "QR" );
    fld->setFieldNameLong( "Truth minus forecast" );
    
    if (fld->convertRounded(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_ZLIB)){
      cerr << "convertRounded failed - I cannot go on." << endl;
      exit(-1);
    }  
    
    Out.addField(fld);
    
    free(trthMinusFcst);
    
    Out.setWriteAsForecast();
    
    if (Out.writeToDir( P->OutputUrl )){
      cerr << "Error on mdv write!" << endl;
      exit(-1);
    }
    
  }

  cerr << endl;

  if (P->Debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }

  return 0;

}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
  return;
}











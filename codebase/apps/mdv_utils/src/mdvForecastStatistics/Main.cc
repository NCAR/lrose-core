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

#include <Mdv/DsMdvx.hh>
#include <toolsa/umisc.h>
#include <iostream>

#include "Params.hh"
#include "mdvTimes.hh"
#include "polyMaps.hh"
#include "workerDrone.hh"
#include "outputMdv.hh"
#include "statistician.hh"

/**
 *
 * Small routine to print command line usage.
 *
 * @author Niles Oien
 */

void usage();



/**
 *
 * Small routine to print an input specification.
 *
 * @author Niles Oien
 */

void printInput(Params::input_t input);


/**
 *
 * Small routine to parse time from command line.
 *
 * @param timeStr the string to parse the time from.
 * @param timeStruct Pointer to date_time_t structure to fill out.
 *
 * @return Integer, 0 if all went well, -1 otherwise.
 *
 * @author Niles Oien
 */

int parseTime(char *timeStr, date_time_t *timeStruct);



/**
 *
 * @file Main.cc
 *
 *
 * The main driver for the mdvForecastStatistics program.
 * Reads the data, instantiates the classes - quite a bit
 * of the work is done at this level.
 *
 *
 * @author Niles Oien
 *
 */


int main(int argc, char *argv[]){

  //
  // Make a note of when we started.
  //
  time_t runStart = time(NULL);

  //
  // Read the command-line specified param file.
  // Do this before time parsing so that -print_params works.
  //
  Params params;

  if (params.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }  

  //
  // If model2Model is set, we are in forecast mode.
  //
  if (params.model2Model) params.forecastMode = (tdrp_bool_t) true;

  //
  // Translate the bad value actions into the constants required by
  // the workerDrone class.
  //
  int truthBadValueAction;
  switch (params.badTruthValueAction){

  case Params::BAD_TRUTH_VALUE_IS_STORM :
    truthBadValueAction = workerDrone::badValueActionStorm;
    break;

  case Params::BAD_TRUTH_VALUE_IS_NOT_STORM :
    truthBadValueAction = workerDrone::badValueActionNoStorm;
    break;

  case Params::BAD_TRUTH_VALUE_IS_IGNORED :
    truthBadValueAction = workerDrone::badValueActionIgnore;
    break;

  default :
    cerr << "Unrecognised value for badTruthValueAction" << endl;
    exit(-1);
    break;

  }

  int forecastBadValueAction;
  switch (params.badForecastValueAction){

  case Params::BAD_FORECAST_VALUE_IS_STORM :
    forecastBadValueAction = workerDrone::badValueActionStorm;
    break;

  case Params::BAD_FORECAST_VALUE_IS_NOT_STORM :
    forecastBadValueAction = workerDrone::badValueActionNoStorm;
    break;

  case Params::BAD_FORECAST_VALUE_IS_IGNORED :
    forecastBadValueAction = workerDrone::badValueActionIgnore;
    break;

  default :
    cerr << "Unrecognised value for badForecastValueAction" << endl;
    exit(-1);
    break;

  }

  //
  // Parse the start and end times from the command line args.
  //
  date_time_t startTime, endTime;
  startTime.unix_time =0L; endTime.unix_time = 0L;

  for (int i=1; i < argc; i++){

    if (!(strcmp(argv[i], "-h"))){
      usage(); exit(-1);
    }

    if (!(strcmp(argv[i], "-start"))){
      if (i == argc-1){
	cerr << "ERROR - -start arg needs time/date, ie. -start \"2006 10 15 10 30 00\"" << endl << endl;
	usage(); exit(-1);
      }
      if (parseTime(argv[i+1], &startTime)){
	cerr << "ERROR - failed to parse start time from " << argv[i+1] << endl;
	exit(-1);
      }
    }

    if (!(strcmp(argv[i], "-end"))){
      if (i == argc-1){
	cerr << "ERROR - -end arg needs time/date, ie. -end \"2006 10 15 10 30 00\"" << endl << endl;
	usage(); exit(-1);
      }
      if (parseTime(argv[i+1], &endTime)){
	cerr << "ERROR - failed to parse end time from " << argv[i+1] << endl;
	exit(-1);
      }
    }
  }

  if ((startTime.unix_time == 0L) || (endTime.unix_time == 0L)){
    cerr << "I need start and end times specified on the command line." << endl << endl;
    usage();    exit(-1);
  }


  //
  // Set up a vector of statisticians, if we're doing ASCII output.
  //
  vector < statistician * > Stats;
  if (strlen(params.outDir) > 0) {
    for (int i=0; i < params.thresholds_n; i++){

      statistician *S = new statistician(params._thresholds[i].name,
					 startTime.unix_time, params.outDir, params.outputInterval);
      S->init();

      Stats.push_back( S );
    }
  }


  //
  // Some debugging.
  //
  if (params.debug >= Params::DEBUG_VERBOSE){
    cerr << "Input spec for truth :" << endl; printInput( params._inputs[0] );
    cerr << "Input spec for forecast :" << endl; printInput( params._inputs[1] );
  }

  //
  // Instantiate an mdvTimes class and use it to get the forecast
  // data times.
  //
  // inputs[0] == truth input, inputs[1] == forecast input.
  //
  mdvTimes forecastTimeList(params._inputs[1].url, params.forecastMode, startTime.unix_time, endTime.unix_time);
  int numForecastTimes = forecastTimeList.getNumTimes();

  if (params.debug){
    cerr << numForecastTimes << " forecasts found at " << params._inputs[1].url;
    cerr << " between " << utimstr(startTime.unix_time) << " and " << utimstr(endTime.unix_time) << endl;
  }

  //
  // Keep track of if we have read a truth/forecast data pair
  // successfully or not. If we do, we'll use the first pair
  // to set up the map masks. Until then, have them be NULL.
  //
  bool readDataPair = false;
  ui08 *truthMapMask = NULL; ui08 *forecastMapMask = NULL;

  for (int iForecast=0; iForecast < numForecastTimes; iForecast++){

    mdvTimes::mdvTime_t forecastTime = forecastTimeList.getNextTime();

    if (params.debug){
      cerr << "Forecast time " << iForecast+1 << " of " << numForecastTimes;
      if (forecastTime.leadTime == mdvTimes::undefinedLeadTime){
	cerr << " : " << utimstr(forecastTime.genTime) << endl;
      } else {
	cerr << " : GenTime : " << utimstr(forecastTime.genTime);
	cerr << " validTime : " << utimstr(forecastTime.genTime + forecastTime.leadTime);
	cerr << " (lead " << forecastTime.leadTime/60.0 << " minutes.)" << endl;
      }
    }

    //
    // See if we skip this lead time.
    //
    if ((params.forecastMode) && (params.useSpecifiedForecastLeadtimes)) {
      bool wantLeadTime = false;
      for (int iw=0; iw < params.specifiedForecastLeadtimes_n; iw++){
	if (forecastTime.leadTime == params._specifiedForecastLeadtimes[iw]){
	  wantLeadTime = true;
	  break;
	  }
      }
      if (!(wantLeadTime)){
	if (params.debug){
	  cerr << "Lead time not in specified lead time list, skipping..." << endl;
	}
	continue;
      }
    }

    //
    // See if we skip this gen time.
    //
    if ((params.forecastMode) && (params.useSpecifiedGentimes)) {
      if ((forecastTime.genTime - params.specifiedGenTimes.offset) % params.specifiedGenTimes.interval != 0){
	if (params.debug){
	  cerr << "Generation time is not consistent with specifiedGenTimes parameter, skipping..." << endl;
	}
	continue;
      }
    }

    //
    // Read the forecast at this time
    //
    DsMdvx forecastMdvx;
    forecastMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    forecastMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
    forecastMdvx.addReadField( params._inputs[1].fieldName );
    
    if (params._inputs[1].vertAction != Params::VERT_ACTION_NONE){
      //
      // Set limits in vertical. We do this if we are taking any vertical action.
      //
      if (params._inputs[1].vlevelsSpecified){
	forecastMdvx.setReadVlevelLimits( params._inputs[1].verticalMin, 
					  params._inputs[1].verticalMax );
      } else {
	forecastMdvx.setReadPlaneNumLimits( (int)rint(params._inputs[1].verticalMin), 
					    (int)rint(params._inputs[1].verticalMax) );
      }
      //
      // Set compositing if desired.
      //
      if (params._inputs[1].vertAction == Params::VERT_ACTION_COMPOSITE)
	forecastMdvx.setReadComposite();
    }

    if (params._inputs[1].doGridRemap){

      switch (params._inputs[1].projType){

      case Params::PROJ_FLAT :
	forecastMdvx.setReadRemapFlat(params._inputs[1].nx, 
				      params._inputs[1].ny,
				      params._inputs[1].minX, 
				      params._inputs[1].minY,
				      params._inputs[1].dx, 
				      params._inputs[1].dy,
				      params._inputs[1].originLat,
				      params._inputs[1].originLon,
				      params._inputs[1].flatRotation);
	break;

      case Params::PROJ_LATLON :
	forecastMdvx.setReadRemapLatlon(params._inputs[1].nx, 
					params._inputs[1].ny,
					params._inputs[1].minX, 
					params._inputs[1].minY,
					params._inputs[1].dx, 
					params._inputs[1].dy);
	break;

      case Params::PROJ_LAMBERT :
	forecastMdvx.setReadRemapLc2(params._inputs[1].nx, 
				     params._inputs[1].ny,
				     params._inputs[1].minX, 
				     params._inputs[1].minY,
				     params._inputs[1].dx, 
				     params._inputs[1].dy,
				     params._inputs[1].originLat,
				     params._inputs[1].originLon,
				     params._inputs[1].trueLambertLat1,
				     params._inputs[1].trueLambertLat2);
	break;

      default :
	cerr << "Unknown forecast grid remap projection type!" << endl;
	exit(-1);
	break;

      }

      if (params.fcstFalseOrigin.falseOrigin){

	if (!(params.fcstFalseOrigin.latLon)){
	  //
	  // It is not lat/lon - this is the easier option.
	  // Plug the x and y specified into the remap process.
	  //
	  forecastMdvx.setReadFalseCoords(params.fcstFalseOrigin.y,
					  params.fcstFalseOrigin.x);

	} else {
	  //
	  // It is lat/lon. In this case we need to do some
	  // work to get the x and y so we can plug them into the remap process.
	  // To do this we need a pjgMath object.
	  //
	  PjgMath *pmath = NULL;

	  bool ok4FalseNorthing = true;
	  switch (params._inputs[1].projType){

	  case Params::PROJ_FLAT :
	    pmath = new PjgAzimEquidistMath( params._inputs[1].originLat,
					     params._inputs[1].originLon,
					     params._inputs[1].flatRotation );
	    break;


	  case Params::PROJ_LAMBERT :
	    pmath = new PjgLambertConfMath( params._inputs[1].originLat,
					    params._inputs[1].originLon,
					    params._inputs[1].trueLambertLat1,
					    params._inputs[1].trueLambertLat2 );

	    break;

	    //
	    // Flat and Lambert are the only two projections false northing makes much
	    // sense for here. It does not apply to lat/lon, so if that's what we've got then
	    // set a boolean so we print a message that the false easting/northing is being ignored.
	    //
	  default :
	    ok4FalseNorthing = false;
	    break;

	  } // End of switch for what projection we have.


	  if (ok4FalseNorthing){
	    pmath->setOffsetOrigin( params.fcstFalseOrigin.y,
				    params.fcstFalseOrigin.x );
	    forecastMdvx.setReadFalseCoords(pmath->getFalseNorthing(),
					    pmath->getFalseEasting());
	    delete pmath;
	  } else {
	    cerr << "WARNING : A false origin was specified for the forecast field but" << endl;
	    cerr << "          the projection is such that it will have no effect. The" << endl;
	    cerr << "          false origin values will be ignored." << endl << endl;
	  }

	}
	

      } // End of if we are asking for a false origin on forecast


    }

    if (params.forecastMode){
      forecastMdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, params._inputs[1].url, 0, forecastTime.genTime, forecastTime.leadTime );
    } else {
      forecastMdvx.setReadTime(Mdvx::READ_FIRST_BEFORE, params._inputs[1].url, 0, forecastTime.genTime );
    }

    if (forecastMdvx.readVolume()){
      cerr << "Failed to read forecast from " << params._inputs[1].url << endl;
       if (params.forecastMode){
	 cerr << " genTime=" << utimstr(forecastTime.genTime);
	 cerr << " leadTime=" << forecastTime.leadTime << endl;
       } else {
	 cerr << " dataTime=" << utimstr(forecastTime.genTime) << endl;
       }
       continue; // Try to read next forecast.
    }

    /////////////// Now try to do similar for truth /////////////////////////////////////


    DsMdvx truthMdvx;
    truthMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    truthMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
    truthMdvx.addReadField( params._inputs[0].fieldName );
    
    if (params._inputs[0].vertAction != Params::VERT_ACTION_NONE){
      if (params._inputs[0].vlevelsSpecified){
	truthMdvx.setReadVlevelLimits( params._inputs[0].verticalMin, 
				       params._inputs[0].verticalMax );
      } else {
	truthMdvx.setReadPlaneNumLimits( (int)rint(params._inputs[0].verticalMin), 
					 (int)rint(params._inputs[0].verticalMax) );
      }

      if (params._inputs[0].vertAction == Params::VERT_ACTION_COMPOSITE)
	truthMdvx.setReadComposite();
    }

    if (params._inputs[0].doGridRemap){

      switch (params._inputs[0].projType){

      case Params::PROJ_FLAT :
	truthMdvx.setReadRemapFlat(params._inputs[0].nx, 
				   params._inputs[0].ny,
				   params._inputs[0].minX, 
				   params._inputs[0].minY,
				   params._inputs[0].dx, 
				   params._inputs[0].dy,
				   params._inputs[0].originLat,
				   params._inputs[0].originLon,
				   params._inputs[0].flatRotation);
	break;
	
      case Params::PROJ_LATLON :
	truthMdvx.setReadRemapLatlon(params._inputs[0].nx, 
				     params._inputs[0].ny,
				     params._inputs[0].minX, 
				     params._inputs[0].minY,
				     params._inputs[0].dx, 
				     params._inputs[0].dy);
	break;

      case Params::PROJ_LAMBERT :
	truthMdvx.setReadRemapLc2(params._inputs[0].nx, 
				  params._inputs[0].ny,
				  params._inputs[0].minX, 
				  params._inputs[0].minY,
				  params._inputs[0].dx, 
				  params._inputs[0].dy,
				  params._inputs[0].originLat,
				  params._inputs[0].originLon,
				  params._inputs[0].trueLambertLat1,
				  params._inputs[0].trueLambertLat2);
	break;

      default :
	cerr << "Unknown truth grid remap projection type!" << endl;
	exit(-1);
	break;

      }

      if (params.truthFalseOrigin.falseOrigin){

	if (!(params.truthFalseOrigin.latLon)){
	  //
	  // It is not lat/lon - this is the easier option.
	  // Plug the x and y specified into the remap process.
	  //
	  truthMdvx.setReadFalseCoords(params.truthFalseOrigin.y,
				       params.truthFalseOrigin.x);

	} else {
	  //
	  // It is lat/lon. In this case we need to do some
	  // work to get the x and y so we can plug them into the remap process.
	  // To do this we need a pjgMath object.
	  //
	  PjgMath *pmath = NULL;

	  bool ok4FalseNorthing = true;
	  switch (params._inputs[0].projType){

	  case Params::PROJ_FLAT :
	    pmath = new PjgAzimEquidistMath( params._inputs[0].originLat,
					     params._inputs[0].originLon,
					     params._inputs[0].flatRotation );
	    break;


	  case Params::PROJ_LAMBERT :
	    pmath = new PjgLambertConfMath( params._inputs[0].originLat,
					    params._inputs[0].originLon,
					    params._inputs[0].trueLambertLat1,
					    params._inputs[0].trueLambertLat2 );

	    break;

	    //
	    // Flat and Lambert are the only two projections false northing makes much
	    // sense for here. It does not apply to lat/lon, so if that's what we've got then
	    // set a boolean so we print a message that the false easting/northing is being ignored.
	    //
	  default :
	    ok4FalseNorthing = false;
	    break;

	  } // End of switch for what projection we have.


	  if (ok4FalseNorthing){
	    pmath->setOffsetOrigin( params.truthFalseOrigin.y,
				    params.truthFalseOrigin.x );
	    truthMdvx.setReadFalseCoords(pmath->getFalseNorthing(),
					 pmath->getFalseEasting());
	    delete pmath;
	  } else {
	    cerr << "WARNING : A false origin was specified for the truth field but" << endl;
	    cerr << "          the projection is such that it will have no effect. The" << endl;
	    cerr << "          false origin values will be ignored." << endl << endl;
	  }

	}
	

      } // End of if we are asking for a false origin on forecast



    }

    //
    // Get the valid time for this forecast, which is what
    // we want to use as the time to read truth data.
    //
    time_t forecastValidTime = forecastTime.genTime;
    if (params.forecastMode) {
      forecastValidTime += forecastTime.leadTime;
    } else {
      forecastValidTime += params.forecastLeadtime;
    }


    if (params.model2Model){
      truthMdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, params._inputs[0].url, 0, forecastTime.genTime, forecastTime.leadTime );
    } else {
      truthMdvx.setReadTime(Mdvx::READ_CLOSEST, params._inputs[0].url, params.temporalTolerance, forecastValidTime);
    }

    if (truthMdvx.readVolume()){
      cerr << "Failed to read truth from " << params._inputs[0].url << endl;
       if (params.model2Model){
	 cerr << " genTime=" << utimstr(forecastTime.genTime);
	 cerr << " leadTime=" << forecastTime.leadTime << endl;
       } else {
	 cerr << " dataTime=" << utimstr(forecastTime.genTime);
       }
       continue; // Try to read next forecast.
    }

    //
    // Compare the two grid dimensions to make sure we don't have differing grids.
    //
    MdvxField *forecastField = forecastMdvx.getFieldByNum( 0 );
    if (forecastField == NULL){
      cerr << "Forecast field name " << params._inputs[1].fieldName << " not found." << endl;
      continue;
    }

    MdvxField *truthField = truthMdvx.getFieldByNum( 0 );
    if (truthField == NULL){
      cerr << "Truth field name " << params._inputs[0].fieldName << " not found." << endl;
      continue;
    }

    Mdvx::field_header_t forecastFhdr = forecastField->getFieldHeader();
    Mdvx::field_header_t truthFhdr = truthField->getFieldHeader();

    Mdvx::vlevel_header_t forecastVhdr = forecastField->getVlevelHeader();
    Mdvx::vlevel_header_t truthVhdr = truthField->getVlevelHeader();

    if (
	(forecastFhdr.nx != truthFhdr.nx) ||
	(forecastFhdr.ny != truthFhdr.ny) ||
	(forecastFhdr.nz != truthFhdr.nz)
	){
      cerr << "ERROR - grid sizes differ, skipping forecast :" << endl;
      cerr << "Forecast : " << forecastFhdr.nx << " by " << forecastFhdr.ny << " by " << forecastFhdr.nz << endl;
      cerr << "Truth : " << truthFhdr.nx << " by " << truthFhdr.ny << " by " << truthFhdr.nz << endl;
      continue;
    }

    //
    // Grids are compatible. Set variables to grid size and get master headers.
    //
    int nx = forecastFhdr.nx;     int ny = forecastFhdr.ny;     int nz = forecastFhdr.nz; 

    Mdvx::master_header_t forecastMhdr = forecastMdvx.getMasterHeader();
    Mdvx::master_header_t truthMhdr = truthMdvx.getMasterHeader();

    if (params.debug){
      cerr << "Comparing truth data in " << truthMdvx.getPathInUse() << endl;
      cerr << "     to forecast data from " << forecastMdvx.getPathInUse() << endl;
    }

    //
    // We have done the work of reading the data. Now it's time to
    // load up some classes and get them to do the work of thresholding,
    // applying map files and clumping limits, generating storm/no storm tables
    // and an outcome table.
    //

    //
    // If this is the first truth/forecast pair we've read, and we
    // are doing mapping, then set up the mapping masks.
    //
    if (!(readDataPair)){
      readDataPair = true;

      if (params._inputs[0].useMapFile){
	if (params.debug){
	  cerr << "Initializing truth map mask from file ";
	  cerr << params._inputs[0].mapFilename << endl;
	}
	truthMapMask = (ui08 *) malloc(nx*ny*sizeof(ui08));
	if (truthMapMask == NULL){
	  cerr << "Malloc failed for truth map mask in main." << endl;
	  exit(-1);
	}
	polyMaps truthMap(params._inputs[0].mapFilename, truthMhdr, truthFhdr);
	memcpy(truthMapMask, truthMap.getMapMask(), nx*ny*sizeof(ui08));

	if (params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "Truth map file indicies in grid : " << endl;
	  truthMap.printMaps();
	}
      }

      if (params._inputs[1].useMapFile){
	if (params.debug){
	  cerr << "Initializing forecast map mask from file ";
	  cerr << params._inputs[1].mapFilename;
	}
	forecastMapMask = (ui08 *) malloc(nx*ny*sizeof(ui08));
	if (forecastMapMask == NULL){
	  cerr << "Malloc failed for forecast map mask in main." << endl;
	  exit(-1);
	}
	polyMaps forecastMap(params._inputs[1].mapFilename, forecastMhdr, forecastFhdr);
	memcpy(forecastMapMask, forecastMap.getMapMask(), nx*ny*sizeof(ui08));
	
	if (params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "Forecast map file indicies in grid : " << endl;
	  forecastMap.printMaps();
	}
      }
    }


    //
    // Time to instantiate (finally) the classes that do the work.
    //
    workerDrone truthWorker( (fl32 *)truthField->getVol(), truthFhdr.bad_data_value,
			     truthFhdr.missing_data_value, nx, ny, nz);

    workerDrone forecastWorker( (fl32 *)forecastField->getVol(), forecastFhdr.bad_data_value,
				forecastFhdr.missing_data_value, nx, ny, nz);
    

    //
    // Now loop through the thresholds.
    //
    for (int iThresh=0; iThresh < params.thresholds_n; iThresh++){
      //
      // Apply the thesholds, apply clumping criteria, apply the map mask, apply relaxation
      // to the truth data.
      //
      truthWorker.calcThresh( params._thresholds[iThresh].truth_min,
			      params._thresholds[iThresh].truth_max,
			      truthBadValueAction);
      if (params._inputs[0].doClumping) truthWorker.applyClump( params._inputs[0].minClumpsize );
      truthWorker.applyHorizontalRelaxation( params._thresholds[iThresh].truth_relax );
      if (params._inputs[0].useMapFile) truthWorker.applyMapMask( truthMapMask );

      //
      // And likewise for the forecast data.
      //
      forecastWorker.calcThresh( params._thresholds[iThresh].forecast_min,
			      params._thresholds[iThresh].forecast_max,
			      forecastBadValueAction);
      if (params._inputs[1].doClumping) forecastWorker.applyClump( params._inputs[1].minClumpsize );
      forecastWorker.applyHorizontalRelaxation( params._thresholds[iThresh].forecast_relax );
      if (params._inputs[1].useMapFile) forecastWorker.applyMapMask( forecastMapMask );

      //
      // Now, with both datasets thresholded, do the validation.
      //
      forecastWorker.validate( truthWorker.getThresholdedData() );

      int numMiss, numHit, numNon, numFalse, numConsidered;
      forecastWorker.getCounts( &numMiss, &numHit, &numNon, 
				&numFalse, &numConsidered);
      

      if (params.debug){

	cerr << "For threshold " << params._thresholds[iThresh].name;
	cerr << " numNon=" << numNon;
	cerr << " numMiss=" << numMiss;
	cerr << " numFalse=" << numFalse;
	cerr << " numHit=" << numHit;
	cerr << " numConsidered=" << numConsidered;
	cerr << endl;
	if (iThresh == params.thresholds_n-1) cerr << endl; // Blank line at end of this timestep.
      }

      //
      // If we're outputting ASCII data, do that.
      //
      if (Stats.size() > 0){
	vector < statistician * >::iterator it;
	it = Stats.begin() + iThresh;
	(*it)->accumulate(numNon, numMiss, numFalse, numHit, 
			  truthMhdr.time_centroid, forecastFhdr.forecast_delta,
			  forecastMhdr.data_set_source);
      }

      //
      // If we're ouputting intermediate grids, do that.
      //
      if (strlen(params.intermediateUrl) > 0){
	outputMdv intermediateOutput(params.intermediateUrl, params._thresholds[iThresh].name,
				     forecastMdvx.getPathInUse(), truthMdvx.getPathInUse(),
				     forecastMhdr, forecastFhdr, forecastVhdr);

	if (params.forecastMode)
	  intermediateOutput.setForecastMode(true );

	string forecastFieldname = "fcst_";  forecastFieldname += forecastFhdr.field_name;
	string forecastFieldnameLong = "fcst_";  forecastFieldnameLong += forecastFhdr.field_name_long;

	intermediateOutput.addField((fl32 *)forecastField->getVol(), forecastFhdr.bad_data_value,
				    forecastFhdr.missing_data_value, forecastFieldname,
				    forecastFieldnameLong, forecastFhdr.units, true);

	string truthFieldname = "truth_";  truthFieldname += truthFhdr.field_name;
	string truthFieldnameLong = "truth_";  truthFieldnameLong += truthFhdr.field_name_long;

	intermediateOutput.addField((fl32 *)truthField->getVol(), truthFhdr.bad_data_value,
				    truthFhdr.missing_data_value, truthFieldname,
				    truthFieldnameLong, truthFhdr.units, true);

	intermediateOutput.addField(forecastWorker.getThresholdedData(), -999.0, -999.0,
				    "threshedFcst", "thresholdedForecast", "none", true);

	intermediateOutput.addField(truthWorker.getThresholdedData(), -999.0, -999.0,
				    "threshedTruth", "thresholdedTruth", "none", true);

	intermediateOutput.addField(forecastMapMask, -999.0, -999.0, "fcstMap", "forecastMapMask", "none", false);
				
	intermediateOutput.addField(truthMapMask, -999.0, -999.0, "truthMap", "truthMapMask", "none", false);

	intermediateOutput.addField(forecastWorker.getValidationData(), -999.0, -999.0, "valid", "validationData", "none", true);

      }

    }



  } // End of loop through forecasts.

  //
  // Free up memory.
  //
  if (NULL != forecastMapMask) free(forecastMapMask);
  if (NULL != truthMapMask) free(truthMapMask);

  if (Stats.size() > 0){
    vector < statistician * >::iterator it;
    for (it = Stats.begin(); it != Stats.end(); it++){
      delete (*it);
    }
  }


  if (params.debug){
    time_t runEnd = time(NULL);
    cerr << "Run started at " << utimstr(runStart) << " and ended at " << utimstr(runEnd) << endl;
    cerr << "Run took " << runEnd - runStart << " seconds (" << ceil(double(runEnd - runStart)/60.0) << " minutes)." << endl;
  }

  return 0;

}




int parseTime(char *timeStr, date_time_t *timeStruct){

  if (6 != sscanf(timeStr, "%d %d %d %d %d %d",
		  &timeStruct->year,  &timeStruct->month,  &timeStruct->day, 
		  &timeStruct->hour,  &timeStruct->min,  &timeStruct->sec))
    return -1;

  uconvert_to_utime( timeStruct );

  return 0;

}



void usage(){

  cerr << "USAGE : mdvForecastStatistics -params <paramFile> -start \"YYYY MM DD hh mm ss\"";
  cerr << " -end \"YYYY MM DD hh mm ss\"" << endl;
  cerr << "Times are UTC." << endl;

  return;

}


void printInput(Params::input_t input){

  cerr << "  url : " << input.url << endl;
  cerr << "  fieldName : " << input.fieldName << endl;

  if (input.useMapFile)
    cerr << "  using map file " << input.mapFilename << endl;
  else
    cerr << "  no map file in use" << endl;

  if (input.doClumping)
    cerr << "  minimum clump size is " << input.minClumpsize << " grid points." << endl;
  else
    cerr << "  no clump removal is invoked." << endl;



  if (input.vertAction == Params::VERT_ACTION_NONE){
    cerr << "  no vertical compositing or limits set." << endl;
  } else {
    
    if (input.vertAction == Params::VERT_ACTION_SET_LIMITS){
      cerr << "  setting vertical limits between ";
    } else {
      cerr << "  compositing between ";
    }
    if (input.vlevelsSpecified)
      cerr << "vertical levels " << input.verticalMin << " and " << input.verticalMax << endl;
    else
      cerr << "plane numbers " << (int)rint(input.verticalMin) << " and " << (int)rint(input.verticalMax) << endl;
  }

  if (!(input.doGridRemap)){
    cerr << "  no grid remapping invoked, native grid used." << endl;
  } else {
    cerr << "  remapping to ";
    switch (input.projType){

    case Params::PROJ_FLAT :
      cerr << "flat";
      break;

    case Params::PROJ_LATLON :
      cerr << "latlon";
      break;

    case Params::PROJ_LAMBERT :
      cerr << "lambert";
      break;

    default :
      cerr << "unknown";
      break;

    }
    cerr << " projection, grid parameters are :" << endl;

    cerr << "    Nx=" << input.nx << " ";
    cerr << "Ny=" << input.ny << " ";

    cerr << "Dx=" << input.dx << " ";
    cerr << "Dy=" << input.dy << " ";

    if (Params::PROJ_LAMBERT == input.projType){
      cerr << "Lambert true latitude 1=" << input.trueLambertLat1 << " ";
      cerr << "Lambert true latitude 2=" << input.trueLambertLat2 << " ";
    }

    if (Params::PROJ_FLAT == input.projType){
      cerr << "Rotation=" << input.flatRotation << " ";
    }
 
   
    cerr << "originLat=" << input.originLat << " ";
    cerr << "originLon=" << input.originLon << " ";

    cerr << "minX=" << input.minX << " ";
    cerr << "minY=" << input.minY;
    cerr << endl;
  }

  cerr << endl;

  return;

}


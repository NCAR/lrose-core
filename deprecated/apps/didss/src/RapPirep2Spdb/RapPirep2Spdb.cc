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
////////////////////////////////////////////////////////////////////////
// RapPirep2Spdb.cc
//
// RapPirep2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2000
//
///////////////////////////////////////////////////////////////////////
//
// RapPirep2Spdb reads ASCII Pirep data and puts to Spdb
//
////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cerrno>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <didss/DsInputPath.hh>
#include <rapformats/pirep.h>
#include "RapPirep2Spdb.hh"
using namespace std;

// Constructor

RapPirep2Spdb::RapPirep2Spdb(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "RapPirep2Spdb";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

RapPirep2Spdb::~RapPirep2Spdb()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RapPirep2Spdb::Run ()
{

  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run");

  // input file object

  DsInputPath *input;
  
  if (_params.mode == Params::ARCHIVE) {
    input = new DsInputPath((char *) _progName.c_str(),
			    _params.debug >= Params::DEBUG_VERBOSE,
			    _args.filePaths);
  } else {
    input = new DsInputPath((char *) _progName.c_str(),
			    _params.debug >= Params::DEBUG_VERBOSE,
			    _params.input_dir,
			    _params.max_realtime_valid_age,
			    PMU_auto_register);
  }

  char *inputFilePath;
  if (_params.mode == Params::ARCHIVE && _params.debug) {
    input->reset();
    cerr << "ARCHIVE mode" << endl;
    cerr << "Nfiles: " << _args.filePaths.size() << endl;
    cerr << "FileList:" << endl;
    while ((inputFilePath = input->next()) != NULL) {
      cerr << "  " <<  inputFilePath << endl;
    }
  }
  
  input->reset();
  while ((inputFilePath = input->next()) != NULL) {
    if (_parseInput(inputFilePath)) {
      cerr << "ERROR - parsing file: " << inputFilePath << endl;
      iret = -1;
      break;
    }
  }

  delete input;
  return iret;

}

//////////////////////////////////////////////////
// parse file

int RapPirep2Spdb::_parseInput (const string &inputFilePath)

{

  if (_params.debug) {
    cerr << "Processing file:  " <<  inputFilePath << endl;
  }

  char line[BUFSIZ];
  FILE *in;
  
  if ((in = fopen(inputFilePath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - RapPirep2Spdb::_parseInput" << endl;
    cerr << "  Pasring file: " << inputFilePath << endl;
    cerr << "  "  << strerror(errNum) << endl;
    return -1;
  }
  
  // SPDB output object

  DsSpdb spdb;

  while (!feof(in)) {

    // read in a line

    if (fgets(line, BUFSIZ, in) == NULL) {
      continue;
    }
    
    // scan line

    time_t reportTime;     // unix time for pirep
    int flightLevel;       // flight level
    double lat;             // latitude
    double lon;             // longitude
    int skyBase1;          // sky base
    int skyTop1;           // sky top
    int skyCvg1;           // sky coverage
    int skyBase2;          // sky base
    int skyTop2;           // sky top
    int skyCvg2;           // sky coverage
    int visibility;        // visibility
    int obsc;              // obstruction
    int temp;              // temperature
    int windDirn;          // wind direction
    int windSpeed;         // wind speed
    int turbBase1;         // turbulence base 1 in feet MSL
    int turbTop1;          // turbulence top 1 in feet MSL
    int turbFreq1;         // turbulence frequency 1
    int turbIntensity1;    // turbulence intensity 1
    int turbType1;         // turbulence type 1
    int turbBase2;         // turbulence base 2 in feet MSL
    int turbTop2;          // turbulence top 2 in feet MSL
    int turbFreq2;         // turbulence frequency 2
    int turbIntensity2;    // turbulence intensity 2
    int turbType2;         // turbulence type 2
    int elevGroup; // set to 1 or 2 depending on elev group for internal use
    int pos;       // posn of end of read
    char acType[PIREP_CALLSIGN_LEN]; // aircraft callsign
    char callSign[PIREP_CALLSIGN_LEN]; // aircraft callsign
    
    int iret = sscanf(line,
		      "%ld%lf%lf%d%d%d%d%d%d%d%d%d%d%d%d%*d%*d%*d%*d%*d%*d%*d%*d%d%d%d%d%d%d%d%d%d%d%s%s%n",
		      &reportTime, &lat, &lon, &flightLevel,
		      &skyBase1, &skyCvg1, &skyTop1,
		      &skyBase2, &skyCvg2, &skyTop2,
		      &visibility, &obsc, &temp, &windDirn, &windSpeed,
		      &turbBase1, &turbTop1, &turbFreq1,
		      &turbIntensity1, &turbType1,
		      &turbBase2, &turbTop2, &turbFreq2,
		      &turbIntensity2, &turbType2,
		      acType, callSign,
		      &pos); 

    if (iret < 25) {
      cerr << "ERROR - RapPirep2Spdb::_parseInput" << endl;
      cerr << "  Bad line: " << line;
      continue;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {

      cerr << "  ==================================================" << endl;
      cerr << line;
      cerr << "  ==================================================" << endl;
      cerr << "  reportTime: " << utimstr(reportTime) << endl;
      cerr << "  flightLevel: " << flightLevel << endl;
      cerr << "  lat: " << lat << endl;
      cerr << "  lon: " << lon << endl;
      cerr << "  skyBase1: " << skyBase1 << endl;
      cerr << "  skyTop1: " << skyTop1 << endl;
      cerr << "  skyCvg1: " << skyCvg1 << endl;
      cerr << "  skyBase2: " << skyBase2 << endl;
      cerr << "  skyTop2: " << skyTop2 << endl;
      cerr << "  skyCvg2: " << skyCvg2 << endl;
      cerr << "  visibility: " << visibility << endl;
      cerr << "  obsc: " << obsc << endl;
      cerr << "  temp: " << temp << endl;
      cerr << "  windDirn: " << windDirn << endl;
      cerr << "  windSpeed: " << windSpeed << endl;
      cerr << "  turbBase1: " << turbBase1 << endl;
      cerr << "  turbTop1: " << turbTop1 << endl;
      cerr << "  turbFreq1: " << turbFreq1 << endl;
      cerr << "  turbIntensity1: " << turbIntensity1 << endl;
      cerr << "  turbType1: " << turbType1 << endl;
      cerr << "  turbBase2: " << turbBase2 << endl;
      cerr << "  turbTop2: " << turbTop2 << endl;
      cerr << "  turbFreq2: " << turbFreq2 << endl;
      cerr << "  turbIntensity2: " << turbIntensity2 << endl;
      cerr << "  turbType2: " << turbType2 << endl;
      cerr << "  elevGroup: " << elevGroup << endl;
      cerr << "  acType: " << acType << endl;
      cerr << "  callSign: " << callSign << endl;
      cerr << "  pos: " << pos << endl;

    }

    // store the pirep

    _store (spdb,
	    reportTime,
	    flightLevel,
	    lat,
	    lon,
	    skyBase1,
	    skyTop1,
	    skyCvg1,
	    visibility,
	    obsc,
	    temp,
	    windDirn,
	    windSpeed,
	    turbBase1,
	    turbTop1,
	    turbFreq1,
	    turbIntensity1,
	    turbType1,
	    callSign);

    // if secondary fields are active, dither the position and store the
    // secondary fields

    if (skyCvg2 != -9 || turbIntensity2 != -9) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "------->> Storing secondary pirep <<--------" << endl;
      }
      _store (spdb,
	      reportTime,
	      flightLevel,
	      lat + 0.05,
	      lon + 0.05,
	      skyBase2,
	      skyTop2,
	      skyCvg2,
	      visibility,
	      obsc,
	      temp,
	      windDirn,
	      windSpeed,
	      turbBase2,
	      turbTop2,
	      turbFreq2,
	      turbIntensity2,
	      turbType2,
	      callSign);
    }
  
  } // while

  fclose(in);

  return 0;

}
  
///////////////////
// store the pirep

int RapPirep2Spdb::_store (DsSpdb &spdb,
			   time_t reportTime,
			   int flightLevel,
			   double lat,
			   double lon,
			   int skyBase,
			   int skyTop,
			   int skyCvg,
			   int visibility,
			   int obsc,
			   int temp,
			   int windDirn,
			   int windSpeed,
			   int turbBase,
			   int turbTop,
			   int turbFreq,
			   int turbIntensity,
			   int turbType,
			   char *callSign)

{

  pirep_t pirep;
  pirep_init(&pirep);

  pirep.time = reportTime;
  pirep.lat = lat;
  pirep.lon = lon;
  pirep.alt = flightLevel * 100.0;
  
  if (temp != -9) {
    pirep.temp = temp;
  }
  if (visibility != -9) {
    pirep.visibility = visibility;
  }
  if (windSpeed != -9) {
    pirep.wind_speed = windSpeed;
  }
  if (windDirn != -9) {
    pirep.wind_dirn = windDirn;
  }
  
  pirep.turb_fl_base = turbBase;
  pirep.turb_fl_top = turbTop;
  
  pirep.turb_freq = turbFreq;
  pirep.turb_index = turbIntensity;
  pirep.sky_index = skyCvg;
  
  STRncopy(pirep.callsign, callSign, PIREP_CALLSIGN_LEN);

  char textBuf[256];
  string textStr;
  
  if (turbIntensity != -9) {
    sprintf(textBuf, "Turb:%d ", turbIntensity);
    textStr += textBuf;
    if (turbFreq != -9) {
      sprintf(textBuf, "Tfreq:%d ", turbFreq);
      textStr += textBuf;
    }
    if (turbBase != -9) {
      sprintf(textBuf, "Tbase:%d ", turbBase);
      textStr += textBuf;
    }
    if (turbTop != -9) {
      sprintf(textBuf, "Ttop:%d ", turbTop);
      textStr += textBuf;
    }
    if (turbType != -9) {
      sprintf(textBuf, "Ttype:%d ", turbType);
      textStr += textBuf;
    }
  }
  
  if (skyCvg != -9) {
    sprintf(textBuf, "Sky:%d ", skyCvg);
    textStr += textBuf;
    if (skyBase != -9) {
      sprintf(textBuf, "Sbase:%d ", skyBase);
      textStr += textBuf;
    }
    if (skyTop != -9) {
      sprintf(textBuf, "Stop:%d ", skyTop);
      textStr += textBuf;
    }
  }

  if (textStr.size() > 0) {
    STRncopy(pirep.text, textStr.c_str(), MIN(textStr.size(), PIREP_TEXT_LEN));
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===== output pirep ============" << endl;
    pirep_print(stderr, "", &pirep);
  }
  
  // swap the struct
  
  BE_from_pirep(&pirep);
  
  // store in SPDB
  
  spdb.setPutMode(Spdb::putModeAddUnique);
  if (spdb.put(_params.output_url,
	       SPDB_PIREP_ID,
	       SPDB_PIREP_LABEL,
	       0,
	       reportTime,
	       reportTime + _params.valid_period,
	       sizeof(pirep_t),
	       &pirep)) {
    cerr << "ERROR - " << _progName << ":Run::_store()" << endl;
    cerr << "Cannot put pirep to spdb url: "
	 << _params.output_url << endl;
    return -1;
  }

  return 0;

}




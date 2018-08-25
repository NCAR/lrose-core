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
// GenPt2Spdb.cc
//
// GenPt2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
///////////////////////////////////////////////////////////////////////
//
// GenPt2Spdb reads an generic point data as ASCII and writes the
// data out to SPDB.
//
///////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cerrno>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <didss/DsInputPath.hh>
#include <rapformats/GenPt.hh>
#include <Spdb/DsSpdb.hh>
#include "GenPt2Spdb.hh"
using namespace std;

// Constructor

GenPt2Spdb::GenPt2Spdb(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "GenPt2Spdb";
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

GenPt2Spdb::~GenPt2Spdb()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int GenPt2Spdb::Run ()
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

int GenPt2Spdb::_parseInput (const string &inputFilePath)

{

  if (_params.debug) {
    cerr << "Parsing file:  " <<  inputFilePath << endl;
  }

  char line[BUFSIZ];
  FILE *in;
  
  if ((in = fopen(inputFilePath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - GenPt2Spdb::_parseInput" << endl;
    cerr << "  Pasring file: " << inputFilePath << endl;
    cerr << "  "  << strerror(errNum) << endl;
    return -1;
  }
  
  // SPDB output object

  DsSpdb spdb;

  while (!feof(in)) {

    // find start of record
    while (fgets(line, BUFSIZ, in) != NULL) {
      // cerr << line;
      if (strstr(line, "start-of-record") != NULL) {
	break;
      }
    }

    // read in the name line and strip the CR
    if (fgets(line, BUFSIZ, in) == NULL) {
      continue;
    }
    line[strlen(line)-1] = '\0';

    string name = line;
    cerr << "-----------------------" << endl;
    cerr << "name: " << name << endl;

    // read in the date and time

    date_time_t ttime;
    if (fscanf(in, "%d %d %d %d %d %d",
	       &ttime.year, &ttime.month, &ttime.day,
	       &ttime.hour, &ttime.min, &ttime.sec) != 6) {
      continue;
    }
	
    uconvert_to_utime(&ttime);

    cerr << "time: " << utimstr(ttime.unix_time) << endl;

    // read in lat/lon

    double lat, lon;
    if (fscanf(in, "%lf %lf", &lat, &lon) != 2) {
      continue;
    }
    cerr << "lat: " << lat << endl;
    cerr << "lon: " << lon << endl;

    // read in nfields and nlevels

    int nfields, nlevels;
    if (fscanf(in, "%d %d", &nfields, &nlevels) != 2) {
      continue;
    }
    cerr << "nfields: " << nfields << endl;
    cerr << "nlevels: " << nlevels << endl;

    // read in the field info string - read 2 lines, because
    // fscanf leaves a dangling carrriage return
    
    fgets(line, BUFSIZ, in);
    if (fgets(line, BUFSIZ, in) == NULL) {
      continue;
    }
    line[strlen(line)-1] = '\0';
    string fieldInfoStr = line;
    cerr << "FieldInfoStr: " << fieldInfoStr << endl;

    // read in the values

    vector<double> vals;
    double val;
    int nvals = nfields * nlevels;
    for (int i = 0; i < nvals; i++) {
      if (fscanf(in, "%lf", &val) != 1) {
	continue;
      }
      vals.push_back(val);
    }

    cerr << "Vals: ";
    for (size_t i = 0; i < vals.size(); i++) {
      cerr << vals[i] << " ";
    }
    cerr << endl;

    // read in the text string - read 2 lines, because
    // fscanf leaves a dangling carrriage return
    
    fgets(line, BUFSIZ, in);
    if (fgets(line, BUFSIZ, in) == NULL) {
      continue;
    }
    line[strlen(line)-1] = '\0';
    string textStr;
    if (strstr(line, "end-of-record") == NULL) {
      textStr = line;
    }
    cerr << "TextStr: " << textStr << endl;

    GenPt pt;
    pt.setName(name);
    pt.setTime(ttime.unix_time);
    pt.setLat(lat);
    pt.setLon(lon);
    pt.setNLevels(nlevels);
    for (size_t i = 0; i < vals.size(); i++) {
      pt.addVal(vals[i]);
    }
    if (pt.setFieldInfo(fieldInfoStr)) {
      cerr << "ERROR - GenPt2Spdb::_parseInput" << endl;
      continue;
    }
    pt.setText(textStr);

    cerr << "============= orig =============" << endl;
    pt.print(cerr);

    if (pt.assemble()) {
      cerr << "ERROR - GenPt2Spdb::_parseInput" << endl;
      continue;
    }

    // store in SPDB

    if (spdb.put(_params.output_url,
		 SPDB_GENERIC_POINT_ID,
		 SPDB_GENERIC_POINT_LABEL,
		 spdb.hash4CharsToInt32(name.c_str()),
		 ttime.unix_time,
		 ttime.unix_time + _params.valid_period,
		 pt.getBufLen(),
		 pt.getBufPtr())) {
      cerr << "WARNING - GenPt2Spdb::_parseInput" << endl;
      cerr << "  Cannot write to output url: " << _params.output_url << endl;
    }
		 
    //     GenPt newPt;
    //     if (newPt.disassemble(pt.getBufPtr(), pt.getBufLen())) {
    //       cerr << "ERROR - GenPt2Spdb::_parseInput" << endl;
    //       continue;
    //     }
    //     cerr << "============= new =============" << endl;
    //     newPt.print(cerr);

  }

  fclose(in);

  return 0;

}
  


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
// SpolDishTemp2Spdb.cc
//
// SpolDishTemp2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2016
//
///////////////////////////////////////////////////////////////
//
// SpolDishTemp2Spdb reads dish temp records from ASCII files,
// converts them to station_report_t format and
// writes them to an SPDB data base.
//
///////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/toolsa_macros.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaStr.hh>
#include <didss/DsInputPath.hh>
#include "SpolDishTemp2Spdb.hh"
using namespace std;

// Constructor

SpolDishTemp2Spdb::SpolDishTemp2Spdb(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "SpolDishTemp2Spdb";
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
    return;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

SpolDishTemp2Spdb::~SpolDishTemp2Spdb()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int SpolDishTemp2Spdb::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // file input object
  
  DsInputPath *input = NULL; // Set to NULL to get around compiler warnings.
  
  if (_params.mode == Params::FILELIST) {
    
    // FILELIST mode
    
    input = new DsInputPath(_progName,
			    _params.debug >= Params::DEBUG_VERBOSE,
			    _args.inputFileList);
    
  } else if (_params.mode == Params::ARCHIVE) {
    
    // archive mode - start time to end time
    
    input = new DsInputPath(_progName,
			    _params.debug >= Params::DEBUG_VERBOSE,
			    _params.input_dir,
			    _args.startTime, _args.endTime);
    
  } else if (_params.mode == Params::REALTIME) {
    
    // realtime mode - no latest_data_info file
    
    input = new DsInputPath(_progName,
			    _params.debug >= Params::DEBUG_VERBOSE,
			    _params.input_dir,
			    _params.max_realtime_valid_age,
			    PMU_auto_register,
			    _params.latest_data_info_avail,
			    true);
    
    if (_params.file_name_check) {
      input->setSubString(_params.file_match_string);
    }
    
  }
  
  // loop through available files
  
  char *inputPath;
  while ((inputPath = input->next()) != NULL) {
    if (_processFile(inputPath)) {
      cerr << "WARNING - SpolDishTemp2Spdb::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
    }
  } // while

  delete input;
    
  return 0;

}

////////////////////
// process the file

int SpolDishTemp2Spdb::_processFile(const char *file_path)
  
{
  
  if (_params.debug) {
    cerr << "Processing file: " << file_path << endl;
  }

  // registration

  char procmapString[BUFSIZ];
  Path path(file_path);
  sprintf(procmapString, "Processing file <%s>", path.getFile().c_str());
  PMU_force_register(procmapString);

  // create output objects

  DsSpdb spdbDecoded;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    spdbDecoded.setDebug();
  }

  // Open the file

  FILE *fp;
  if((fp = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SpolDishTemp2Spdb::_processFile" << endl;
    cerr << "  Cannot open metar file: "
	 << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
   
  char line[BUFSIZ];
  int iret = 0;
  int idIndex = -1;
  int timeIndex = -1;
  int valIndex = -1;

  // read in a line at a time

  while(fgets(line, BUFSIZ, fp) != NULL) {

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << line;
    }

    // tokenize the line
    
    vector<string> toks;
    TaStr::tokenize(line, "\",\n\r", toks);
    if (toks.size() < 4) {
      break;
    }

    // set the index for each field

    bool isLabelLine = false;
    for (size_t ii = 0; ii < toks.size(); ii++) {
      cerr << "ii, tok: " << ii << ", " << toks[ii] << endl;
      if (toks[ii] == "SensorID") {
        idIndex = ii;
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "idIndex: " << idIndex << endl;
        }
        isLabelLine = true;
      }
      if (toks[ii] == "Date") {
        timeIndex = ii;
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "timeIndex: " << timeIndex << endl;
        }
        isLabelLine = true;
      }
      if (toks[ii] == "Value") {
        valIndex = ii;
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "valIndex: " << valIndex << endl;
        }
        isLabelLine = true;
      }
    }
    if (isLabelLine) {
      continue;
    }

    // decode the tokens

    int stationId;
    if (sscanf(toks[idIndex].c_str(), "%d", &stationId) != 1) {
      cerr << "ERROR - cannot decode ID: " << toks[idIndex] << endl;
      continue;
    }

    double value;
    if (sscanf(toks[valIndex].c_str(), "%lg", &value) != 1) {
      cerr << "ERROR - cannot decode value: " << toks[valIndex] << endl;
      continue;
    }

    int year, month, day, hour, min, sec;
    char ampm[16];
    if (sscanf(toks[timeIndex].c_str(), "%d/%d/%d %d:%d:%d %s",
               &year, &month, &day, &hour, &min, &sec, ampm) != 7) {
      cerr << "ERROR - cannot decode datetime: " << toks[timeIndex] << endl;
      continue;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << line;
    }

#ifdef JUNK
    
  StationLoc &stationLoc = iloc->second;

    if (complete) {
      
      MemBuf buf;
      time_t valid_time;
      string stationName;

      if (_decodeMetar(file_path, file_time, blockHour, blockMin, blockDate,
		       metarMessage, reportType, stationName, buf, valid_time) == 0) {

	// add chunks to spdb objects

	int stationId = Spdb::hash4CharsToInt32(stationName.c_str());

	if (_params.write_decoded_metars) {
	  spdbDecoded.addPutChunk(stationId,
				  valid_time,
				  valid_time + _params.expire_seconds,
				  buf.getLen(), buf.getPtr());
	}
      
	if (_params.write_ascii_metars) {

	  if (_params.dress_raw_metar_text) {
	    metarMessage = reportType + " " + metarMessage;
	  }

	  spdbAscii.addPutChunk(stationId,
				valid_time,
				valid_time + _params.expire_seconds,
				metarMessage.size() + 1,
				metarMessage.c_str());
	}
      } /* endif - _decodeMetar(...) == 0) */

      metarMessage = "";
    } 

  WxObs obs;
  if (obs.setFromDecodedMetar(metarText, stationName, dcdMetar, valid_time,
                              lat, lon, alt)) {
    return -1;
  }

  // Dress the raw text with the report type and ending character (=)

  if (_params.dress_raw_metar_text) {
    obs.dressRawMetarText(reportType);
  }

  if (_params.output_report_type == Params::REPORT_PLUS_METAR_XML) {
    obs.assembleAsReport(REPORT_PLUS_METAR_XML);
  } else if (_params.output_report_type == Params::REPORT_PLUS_FULL_XML) {
    obs.assembleAsReport(REPORT_PLUS_FULL_XML);
  } else if (_params.output_report_type == Params::XML_ONLY) {
    obs.assembleAsXml();
  } else {
    return -1;
  }
  
  buf.add(obs.getBufPtr(), obs.getBufLen());

    if (spdbDecoded.put(_params.decoded_output_url,
                        SPDB_STATION_REPORT_ID,
                        SPDB_STATION_REPORT_LABEL)) {
      cerr << "ERROR - SpolDishTemp2Spdb::_doPut" << endl;
      cerr << "  Cannot put decoded metars to: "
           << _params.decoded_output_url << endl;
      cerr << "  " << spdbDecoded.getErrStr() << endl;
      iret = -1;
    }
    spdbDecoded.clearPutChunks();

#endif
    
  } // while (fgets ...
  
  fclose(fp);

  // write the output

  // if (_doPut(spdbDecoded, spdbAscii)) {
  //   iret = -1;
  // }
  
  return iret;
   
}



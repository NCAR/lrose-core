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
// Ude2Spdb.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2005
//
///////////////////////////////////////////////////////////////
//
// Ude2Spdb reads LTG records from ASCII files, converts them to
// LTG_strike_t format (rapformats library) and stores them in SPDB.
//
////////////////////////////////////////////////////////////////

#include <cerrno>

#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <didss/DsInputPath.hh>
#include <Spdb/LtgSpdbBuffer.hh>

#include "Ude2Spdb.hh"

using namespace std;

// Constructor

Ude2Spdb::Ude2Spdb(int argc, char **argv)
{
  isOK = true;
  _startTime = 0L;
  _currentTime = 0L;

  // set program name

  _progName = "Ude2Spdb";
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

Ude2Spdb::~Ude2Spdb()
{
  // unregister process
  PMU_auto_unregister();

  return;
}

//////////////////////////////////////////////////
// Run

int Ude2Spdb::Run ()
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
    
    if (_params.strict_subdir_check) {
      input->setStrictDirScan(true);
    }
    
    if (_params.file_name_check) {
      input->setSubString(_params.file_match_string);
    }
    
  }
  
  // loop through available files
  
  char *inputPath;
  while ((inputPath = input->next()) != NULL) {
    
    if (_processFile(inputPath)) {
      cerr << "WARNING - Ude2Spdb::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
    }
    
  } // while
    
  return 0;
}

////////////////////
// process the file

int Ude2Spdb::_processFile(const char *file_path)
{
  if (_params.debug) {
    cerr << "Processing file: " << file_path << endl;
  }

  // registration
  
  char procmapString[BUFSIZ];
  Path path(file_path);
  sprintf(procmapString, "Processing file <%s>", path.getFile().c_str());
  PMU_force_register(procmapString);

  // Open the file
  
  FILE *fp;
  if((fp = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Ude2Spdb::_processFile" << endl;
    cerr << "  Cannot open metar file: "
	 << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
   
  // read in line-by-line

  int iret = 0;
  char line[BUFSIZ];
  while( fgets(line, BUFSIZ, fp) != 0)
  {
    GenPt gen_pt;
    
    if (!_decodeLine(line, gen_pt))
      continue;
    
    // write if ready

    if (!_write2Spdb(gen_pt))
    {
      iret = -1;
    }

  } // while (fgets ...

  fclose(fp);
 
  return iret;
   
}

///////////////////////////////////////////
// decode the ltg strike from the data line
// format type 1
//
// NOTE: this fills out LTG_strike_t
  
bool Ude2Spdb::_decodeLine(const char *line,
			   GenPt &gen_pt)
{
  static const string method_name = "Ude2Spdb::_decodeLine()";
  
  int data_day, data_time;
  double lat, lon;
  int altitude;
  double ude;
  
  if (sscanf(line, "%d %d %lg %lg %d %lg",
             &data_day, &data_time, &lat, &lon, &altitude, &ude) != 6)
  {
    if (_params.debug >= Params::DEBUG_VERBOSE)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "  Cannot decode line: " << line << endl;
      cerr << "  Expecting: day time lat lon alt ude" << endl;
    }
    return false;
  }
  
  if (altitude < 30000 || altitude > 40000)
    return false;
  
  // Convert the time information to our internal format

  int year = data_day / 10000;
  data_day -= (year * 10000);
  int month = data_day / 100;
  data_day -= (month * 100);
  int day = data_day;
  
  int hour = data_time / 100;
  data_time -= (hour * 100);
  int minute = data_time;
  
  DateTime ude_time(year, month, day, hour, minute);
  
  // Set point information
  
  gen_pt.clear();
  gen_pt.setTime(ude_time.utime());
  gen_pt.setLat(lat);
  gen_pt.setLon(lon);
  
  gen_pt.addFieldInfo("altitude", "ft");
  gen_pt.addVal(altitude);
  
  gen_pt.addFieldInfo("ude", "none");
  gen_pt.addVal(ude);
  
  return true;
}

///////////////////////////////////////////
// write output if we are ready to do so

bool Ude2Spdb::_write2Spdb(GenPt &gen_pt)
{
  static const string method_name = "Ude2Spdb::_write2Spdb()";
  
  // Get the data ready for writing
  
  if (gen_pt.assemble() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error assembling GenPt for writing to SPDB database" << endl;
    
    return false;
    
  }
  
  // Add the point to the database

  _spdb.addPutChunk(0,
		    gen_pt.getTime(),
		    gen_pt.getTime() + _params.expire_seconds,
		    gen_pt.getBufLen(),
		    gen_pt.getBufPtr());
  
  if (_spdb.put(_params.output_url,
		SPDB_GENERIC_POINT_ID,
		SPDB_GENERIC_POINT_LABEL) != 0)
  {
    return false;
  }
  
  return true;
}



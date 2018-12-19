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
/**
 * @file GenPoly2Spdb.cc
 */
#include "GenPoly2Spdb.hh"
#include <Spdb/DsSpdb.hh>
#include <rapformats/GenPoly.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>


//---------------------------------------------------------------------------
GenPoly2Spdb::GenPoly2Spdb(int argc, char **argv)
{
  isOK = true;

  // set programe name
  _progName = argv[0];
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName))
  {
    LOG(ERROR) << "Problem with command line args";
    isOK = FALSE;
    return;
  }

  // get TDRP params
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list, &_paramsPath))
  {
    LOG(ERROR) << "Problem with TDRP parameters";
    isOK = FALSE;
    return;
  }

  // init process mapper registration
  PMU_auto_init((char *) _progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);

  bool debug, debugVerbose, realtime=true, showFile=true;

  debug = _params.debug >= Params::DEBUG_NORM;
  debugVerbose = _params.debug >= Params::DEBUG_VERBOSE;
  LOG_STREAM_INIT(debug, debugVerbose, realtime, showFile);

  if (_params.restrict_time_range)
  {
    DateTime dt0(_params._time_limits[0].year,
		 _params._time_limits[0].month,
		 _params._time_limits[0].day,
		 _params._time_limits[0].hour,
		 _params._time_limits[0].min,
		 _params._time_limits[0].sec);
    _time0 = dt0.utime();
    DateTime dt1(_params._time_limits[1].year,
		 _params._time_limits[1].month,
		 _params._time_limits[1].day,
		 _params._time_limits[1].hour,
		 _params._time_limits[1].min,
		 _params._time_limits[1].sec);
    _time1 = dt1.utime();
  }
  else
  {
    _time0 = 0;
    _time1 = 0;
  }
  return;

}

//---------------------------------------------------------------------------
GenPoly2Spdb::~GenPoly2Spdb()
{
  PMU_auto_unregister();
}

//---------------------------------------------------------------------------
int GenPoly2Spdb::Run ()
{
  int iret = 0;

  // register with procmap
  PMU_auto_register("Run");

  // input file object
  DsInputPath *input = _setInput();

  char *inputFilePath;
  if (_params.mode == Params::ARCHIVE && _params.debug)
  {
    input->reset();
    LOG(DEBUG) << "ARCHIVE mode, Nfiles: " << _args.filePaths.size()
	       << " Files:";
    while ((inputFilePath = input->next()) != NULL)
    {
      LOG(DEBUG) << "  " <<  inputFilePath;
    }
  }
  input->reset();

  int i = 1;
  while ((inputFilePath = input->next()) != NULL)
  {
    if (_parseInput(inputFilePath, i++))
    {
      LOG(ERROR) << "parsing file: " << inputFilePath;
      iret = -1;
      break;
    }
  }
  delete input;
  return iret;
}

//---------------------------------------------------------------------------
DsInputPath *GenPoly2Spdb::_setInput(void) const
{  
  if (_params.mode == Params::ARCHIVE)
  {
    return new DsInputPath((char *) _progName.c_str(),
			   _params.debug >= Params::DEBUG_VERBOSE,
			   _args.filePaths);
  }
  else
  {
    return new DsInputPath((char *) _progName.c_str(),
			   _params.debug >= Params::DEBUG_VERBOSE,
			   _params.input_dir, _params.max_realtime_valid_age,
			    PMU_auto_register);
  }
}

//---------------------------------------------------------------------------
int GenPoly2Spdb::_parseInput (const string &inputFilePath, int index)
{
  int year, month, day;
  FILE *in;

  if (!_parseInit(inputFilePath, year, month, day))
  {
    return -1;
  }

  if ((in = fopen(inputFilePath.c_str(), "r")) == NULL)
  {
    int errNum = errno;
    LOG(ERROR) << "Could not open file " << inputFilePath;
    LOG(ERROR) << strerror(errNum);
    return -1;
  }
  
  // SPDB output object
  DsSpdb spdb;
  time_t time0, time1;
  bool first = true;
  GenPoly pt;
  while (!feof(in))
  {
    _parseInputLine(in, year, month, day, first, pt, time0, time1);
  }
  fclose(in);

  pt.setExpireTime(time1 + _params.valid_period);
  if (_params.debug >= Params::DEBUG_VERBOSE)
  {
    pt.print(stdout);
  }

  if (!pt.assemble())
  {
    LOG(ERROR) << "Could not assemble object";
    return -1;
  }

  // store in SPDB
  if (spdb.put(_params.output_url,
	       SPDB_GENERIC_POLYLINE_ID,
	       SPDB_GENERIC_POLYLINE_LABEL,
	       index,
	       time0,
	       time1 + _params.valid_period,
	       pt.getBufLen(),
	       pt.getBufPtr()))
  {
    LOG(WARNING) << "  Cannot write to output url: " << _params.output_url;
  }
  return 0;
}
  


bool GenPoly2Spdb::_parseInit(const string &inputFilePath, int &year,
			      int &month, int &day)
{
  LOG(DEBUG) <<"Parsing file:  " <<  inputFilePath;

  string fileName = inputFilePath;

  // parse the ymd from the file name, first strip off any path stuff
  std::size_t f = inputFilePath.find_last_of("/");
  if (f != std::string::npos)
  {
    fileName = inputFilePath.substr(f+1);
  }

  if (sscanf(fileName.c_str(), "%04d%02d%02d", &year, &month, &day) != 3)
  {
    LOG(ERROR) << "parsing date out of string " << fileName;
    return false;
  }
  
  return true;
}

void GenPoly2Spdb::_parseInputLine(FILE *in, int year, int month, int day,
				   bool &first, GenPoly &pt, time_t &time0,
				   time_t &time1)
{
  char line[BUFSIZ];
  // read in the name line and strip the CR
  if (fgets(line, BUFSIZ, in) == NULL)
  {
    return;
  }
  line[strlen(line)-1] = '\0';

  string name = line;
  LOG(DEBUG_VERBOSE) << "Read: " << name;

  int hour, min, sec;
  double lat, lon, alt;
  if (sscanf(line, "%02d%02d%02d %lf %lf %lf", &hour, &min, &sec,
	     &lat, &lon, &alt) != 6)
  {
    LOG(ERROR) << "parsing as hhmmss  lat  lon alt" << name;
    return;
  }
  DateTime dt(year, month, day, hour, min, sec);

  if (!_inRange(dt.utime()))
  {
    return;
  }

  if (first)
  {
    first = false;
    time0 = time1 = dt.utime();
    pt.setName("flightpathdata");
    pt.setTime(time0);
    pt.setNLevels(1);
    pt.setClosedFlag(false);
    pt.clearVertices();
  }
  else
  {
    time1 = dt.utime();
  }
  GenPoly::vertex_t vertex;
  vertex.lat = lat;
  vertex.lon = lon;
  pt.addVertex(vertex);
}

bool GenPoly2Spdb::_inRange(const time_t &t) const
{
  if (_params.restrict_time_range)
  {
    return (t >= _time0  && t <= _time1);
  }
  else
  {
    return true;
  }
}

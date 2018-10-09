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
// ArgentineAwsIngest.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2018
//
////////////////////////////////////////////////////////////
// Reads in CSV-formatted AWS data for Argentina
// Stores in SPDB and NetCDF
////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/toolsa_macros.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/TaStr.hh>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <rapformats/WxObs.hh>
#include <rapformats/station_reports.h>
#include <Spdb/DsSpdb.hh>
#include "ArgentineAwsIngest.hh"
using namespace std;

/////////////////////////////////////////////////////////////////
// Constructor

ArgentineAwsIngest::ArgentineAwsIngest(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "ArgentineAwsIngest";
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

}

/////////////////////////////////////////////////////////////////
// destructor

ArgentineAwsIngest::~ArgentineAwsIngest()
{
  PMU_auto_unregister();
}

/////////////////////////////////////////////////////////////////
// Run

int ArgentineAwsIngest::Run ()
{

  // register with procmap

  PMU_auto_register("Run");

  // create file input object

  DsInputPath *input = NULL; 
  
  if (_params.mode == Params::FILELIST) {
    // FILELIST mode
    input = new DsInputPath(_progName,
                            _params.debug >= Params::DEBUG_VERBOSE,
                            _args.inputFileList);
  } else if (_params.mode == Params::REALTIME) {
    // realtime mode
    input = new DsInputPath(_progName,
                            _params.debug >= Params::DEBUG_VERBOSE,
                            _params.input_dir,
                            _params.max_realtime_valid_age,
                            PMU_auto_register,
                            _params.latest_data_info_avail,
                            false);
    
  }

  // loop through available files
  
  char *inputPath;
  while ((inputPath = input->next()) != NULL) {
    if (_processFile(inputPath)) {
      cerr << "WARNING - ArgentineAwsIngest::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
    }
    
  } // while
  
  return 0;

}

/////////////////////////////////////////////////////////////////
//  process the file

int ArgentineAwsIngest::_processFile(const char *file_path)
  
{

  if (_params.debug) {
    cerr << "Processing file: " << file_path << endl;
  }
  
  // registration with procmap

  char procmapString[BUFSIZ];
  Path path(file_path);
  snprintf(procmapString, BUFSIZ, "Processing file <%s>", path.getFile().c_str());
  PMU_force_register(procmapString);

  // Open the file

  FILE *fp;
  if((fp = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - ArgentineAwsIngest::_processFile" << endl;
    cerr << "  Cannot open AWS file: "
         << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // Read each line
  
  char line[BUFSIZ];

  // read first line - labels
  fgets(line, BUFSIZ, fp);

  // read data lines
  vector<WxObs *> observations;
  while(fgets(line, BUFSIZ, fp) != NULL) {
    
    // tokenize the line
    
    vector<string> toks;
    TaStr::tokenize(line, ",", toks);

    if ((int) toks.size() != _params.input_fields_n) {
      cerr << "============================================" << endl;
      cerr << "WARNING - bad line: " << line << endl;
      cerr << "  nFields found: " << toks.size() << endl;
      cerr << "  should be: " << _params.input_fields_n << endl;
      continue;
    }

    // fill out obs

    WxObs *obs = new WxObs;

    for (int ifield = 0; ifield < _params.input_fields_n; ifield++) {

      Params::csv_field_t ftype = _params._input_fields[ifield];
      string tok = toks[ifield];
      
      switch (ftype) {
        case Params::FIELD_LATITUDE:
          obs->setLatitude(_decodeFloatField(tok));
          break;
        case Params::FIELD_LONGITUDE:
          obs->setLongitude(_decodeFloatField(tok));
          break;
        case Params::FIELD_ALTITUDE:
          obs->setElevationM(_decodeFloatField(tok));
          break;
        case Params::FIELD_STATION_CODE:
          obs->setStationId(_decodeStationCode(tok));
          break;
        case Params::FIELD_TIME_UTC:
          obs->setObservationTime(_decodeTimeField(tok));
          break;
        case Params::FIELD_TEMPERATURE_C:
          obs->setTempC(_decodeFloatField(tok));
          break;
        case Params::FIELD_RH_PERCENT:
          obs->setRhPercent(_decodeFloatField(tok));
          break;
        case Params::FIELD_DEWPOINT_C:
          obs->setDewpointC(_decodeFloatField(tok));
          break;
        case Params::FIELD_WIND_VEL_KMPH:
          obs->setWindSpeedMps(_decodeFloatField(tok) / MPERSEC_TO_KMPERHOUR);
          break;
        case Params::FIELD_WIND_DIRN_DEGT:
          obs->setWindDirnDegT(_decodeFloatField(tok));
          break;
        case Params::FIELD_PRESSURE_HPA:
          obs->setPressureMb(_decodeFloatField(tok));
          break;
        case Params::FIELD_PRECIP_10MIN_MM:
          obs->addPrecipLiquidMm(_decodeFloatField(tok), 600);
          break;
        case Params::FIELD_PRECIP_1HR_MM:
          obs->addPrecipLiquidMm(_decodeFloatField(tok), 3600);
          break;
        case Params::FIELD_RAIN_4HR_MM:
          obs->addPrecipLiquidMm(_decodeFloatField(tok), 4 * 3600);
          break;
        case Params::FIELD_RAIN_6HR_MM:
          obs->addPrecipLiquidMm(_decodeFloatField(tok), 6 * 3600);
          break;
        default:
          {};
      } // switch
      
    } // ifield

    observations.push_back(obs);

    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "===>> Adding observation <<===" << endl;
      obs->print(cerr);
    }

  } // while (fgets ...
  
  fclose(fp);       

  // write the observations

  int iret = _writeObs(observations);

  // clean up

  for (size_t ii = 0; ii < observations.size(); ii++) {
    delete observations[ii];
  }
  observations.clear();

  return iret;
   
}

///////////////////////////////////////////////////////////
// decode a floating point value
// return NAN on error

double ArgentineAwsIngest::_decodeFloatField(const string &str)

{
  
  if (str == "NaN") {
    return NAN;
  }

  double val;
  if (sscanf(str.c_str(), "%lg", &val) == 1) {
    return val;
  }

  return NAN;

}

///////////////////////////////////////////////////////////
// decode a time
// return -1 on error

time_t ArgentineAwsIngest::_decodeTimeField(const string &str)
  
{
  if (str.size() != 14) {
    return -1;
  }
  DateTime dtime(str);
  return dtime.utime();
}

///////////////////////////////////////////////////////////
// decode station code
// strip off quotes

string ArgentineAwsIngest::_decodeStationCode(const string &str)

{
  
  string code;
  for (size_t ii = 0; ii < str.size(); ii++) {
    if (isalnum(str[ii])) {
      code.append(1, str[ii]);
    }
  }
  return code;

}

////////////////////////////////////////////////////////////
// write observations


int ArgentineAwsIngest::_writeObs(vector<WxObs *> &observations)

{

  DsSpdb spdb;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    spdb.setDebug();
  }
  spdb.setPutMode(Spdb::putModeOver);

  for (size_t ii = 0; ii < observations.size(); ii++) {
    WxObs *obs = observations[ii];
    obs->assembleAsXml();
    string idStr = obs->getStationId();
    int idLen = idStr.size();
    int startIndex = idLen - 4;
    if (startIndex < 0) {
      startIndex = 0;
    }
    int stationId = Spdb::hash4CharsToInt32(idStr.c_str() + startIndex);
    time_t validTime = obs->getObservationTime();
    spdb.addPutChunk(stationId,
                     validTime,
                     validTime + _params.spdb_expire_seconds,
                     obs->getBufLen(), obs->getBufPtr());
  } // ii
  
  // Write report to database
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "\nWriting obs to " << _params.spdb_output_url << endl;
    cerr << "--------------------------------------------------\n\n";
    
  }
  
  if (spdb.put(_params.spdb_output_url,
               SPDB_STATION_REPORT_ID,
               SPDB_STATION_REPORT_LABEL) != 0) {
    cerr << "ERROR - ArgentineAwsIngest::_processFile" << endl;
    cerr << "  Cannot put aws report to: "
         << _params.spdb_output_url << endl;
    cerr << "  " << spdb.getErrStr() << endl;
    return(1);
  }
  
  return 0;
  
}












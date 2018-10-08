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
// Ltg2Spdb.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2005
//
///////////////////////////////////////////////////////////////
//
// Ltg2Spdb reads LTG records from ASCII and netcdf files, converts them to
// LTG_strike_t format (rapformats library) and stores them in SPDB.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <string>
#include <Ncxx/Nc3File.hh>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/pjg_flat.h>
#include <dataport/bigend.h>
#include <euclid/PjgFlatCalc.hh>

#include "Ltg2Spdb.hh"

using namespace std;

// Constructor

Ltg2Spdb::Ltg2Spdb(int argc, char **argv)

{

  isOK = true;
  _startTime = 0L;
  _latestTime = 0L;
  _writeTime = time(NULL);
  _dataId = 0;
  _nStrikes = 0;

  _lastLat = 0.0;
  _lastLon = 0.0;
  _lastTime = 0L;


  // set program name

  _progName = "Ltg2Spdb";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *)"unknown";
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

// destructor

Ltg2Spdb::~Ltg2Spdb()

{

  // unregister process

  PMU_auto_unregister();

  // Write any strikes in the buffer out to SPDB and exit.
  
  _startTime = _latestTime;
  _doWrite();

}

//////////////////////////////////////////////////
// Run

int Ltg2Spdb::Run ()
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
      cerr << "WARNING - Ltg2Spdb::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
    }
    
  } // while
    
  return 0;

}

////////////////////
// process the file

int Ltg2Spdb::_processFile(const char *file_path)
  
{

  if (_params.debug) {
    cerr << "Processing file: " << file_path << endl;
  }

  // registration
  
  char procmapString[BUFSIZ];
  Path path(file_path);
  sprintf(procmapString, "Processing file <%s>", path.getFile().c_str());
  PMU_force_register(procmapString);

  if (_params.mode == Params::REALTIME){
    for (int id=0; id < _params.delay_before_processing; id++){
      sleep(1);
      PMU_force_register(procmapString);
    }
  }

  // special case for binary data

  if (_params.input_format == Params::NLDN_BINARY) {
    return _processNldnFile(file_path);
  }

  // special case for netcdf data
  if (_params.input_format == Params::AOAWS_NETCDF){
	  return _processAOAWSNetCDFFile(file_path);
  }
  // Open the file
  
  FILE *fp;
  if((fp = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Ltg2Spdb::_processFile" << endl;
    cerr << "  Cannot open ltg file: "
	 << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
   
 

  // read in line-by-line

  unsigned long nLines = 0L;
  int iret = 0;
  char line[BUFSIZ];
  while( fgets(line, BUFSIZ, fp) != NULL ) {

    nLines++;

    PMU_auto_register("processing strikes");

    // discard comments

    if (line[0] == '#') {
      continue;
    }

    // NAPLN data has lines that start with
    // LIGHTNING-NAPLN1EX
    if (line[0] == 'L') {
      continue;
    }

    switch (_params.input_format) {

      case Params::FORMAT_1: {
        _decode_format_1(line);
        break;
      }

      case Params::FORMAT_2: {
        _decode_format_2(line);
        break;
      }

      case Params::FORMAT_3: {
        _decode_format_3(line);
        break;
      }

      case Params::FORMAT_4: {
        _decode_format_4(line);
        break;
      }

      case Params::UALF_LF_1: {
        _decode_ualf_lf_1(line);
        break;
      }

      case Params::UALF_LF_1B: {
        _decode_ualf_lf_1b(line);
        break;
      }

      case Params::BOM_AXF: {
        _decode_bom_axf(line);
        break;
      }

      case Params::WWLDN: {
	_decode_wwldn(line);
	break;
      }
    
      case Params::WWLLN: {
	_decode_wwlln(line);
	break;
      }
    
      case Params::NAPLN: {
	_decode_napln(line);
	break;
      }

     case Params::KSC: {
       _decode_ksc(line, file_path);
        break;
      }

    case Params::ALBLM: {
       _decode_alblm(line);
        break;
    }

    
      default: {}

    } // switch

    // write if ready

    if (_writeIfReady()) {
      iret = -1;
    }

  } // while (fgets ...

  fclose(fp);
 
  if (_params.flush_buffer_per_file) {
    _startTime = _latestTime;
    if (_doWrite()) {
      iret = -1;
    }
  }

  if (_params.debug){
    cerr << endl;
    cerr << nLines << " lines read from " << file_path << endl;
  }

  return iret;
   
}

/////////////////////////
// process an AOAWS NetCDF file

int Ltg2Spdb::_processAOAWSNetCDFFile(const char *file_path)
{
	const string methodName = "Ltg2Spdb::_processAOAWSNetCDFFile";

	int iret = 0;
	Nc3File ncfFile(file_path);

	if(!ncfFile.is_valid()) {
		cerr << "ERROR - " << methodName << endl;
		cerr << file_path << " is not a valid netcdf file." << endl;
		return -1;
	}

	// declare an error object

	Nc3Error err(Nc3Error::silent_nonfatal);


	// check that this is a valid file

	if(_checkAOAWSNetCDFFile(ncfFile) == false) {
		cerr << "ERROR - " << methodName << endl;
		cerr << "  Not a valid NetCDF file" << endl;
		cerr << "  File: " << file_path << endl;
		return -1;
	}

	size_t nRecords = ncfFile.get_dim(_params.netcdf_strike_num_dim)->size();

	if (_params.debug) {
		cerr << "Opened NetCDF file: " << file_path << " for reading.\n";
		cerr << "\t" << nRecords << " records to read\n";
	}

  Nc3Var *latVar = ncfFile.get_var(_params.netcdf_lat_var);
  Nc3Var *lonVar = ncfFile.get_var(_params.netcdf_lon_var);
  Nc3Var *timeVar = ncfFile.get_var(_params.netcdf_time_var);

  vector<float> latVector;
  vector<float> lonVector;
  vector<float> timeVector;

  latVector.reserve(nRecords);
  float *latPtr = (float *) &latVector[0];
  if (latVar->get(latPtr, nRecords) == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  Cannot get latitudes from input netcdf variable" << endl;
    return -1;
  }

  lonVector.reserve(nRecords);
  float *lonPtr = (float *) &lonVector[0];
  if (lonVar->get(lonPtr, nRecords) == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  Cannot get longitudes from input netcdf variable" << endl;
    return -1;
  }

  timeVector.reserve(nRecords);
  float *timePtr = (float *) &timeVector[0];
  if (timeVar->get(timePtr, nRecords) == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  Cannot get time from input netcdf variable" << endl;
    return -1;
  }

  for (size_t ix = 0; ix < nRecords; ix++) {
	 
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "============= strike ============" << endl;
      cerr << "  timeSecs: " << DateTime::strm(timeVector[ix]) << endl;
      cerr << "  lat: " << latVector[ix] << endl;
      cerr << "  lon: " << lonVector[ix] << endl;
    }

    LTG_extended_t strike;
    LTG_init_extended(&strike);
    strike.time = timeVector[ix];
    strike.latitude = latVector[ix];
    strike.longitude = lonVector[ix];

    // Check for duplicates
    bool useStrike = true;
    if (_params.duplicates.check){
	    if ( _checkNearDuplicate( strike )){
		    if (_params.debug >= Params::DEBUG_VERBOSE) {
			    cerr << "Strike for " << strike.latitude << ", ";
			    cerr << strike.longitude << " at " << utimstr(strike.time);
			    cerr << " rejected as near duplicate." << endl;
		    }
		    useStrike = false;
	    }
    }


    if (_params.debug >= Params::DEBUG_VERBOSE) {
	    LTG_print_extended(stderr, &strike);
    }
   
    if (useStrike) {
      _addStrike(strike);
    }
  }

  ncfFile.close();

  // write if ready

  if (_writeIfReady()) {
	  iret = -1;
  }

  if (_params.flush_buffer_per_file) {
    _startTime = _latestTime;
    if (_doWrite()) {
      iret = -1;
    }
  }

  return iret;
}

int Ltg2Spdb::_checkAOAWSNetCDFFile(const Nc3File &ncf)
{

  const string methodName = "Ltg2Spdb::_checkAOAWSNetCDFFIle";
	
  int errorCount = 0;
  if (ncf.get_var(_params.netcdf_lat_var) == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  attribute missing: " << _params.netcdf_lat_var << endl;
    errorCount++;
  }
  if (ncf.get_var(_params.netcdf_lon_var) == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  attribute missing: " << _params.netcdf_lon_var << endl;
    errorCount++;
  }
  
  if (ncf.get_var(_params.netcdf_time_var) == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  attribute missing: " << _params.netcdf_time_var << endl;
    errorCount++;
  }  

  if (ncf.get_dim(_params.netcdf_strike_num_dim) == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  attribute missing: " << _params.netcdf_strike_num_dim << endl;
    errorCount++;
  }  

  if(errorCount > 0) {
    return false;
  }
  else {
    return true;
  }

}

/////////////////////////
// process an NLDN file

int Ltg2Spdb::_processNldnFile(const char *file_path)
  
{

  // Open the file

  int iret = 0;
  FILE *fp;
  if((fp = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Ltg2Spdb::_processNldnFile" << endl;
    cerr << "  Cannot open ltg file: "
	 << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // read in the header

  nldn_bin_header_t hdr;
  if (fread(&hdr, sizeof(hdr), 1, fp) != 1) {
    int errNum = errno;
    cerr << "ERROR - Ltg2Spdb::_processNldnFile" << endl;
    cerr << "  Cannot read in header, file: "
	 << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    fclose(fp);
    return -1;
  }

  char label[8];
  MEM_zero(label);
  memcpy(label, hdr.nldn, 4);
  int nHead = BE_to_si32(hdr.nhead);
  int nbytesExtra = nHead * 28 - sizeof(hdr);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  file label: " << label << endl;
    cerr << "  nHead: " << nHead << endl;
    cerr << "  nbytesExtra: " << nbytesExtra << endl;
  }

  // read in the extra bytes

  TaArray<char> extra_;
  char *extra = extra_.alloc(nbytesExtra);
  if (fread(extra, nbytesExtra, 1, fp) != 1) {
    int errNum = errno;
    cerr << "ERROR - Ltg2Spdb::_processNldnFile" << endl;
    cerr << "  Cannot read in rest of header, file: "
	 << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    fclose(fp);
    return -1;
  }

  // read in the records

  while (!feof(fp)) {

    nldn_bin_record_t rec;
    if (fread(&rec, 28, 1, fp) != 1) {
      break;
    }

    time_t timeSecs = BE_to_si32(rec.tsec);
    int fracSecs = (double) BE_to_si32(rec.nsec) / 1.0e9;
    double lat = (double) BE_to_si32(rec.lat) / 1000.0;
    double lon = (double) BE_to_si32(rec.lon) / 1000.0;
    double kAmps = ((double) BE_to_si16(rec.sgnl) / 10.0) * (30.0 / 150.0);
    int nStrokesPerFlash = rec.mult;
    double semiMajorAxis = rec.semimaj;
    double eccent = rec.eccent;
    double angle = rec.angle;
    double chiSq = rec.chisqr;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "============= strike ============" << endl;
      cerr << "  timeSecs: " << DateTime::strm(timeSecs) << endl;
      cerr << "  fracSecs: " << fracSecs << endl;
      cerr << "  lat: " << lat << endl;
      cerr << "  lon: " << lon << endl;
      cerr << "  kAmps: " << kAmps << endl;
      cerr << "  nStrokesPerFlash: " << nStrokesPerFlash << endl;
      cerr << "  semiMajorAxis: " << semiMajorAxis << endl;
      cerr << "  eccent: " << eccent << endl;
      cerr << "  angle: " << angle << endl;
      cerr << "  chiSq: " << chiSq << endl;
    }

    LTG_extended_t strike;
    LTG_init_extended(&strike);
    strike.time = (si32) timeSecs;
    strike.latitude = lat;
    strike.longitude = lon;
    strike.amplitude = kAmps;
    strike.nanosecs = (int) (fracSecs * 1.0e9 + 0.5);
    strike.type = LTG_GROUND_STROKE;
    strike.n_sensors = nStrokesPerFlash;
    if (rec.semimaj != 0 ||
        rec.eccent != 0 ||
        rec.angle != 0 ||
        rec.chisqr != 0) {
      strike.semi_major_axis = semiMajorAxis;
      if (eccent != 0) {
        strike.semi_minor_axis = semiMajorAxis / eccent;
      }
      strike.ellipse_angle = angle;
      strike.chi_sq = chiSq;
    }
    
    // Check for duplicates?
    
    bool useStrike = true;
    if (_params.duplicates.check){
      if (_checkNearDuplicate(strike)) {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "Strike for " << strike.latitude << ", ";
          cerr << strike.longitude << " at " << utimstr(strike.time);
          cerr << " rejected as near duplicate." << endl;
        }
        useStrike = false;
      }
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      LTG_print_extended(stderr, &strike);
    }
    
    if (useStrike) {
      _addStrike(strike);
    }
    
  } // while

  fclose(fp);
  
  if (_params.flush_buffer_per_file) {
    _startTime = _latestTime;
    if (_doWrite()) {
      iret = -1;
    }
  }

  return iret;
   
}

///////////////////////////////////////////
// decode the ltg strike from the data line
// format type 1
//
// NOTE: this fills out LTG_strike_t
  
int Ltg2Spdb::_decode_format_1(const char *line)
  
{


  int year, month, day, hour, min, sec;
  double lat, lon, amplitude;
  char typeStr[32];

  if (sscanf(line, "%d %d %d %d %d %d %lg %lg %lg %s",
             &year, &month, &day, &hour, &min, &sec,
             &lat, &lon, &amplitude, typeStr) != 10) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - _decode_format_1" << endl;
      cerr << "  Cannot decode line: " << line << endl;
      cerr << "  Expecting: year month day hour min sec "
           << "lat lon amplitude type(G or C)" << endl;
    }
    return -1;
  }
  
  // check bounding box?
  
  if (_params.checkBoundingBox) {
    if (lat < _params.boundingBox.min_lat ||
        lat > _params.boundingBox.max_lat ||
        lon < _params.boundingBox.min_lon ||
        lon > _params.boundingBox.max_lon) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - _decode_format_1" << endl;
        cerr << "  Data outside bounding box" << endl;
        cerr << "  Data line: " << line << endl;
      }
      return -1;
    }
  }

  // load up strike

  DateTime stime(year, month, day, hour, min, sec);
  time_t utime = stime.utime();
  LTG_strike_t strike;
  LTG_init(&strike);
  strike.time = (si32) utime;
  strike.latitude = (fl32) lat;
  strike.longitude = (fl32) lon;
  strike.amplitude = (si16) floor(amplitude + 0.5);
  if (typeStr[0] == 'G') {
    strike.type = LTG_GROUND_STROKE;
  } else if (typeStr[0] == 'C') {
    strike.type = LTG_CLOUD_STROKE;
  } else {
    strike.type = LTG_TYPE_UNKNOWN;
  }

  // Check for duplicates?
  if (_params.duplicates.check){
    if ( _checkNearDuplicate( strike ) ){
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Strike for " << strike.latitude << ", ";
	cerr << strike.longitude << " at " << utimstr(strike.time);
	cerr << " rejected as near duplicate." << endl;
      }
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    LTG_print_strike(stderr, &strike);
  }

  _addStrike(strike);

  return 0;

}

///////////////////////////////////////////
// decode the ltg strike from the data line
// format type 2
//
// NOTE: this fills out LTG_extended_t

int Ltg2Spdb::_decode_format_2(const char *line)
  
{


  int year, month, day, hour, min, sec;
  double lat, lon, alt, amplitude;
  char typeStr[32];

  if (sscanf(line, "%d %d %d %d %d %d %lg %lg %lg %lg %s",
             &year, &month, &day, &hour, &min, &sec,
             &lat, &lon, &alt, &amplitude, typeStr) != 11) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - _decode_format_2" << endl;
      cerr << "  Cannot decode line: " << line << endl;
      cerr << "  Expecting: year month day hour min sec "
           << "lat lon alt amplitude type(G or C)" << endl;
    }
    return -1;
  }
  
  // check bounding box?
  
  if (_params.checkBoundingBox) {
    if (lat < _params.boundingBox.min_lat ||
        lat > _params.boundingBox.max_lat ||
        lon < _params.boundingBox.min_lon ||
        lon > _params.boundingBox.max_lon) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - _decode_format_2" << endl;
        cerr << "  Data outside bounding box" << endl;
        cerr << "  Data line: " << line << endl;
      }
      return -1;
    }
  }

  // load up strike

  DateTime stime(year, month, day, hour, min, sec);
  time_t utime = stime.utime();
  LTG_extended_t strike;
  LTG_init_extended(&strike);
  strike.time = (si32) utime;
  strike.nanosecs = 0;
  strike.latitude = (fl32) lat;
  strike.longitude = (fl32) lon;
  strike.altitude = (fl32) alt;
  strike.amplitude = (fl32) amplitude;
  if (typeStr[0] == 'G') {
    strike.type = LTG_GROUND_STROKE;
  } else if (typeStr[0] == 'C') {
    strike.type = LTG_CLOUD_STROKE;
  } else {
    strike.type = LTG_TYPE_UNKNOWN;
  }

  // Check for duplicates?
  if (_params.duplicates.check){
    if ( _checkNearDuplicate( strike ) ){
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Strike for " << strike.latitude << ", ";
	cerr << strike.longitude << " at " << utimstr(strike.time);
	cerr << " rejected as near duplicate." << endl;
      }
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    LTG_print_extended(stderr, &strike);
  }

  _addStrike(strike);

  return 0;

}

///////////////////////////////////////////
// decode the ltg strike from the data line
// format type 3
//
// NOTE: this fills out LTG_extended_t

int Ltg2Spdb::_decode_format_3(const char *line)
  {

  int year, month, day, hour, min, sec, cg;
  double dSec;
  double lat, lon, amplitude;

  if (sscanf(line, "%d/%d/%d %d:%d:%lf %lg %lg %lg %d",
             &year, &month, &day, &hour, &min, &dSec,
             &lat, &lon, &amplitude, &cg) != 10) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - _decode_format_3" << endl;
      cerr << "  Cannot decode line: " << line << endl;
      cerr << "  month/day/year hour:min:sec lat lon Ka C/G, where "
           << "sec is a float (not an int) and C/G is either 0 or 1"
           << "lat lon alt amplitude type(G or C)" << endl;
    }
    return -1;
  }
  
  // check bounding box?
  
  if (_params.checkBoundingBox) {
    if (lat < _params.boundingBox.min_lat ||
        lat > _params.boundingBox.max_lat ||
        lon < _params.boundingBox.min_lon ||
        lon > _params.boundingBox.max_lon) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - _decode_format_3" << endl;
        cerr << "  Data outside bounding box" << endl;
        cerr << "  Data line: " << line << endl;
      }
      return -1;
    }
  }

  sec = (int)rint(dSec);

  // load up strike

  DateTime stime(year, month, day, hour, min, sec);
  time_t utime = stime.utime();
  LTG_extended_t strike;
  LTG_init_extended(&strike);
  strike.time = (si32) utime;
  strike.nanosecs = 0;
  strike.latitude = (fl32) lat;
  strike.longitude = (fl32) lon;
  strike.altitude = 0.0;
  strike.amplitude = (fl32) amplitude;

  if (cg)
    strike.type = LTG_GROUND_STROKE;
  else
    strike.type = LTG_CLOUD_STROKE;

  // Check for duplicates?
  if (_params.duplicates.check){
    if ( _checkNearDuplicate( strike ) ){
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Strike for " << strike.latitude << ", ";
	cerr << strike.longitude << " at " << utimstr(strike.time);
	cerr << " rejected as near duplicate." << endl;
      }
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    LTG_print_extended(stderr, &strike);
  }

  _addStrike(strike);

  return 0;

}


///////////////////////////////////////////
// decode the ltg strike from the data line
// format type 4
//
// NOTE: this fills out LTG_extended_t

int Ltg2Spdb::_decode_format_4(const char *line)
{

  time_t unixTime;
  double lat, lon, amplitude;
  int dummy, cg;

  if (sscanf(line, "%ld,%lf,%lf,%lf,%d,%d",
             &unixTime,
             &lat, &lon, &amplitude, &dummy, &cg) != 6) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - _decode_format_4" << endl;
      cerr << "  Cannot decode line: " << line << endl;
      cerr << " unixTime,lat,lon,amp" << endl;
    }
    return -1;
  }

  if (unixTime < 86400){
    cerr << "Failed to get time from " << line;
    return -1;
  }


  // check bounding box?

  if (_params.checkBoundingBox) {
    if (lat < _params.boundingBox.min_lat ||
        lat > _params.boundingBox.max_lat ||
        lon < _params.boundingBox.min_lon ||
        lon > _params.boundingBox.max_lon) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - _decode_format_4" << endl;
        cerr << "  Data outside bounding box" << endl;
        cerr << "  Data line: " << line << endl;
      }
      return -1;
    }
  }


  // load up strike

  time_t utime = unixTime;
  LTG_extended_t strike;
  LTG_init_extended(&strike);
  strike.time = (si32) utime;
  strike.nanosecs = 0;
  strike.latitude = (fl32) lat;
  strike.longitude = (fl32) lon;
  strike.altitude = 0.0;
  strike.amplitude = amplitude;
  if (cg == 0) {
    strike.type = LTG_GROUND_STROKE;
  } else {
    strike.type = LTG_CLOUD_STROKE;
  }

  // Check for duplicates?
  if (_params.duplicates.check){
    if ( _checkNearDuplicate( strike ) ){
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Strike for " << strike.latitude << ", ";
	cerr << strike.longitude << " at " << utimstr(strike.time);
	cerr << " rejected as near duplicate." << endl;
      }
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    LTG_print_extended(stderr, &strike);
  }

  _addStrike(strike);

  return 0;

}


///////////////////////////////////////////
// decode the ltg strike from the data line
// UALF LF version 1
//
// NOTE: this fills out LTG_extended_t

int Ltg2Spdb::_decode_ualf_lf_1(const char *line)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Decoding line: " << line;
  }

  int version, year, month, day, hour, min, sec, nanosecs;
  double lat, lon, amplitude;
  int multiplicity, n_sensors, degrees_freedom;
  double ellipse_angle, semi_major_axis, semi_minor_axis;
  double chi_sq, rise_time, peak_to_zero_time, max_rate_of_rise;
  int cloud_flag, angle_flag, signal_flag, timing_flag;

  if (sscanf(line,
             "%d %d %d %d %d %d %d %d "
             "%lg %lg %lg"
             "%d %d %d"
             "%lg %lg %lg"
             "%lg %lg %lg %lg"
             "%d %d %d %d",
             &version, &year, &month, &day, &hour, &min, &sec, &nanosecs,
             &lat, &lon, &amplitude,
             &multiplicity, &n_sensors, &degrees_freedom,
             &ellipse_angle, &semi_major_axis, &semi_minor_axis,
             &chi_sq, &rise_time, &peak_to_zero_time, &max_rate_of_rise,
             &cloud_flag, &angle_flag, &signal_flag, &timing_flag) != 25) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - _decode_ualf_lf_1" << endl;
      cerr << "  Cannot decode line: " << line << endl;
      cerr << "  Expecting 25 fields: " << endl;
      cerr << "    version# year month day hour min sec nanosec" << endl;
      cerr << "    lat lon amplitude multiplicity n_sensors" << endl;
      cerr << "    degress_freedom ellipse_angle" << endl;
      cerr << "    semi_major_axis semi_minor_axis chi_sq" << endl;
      cerr << "    rise_time peak_to_zero_time max_rate_of_rise" << endl;
      cerr << "    cloud_flag angle_flag signal_flag timing_flag" << endl;
    }
    return -1;
  }
  
  // check bounding box?
  
  if (_params.checkBoundingBox) {
    if (lat < _params.boundingBox.min_lat ||
        lat > _params.boundingBox.max_lat ||
        lon < _params.boundingBox.min_lon ||
        lon > _params.boundingBox.max_lon) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - _decode_ualf_lf_1" << endl;
        cerr << "  Data outside bounding box" << endl;
        cerr << "  Data line: " << line << endl;
      }
      return -1;
    }
  }

  // load up strike

  DateTime stime(year, month, day, hour, min, sec);
  time_t utime = stime.utime();
  LTG_extended_t strike;
  LTG_init_extended(&strike);
  strike.time = (si32) utime;
  strike.latitude = (fl32) lat;
  strike.longitude = (fl32) lon;
  strike.amplitude = (fl32) amplitude;
  if (cloud_flag == 0) {
    strike.type = LTG_GROUND_STROKE;
  } else {
    strike.type = LTG_CLOUD_STROKE;
  }
  strike.nanosecs = nanosecs;
  strike.n_sensors = n_sensors;
  strike.degrees_freedom = degrees_freedom;
  strike.ellipse_angle = ellipse_angle;
  strike.semi_major_axis = semi_major_axis;
  strike.semi_minor_axis = semi_minor_axis;
  strike.chi_sq = chi_sq;
  strike.rise_time = rise_time;
  strike.peak_to_zero_time = peak_to_zero_time;
  strike.max_rate_of_rise = max_rate_of_rise;
  strike.angle_flag = angle_flag;
  strike.signal_flag = signal_flag;
  strike.timing_flag = timing_flag;

  // Check for duplicates?
  if (_params.duplicates.check){
    if ( _checkNearDuplicate( strike ) ){
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Strike for " << strike.latitude << ", ";
	cerr << strike.longitude << " at " << utimstr(strike.time);
	cerr << " rejected as near duplicate." << endl;
      }
      return -1;
    }
  }


  if (_params.debug >= Params::DEBUG_VERBOSE) {
    LTG_print_extended(stderr, &strike);
  }

  _addStrike(strike);

  return 0;

}

///////////////////////////////////////////
// decode the ltg strike from the data line
// UALF LF version 1B
//
// UALF_LF_1 with - and : in date and time
//
// NOTE: this fills out LTG_extended_t

int Ltg2Spdb::_decode_ualf_lf_1b(const char *line)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Decoding line: " << line;
  }

  int version, year, month, day, hour, min, sec, nanosecs;
  double dsec;
  double lat, lon, amplitude;
  int multiplicity, n_sensors, degrees_freedom;
  double ellipse_angle, semi_major_axis, semi_minor_axis;
  double chi_sq, rise_time, peak_to_zero_time, max_rate_of_rise;
  int cloud_flag, angle_flag, signal_flag, timing_flag;

  if (sscanf(line,
             "%d %d-%d-%d %d:%d:%lg %d "
             "%lg %lg %lg"
             "%d %d %d"
             "%lg %lg %lg"
             "%lg %lg %lg %lg"
             "%d %d %d %d",
             &version, &year, &month, &day, &hour, &min, &dsec, &nanosecs,
             &lat, &lon, &amplitude,
             &multiplicity, &n_sensors, &degrees_freedom,
             &ellipse_angle, &semi_major_axis, &semi_minor_axis,
             &chi_sq, &rise_time, &peak_to_zero_time, &max_rate_of_rise,
             &cloud_flag, &angle_flag, &signal_flag, &timing_flag) != 25) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - _decode_ualf_lf_1" << endl;
      cerr << "  Cannot decode line: " << line << endl;
      cerr << "  Expecting 25 fields: " << endl;
      cerr << "    version# year month day hour min sec nanosec" << endl;
      cerr << "    lat lon amplitude multiplicity n_sensors" << endl;
      cerr << "    degress_freedom ellipse_angle" << endl;
      cerr << "    semi_major_axis semi_minor_axis chi_sq" << endl;
      cerr << "    rise_time peak_to_zero_time max_rate_of_rise" << endl;
      cerr << "    cloud_flag angle_flag signal_flag timing_flag" << endl;
    }
    return -1;
  }

  sec = (int) (dsec * 1.0e3);
  
  // check bounding box?
  
  if (_params.checkBoundingBox) {
    if (lat < _params.boundingBox.min_lat ||
        lat > _params.boundingBox.max_lat ||
        lon < _params.boundingBox.min_lon ||
        lon > _params.boundingBox.max_lon) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - _decode_ualf_lf_1" << endl;
        cerr << "  Data outside bounding box" << endl;
        cerr << "  Data line: " << line << endl;
      }
      return -1;
    }
  }

  // load up strike

  DateTime stime(year, month, day, hour, min, sec);
  time_t utime = stime.utime();
  LTG_extended_t strike;
  LTG_init_extended(&strike);
  strike.time = (si32) utime;
  strike.latitude = (fl32) lat;
  strike.longitude = (fl32) lon;
  strike.amplitude = (fl32) amplitude;
  if (cloud_flag == 0) {
    strike.type = LTG_GROUND_STROKE;
  } else {
    strike.type = LTG_CLOUD_STROKE;
  }
  strike.nanosecs = nanosecs;
  strike.n_sensors = n_sensors;
  strike.degrees_freedom = degrees_freedom;
  strike.ellipse_angle = ellipse_angle;
  strike.semi_major_axis = semi_major_axis;
  strike.semi_minor_axis = semi_minor_axis;
  strike.chi_sq = chi_sq;
  strike.rise_time = rise_time;
  strike.peak_to_zero_time = peak_to_zero_time;
  strike.max_rate_of_rise = max_rate_of_rise;
  strike.angle_flag = angle_flag;
  strike.signal_flag = signal_flag;
  strike.timing_flag = timing_flag;


  // Check for duplicates?
  if (_params.duplicates.check){
    if ( _checkNearDuplicate( strike ) ){
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Strike for " << strike.latitude << ", ";
	cerr << strike.longitude << " at " << utimstr(strike.time);
	cerr << " rejected as near duplicate." << endl;
      }
      return -1;
    }
  }


  if (_params.debug >= Params::DEBUG_VERBOSE) {
    LTG_print_extended(stderr, &strike);
  }

  _addStrike(strike);

  return 0;

}

///////////////////////////////////////////
// decode the ltg strike from the data line
// bom type axf
//
// NOTE: this fills out LTG_strike_t
  
int Ltg2Spdb::_decode_bom_axf(const char *line)
  
{


  int year, month, day, hour, min, sec;
  double lat, lon, amplitude;
  
  if (sscanf(line, "\"%4d%2d%2d%2d%2d%2d\",%lg,%lg,%lg",
             &year, &month, &day, &hour, &min, &sec,
             &lat, &lon, &amplitude) != 9) {
    return -1;
  }
  
  // check bounding box?
  
  if (_params.checkBoundingBox) {
    if (lat < _params.boundingBox.min_lat ||
        lat > _params.boundingBox.max_lat ||
        lon < _params.boundingBox.min_lon ||
        lon > _params.boundingBox.max_lon) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - _decode_format_1" << endl;
        cerr << "  Data outside bounding box" << endl;
        cerr << "  Data line: " << line << endl;
      }
      return -1;
    }
  }

  // load up strike

  DateTime stime(year, month, day, hour, min, sec);
  time_t utime = stime.utime();
  LTG_strike_t strike;
  LTG_init(&strike);
  strike.time = (si32) utime;
  strike.latitude = (fl32) lat;
  strike.longitude = (fl32) lon;
  strike.amplitude = (si16) floor(amplitude + 0.5);
  strike.type = LTG_TYPE_UNKNOWN;

  // Check for duplicates?
  if (_params.duplicates.check){
    if ( _checkNearDuplicate( strike ) ){
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Strike for " << strike.latitude << ", ";
	cerr << strike.longitude << " at " << utimstr(strike.time);
	cerr << " rejected as near duplicate." << endl;
      }
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    LTG_print_strike(stderr, &strike);
  }

  _addStrike(strike);

  return 0;

}

///////////////////////////////////////////
// decode the ltg strike from the data line
// wwldn
//
// NOTE: this fills out LTG_strike_t
  
int Ltg2Spdb::_decode_wwldn(const char *line)
  
{


  int year, month, day, hour, min, sec;
  double millisec, lat, lon, residual;
  int n;
  
  if (sscanf(line, "%4d/%2d/%2d %2d:%2d:%2d %lf %lf %lf %lf %d",
             &year, &month, &day, &hour, &min, &sec,
             &millisec, &lat, &lon, &residual, &n) != 11) {
    cerr << "Invalid line: " << line << endl;
    
    return -1;
  }
  
  // check bounding box?
  
  if (_params.checkBoundingBox) {
    if (lat < _params.boundingBox.min_lat ||
        lat > _params.boundingBox.max_lat ||
        lon < _params.boundingBox.min_lon ||
        lon > _params.boundingBox.max_lon) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - _decode_format_1" << endl;
        cerr << "  Data outside bounding box" << endl;
        cerr << "  Data line: " << line << endl;
      }
      return -1;
    }
  }

  // load up strike

  DateTime stime(year, month, day, hour, min, sec);
  time_t utime = stime.utime();
  LTG_strike_t strike;
  LTG_init(&strike);
  strike.time = (si32) utime;
  strike.latitude = (fl32) lat;
  strike.longitude = (fl32) lon;
  strike.amplitude = 0;
  strike.type = LTG_TYPE_UNKNOWN;

  // Check for duplicates?
  if (_params.duplicates.check){
    if ( _checkNearDuplicate( strike ) ){
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Strike for " << strike.latitude << ", ";
	cerr << strike.longitude << " at " << utimstr(strike.time);
	cerr << " rejected as near duplicate." << endl;
      }
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    LTG_print_strike(stderr, &strike);
  }

  _addStrike(strike);

  return 0;

}

///////////////////////////////////////////
// decode the ltg strike from the data line
// wwldn type 2 - comma-delimited
//
// NOTE: this fills out LTG_extended_t

int Ltg2Spdb::_decode_wwlln(const char *line)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Decoding line: " << line;
  }

  int year, month, day, hour, min, sec, usec;
  double lat, lon, residual;
  int n_sensors;
  
  if (sscanf(line, "%4d/%2d/%2d,%2d:%2d:%2d.%6d, %lf, %lf, %lf, %d",
             &year, &month, &day, &hour, &min, &sec, &usec,
             &lat, &lon, &residual, &n_sensors) != 11) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - _decode_wwlln" << endl;
      cerr << "  Cannot decode line: " << line << endl;
      cerr << "  Expecting 11 fields as follows: " << endl;
      cerr << "    yyyy/mm/dd,hh:mm:ss.uuuuu, lat, lon, residual, n" << endl;
    }
    return -1;
  }

  int nanosecs = usec * 1000;
  
  // check bounding box?
  
  if (_params.checkBoundingBox) {
    if (lat < _params.boundingBox.min_lat ||
        lat > _params.boundingBox.max_lat ||
        lon < _params.boundingBox.min_lon ||
        lon > _params.boundingBox.max_lon) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - _decode_wwlln" << endl;
        cerr << "  Data outside bounding box" << endl;
        cerr << "  Data line: " << line << endl;
      }
      return -1;
    }
  }
  
  // load up strike
  
  DateTime stime(year, month, day, hour, min, sec);
  time_t utime = stime.utime();
  LTG_extended_t strike;
  LTG_init_extended(&strike);
  strike.time = (si32) utime;
  strike.latitude = (fl32) lat;
  strike.longitude = (fl32) lon;
  strike.residual = (fl32) residual;
  strike.type = LTG_GROUND_STROKE;
  strike.nanosecs = nanosecs;
  strike.n_sensors = n_sensors;

  // Check for duplicates?
  if (_params.duplicates.check){
    if ( _checkNearDuplicate( strike ) ){
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Strike for " << strike.latitude << ", ";
	cerr << strike.longitude << " at " << utimstr(strike.time);
	cerr << " rejected as near duplicate." << endl;
      }
      return -1;
    }
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    LTG_print_extended(stderr, &strike);
  }

  _addStrike(strike);

  return 0;

}

///////////////////////////////////////////
// decode the ltg strike from the data line
// NAPLN - comma-delimited
//
// NOTE: this fills out LTG_extended_t

int Ltg2Spdb::_decode_napln(const char *line)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Decoding line: " << line;
  }

  int year, month, day, hour, min, sec, usec;
  double lat, lon, amplitude, semi_major_axis, semi_minor_axis; 
  int ellipse_angle;
  
  if (sscanf(line, "%4d-%2d-%2dT%2d:%2d:%2d.%3d,%lf,%lf,%lf,%lf,%lf,%d",
             &year, &month, &day, &hour, &min, &sec, &usec,
             &lat, &lon, &amplitude, &semi_major_axis,
	     &semi_minor_axis, &ellipse_angle) != 13) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - _decode_napln" << endl;
      cerr << "  Cannot decode line: " << line << endl;
      cerr << "  Expecting 13 fields as follows: " << endl;
      cerr << "    yyyy-mm-ddThh:mm:ss.uuu, lat, lon, semi_major_axis";
      cerr << "    semi_minor_axis, ellipse_angle" << endl;
    }
    return -1;
  }

  int nanosecs = usec * 1000;
  
  // check bounding box?
  
  if (_params.checkBoundingBox) {
    if (lat < _params.boundingBox.min_lat ||
        lat > _params.boundingBox.max_lat ||
        lon < _params.boundingBox.min_lon ||
        lon > _params.boundingBox.max_lon) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - _decode_wwlln" << endl;
        cerr << "  Data outside bounding box" << endl;
        cerr << "  Data line: " << line << endl;
      }
      return -1;
    }
  }
  
  // load up strike
  
  DateTime stime(year, month, day, hour, min, sec);
  time_t utime = stime.utime();
  LTG_extended_t strike;
  LTG_init_extended(&strike);
  strike.time = (si32) utime;
  strike.latitude = (fl32) lat;
  strike.longitude = (fl32) lon;
  strike.amplitude = (fl32) amplitude;
  strike.semi_major_axis = (fl32) semi_major_axis;
  strike.semi_minor_axis = (fl32) semi_minor_axis;
  strike.ellipse_angle = (fl32) ellipse_angle;
  strike.type = LTG_GROUND_STROKE;
  strike.nanosecs = nanosecs;

  // Check for duplicates?
  if (_params.duplicates.check){
    if ( _checkNearDuplicate( strike ) ){
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Strike for " << strike.latitude << ", ";
	cerr << strike.longitude << " at " << utimstr(strike.time);
	cerr << " rejected as near duplicate." << endl;
      }
      return -1;
    }
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    LTG_print_extended(stderr, &strike);
  }

  _addStrike(strike);

  return 0;

}

int Ltg2Spdb::_decode_ksc(char const* line, char const *file_path)
{
  // get first 4 charaters of filename  which is the year since it is 
  // not found in the data file
  // filenaming convention is yyyydddhhmm.txt
  string fileStr(file_path);
  string yearStr = fileStr.substr(fileStr.length()-15,4);      
  int year = atoi(yearStr.c_str());
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Decoding line: " << line;
  }

  const double ksc_lat = 28.538486;
  const double ksc_lon = -80.639578; 

  int jday, hour, min, sec, usec;
  int xCoord, yCoord, zCoord;
  char eventType[16];
  
  // sample line:
  // JDAY    TIME(UTC)               X(M)            Y(M)            Z(M)    EVENT TYPE
  // 155     15:30:02:079381         -0001300        -0001500        +00500  CAL EVENT
  if (sscanf(line, "%3d %2d:%2d:%2d:%6d   %d %d  %d %s %*s",
             &jday, &hour, &min, &sec, &usec,
             &xCoord, &yCoord, &zCoord, eventType) != 9) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - _decode_ksc" << endl;
      cerr << "  Cannot decode line: " << line << endl;
      cerr << "  Expecting 10 fields as follows: " << endl;
      cerr << "  JDAY HH:MM:SS:ssssss  xCoord yCoord zCoord EVENT TYPE " << endl;
    }
    return -1;
  }
  else
  {
    // valid event strings
    string event(eventType);
    string ldar_event("LDAR");
    string dlss_event("4DLSS");
    string gglss_event("GGLSS");
    string cal_event("CAL");
    
    if ( (_params.ksc_event == Params::KSC_LDAR_EVENT && event != ldar_event) || 
         (_params.ksc_event == Params::KSC_4DLSS_EVENT && event != dlss_event) || 
         (_params.ksc_event == Params::KSC_GGLSS_EVENT && event != gglss_event) || 
         (_params.ksc_event == Params::KSC_CAL_EVENT && event != cal_event) )  {
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "KSC event " << event.c_str() << " is not a valid event. Skipping." << endl;
      }
      return -1;
    }

    DateTime stime;
    stime.setByDayOfYear(year, jday, hour, min, sec);
    time_t utime = stime.utime();
    LTG_strike_t strike;
    LTG_init(&strike);
    strike.time = (si32) utime;
    
    //The flat projection for area surrounding KSC is hard wired below
    // and contains lat lon, rotation, nx, ny, nz, dx, dy ,dz,minx, miny, minz
    PjgFlatCalc pjg( 28.538486, -80.642633, 0.0,
                     600,600,10,1,1,1,-300,-300,0);

    double xKm = xCoord/1000;
    double yKm = yCoord/1000;
    double lat,lon;
    pjg.xy2latlon(xKm,yKm,lat,lon);
    strike.latitude = (fl32) lat;
    strike.longitude = (fl32) lon;
    // strike.amplitude not set
    strike.type = LTG_TYPE_UNKNOWN;
    
    // Check for duplicates?
    if (_params.duplicates.check){
      if ( _checkNearDuplicate( strike ) ){
    	if (_params.debug >= Params::DEBUG_VERBOSE) {
    	  cerr << "Strike for " << strike.latitude << ", ";
    	  cerr << strike.longitude << " at " << utimstr(strike.time);
    	  cerr << " rejected as near duplicate." << endl;
    	}
    	return -1;
      }
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      LTG_print_strike(stderr, &strike);
    }

    _addStrike(strike);
    
  }
  return 0;
}

/*********************************************************************
 * Adds short strike to the strike buffer.
 */

void Ltg2Spdb::_addStrike(const LTG_strike_t &strike)
{

  // at start of buffer, save out id and time
  
  if (_strikeBuffer.getLen() == 0) {
    _dataId = SPDB_KAV_LTG_ID;
    _dataLabel = SPDB_KAV_LTG_LABEL,
    _startTime = strike.time;
    _nStrikes = 0;
  }

  if (_dataId != SPDB_KAV_LTG_ID) {
    cerr << "ERROR - incorrect strike type, should be SPDB_KAV_LTG_ID" << endl;
    return;
  }

  LTG_strike_t copy = strike;
  LTG_to_BE(&copy);
  
  _strikeBuffer.add(&copy, sizeof(copy));

  _nStrikes++;
  _latestTime = strike.time;
  
}

/*********************************************************************
 * Adds extended strike to the strike buffer.
 */

void Ltg2Spdb::_addStrike(const LTG_extended_t &strike)
{

  // at start of buffer, save out id and time
  
  if (_strikeBuffer.getLen() == 0) {
    _dataId = SPDB_LTG_ID;
    _dataLabel = SPDB_LTG_LABEL,
    _startTime = strike.time;
    _nStrikes = 0;
  }

  if (_dataId != SPDB_LTG_ID) {
    cerr << "ERROR - incorrect strike type, should be SPDB_LTG_ID" << endl;
    return;
  }
  
  LTG_extended_t copy = strike;
  LTG_extended_to_BE(&copy);

  _strikeBuffer.add(&copy, sizeof(copy));

  _nStrikes++;
  _latestTime = strike.time;

}
///////////////////////////////////////////
// write output if we are ready to do so

int Ltg2Spdb::_writeIfReady()
  
{

  int iret = 0;

  // write the output, if we are ready to.

  if (_params.debug >= Params::DEBUG_VERBOSE) { 
    cerr << "latestTime: " << DateTime::strm(_latestTime) << endl;
    cerr << "startTime: " << DateTime::strm(_startTime) << endl;
  }

  bool readyToWrite = false;

  if ((_latestTime - _startTime) >= _params.bufferingPeriod) {
    readyToWrite = true;
    _startTime = _latestTime;
  }

  // In realtime, do the write if enough time has elapsed,
  // regardless of data time.
  
  if (_params.mode == Params::REALTIME){
    time_t now = time(NULL);
    if ((now - _writeTime) >= _params.bufferingPeriod){
      readyToWrite = true;
    }
  }

  if (readyToWrite){
    if (_doWrite()) {
      iret = -1;
    }
  }
      
  return iret;

}


///////////////////////////////////////////
// write output to spdb

int Ltg2Spdb::_doWrite()
  
{

  int iret = 0;

  _writeTime = time(NULL);

  if (_params.debug) {
    cerr << endl;
    cerr << "Writing data for time: " << DateTime::strm(_startTime) << endl;
    cerr << " url: " << _params.output_url << endl;
    cerr << " nstrikes: " << _nStrikes << endl;
  }

  // Send the data to the database

  DsSpdb spdb;
  switch (_params.put_mode) {
  case Params::PUT_OVER:
    spdb.setPutMode(Spdb::putModeOver);
    break;
  case Params::PUT_ADD:
    spdb.setPutMode(Spdb::putModeAdd);
    break;
  case Params::PUT_ADD_UNIQUE:
    spdb.setPutMode(Spdb::putModeAddUnique);
    break;
  case Params::PUT_ONCE:
    spdb.setPutMode(Spdb::putModeOnce);
    break;
  }

  int dataType = 0;

  if (spdb.put(_params.output_url,
	       _dataId, _dataLabel,
	       dataType,
	       _startTime,
	       _startTime + _params.expire_seconds,
	       _strikeBuffer.getLen(), _strikeBuffer.getPtr())) {
    fprintf(stderr, "ERROR: Ltg2Spdb::writeToSpdb\n");
    fprintf(stderr, "  Error writing ltg to URL <%s>\n", _params.output_url);
    fprintf(stderr, "%s\n", spdb.getErrStr().c_str());
    iret = -1;
  }

  _strikeBuffer.free();

  // sleep to make sure data can get distributed

  if (_params.delay_after_write) {
    umsleep(_params.delay_after_write);
  }

  return iret;

}

//////////////////////////////////////////////////////////////
//
// Check for near duplicates. Returns 0 if it is not
// a near duplicate, else -1. Two overloaded entry points
// and then the real routine.
//
int Ltg2Spdb::_checkNearDuplicate(const LTG_extended_t &strike ){
  if (_params.dup_check_cg_only && strike.type ==LTG_CLOUD_STROKE) return 0;
  return _checkNearDuplicate(strike.latitude, strike.longitude, strike.time, strike.nanosecs);
}


int Ltg2Spdb::_checkNearDuplicate(const LTG_strike_t &strike ){
  if (_params.dup_check_cg_only && strike.type ==LTG_CLOUD_STROKE) return 0;
  return _checkNearDuplicate(strike.latitude, strike.longitude, strike.time, 0);
}


int Ltg2Spdb::_checkNearDuplicate(const double lat, const double lon, const time_t time, const int nanosecs){

  //
  // Is this our first? If so, just record position and
  // return 0
  //
  if ((_lastLat == 0) && (_lastLon == 0) && (_lastTime == 0L)){
    _lastLat = lat;
    _lastLon = lon;
    _lastTime = time;
    _strokeCount = 1;
    _flashDuration = 0;
    _lastNanosecs = nanosecs;
    return 0;
  }

  _strokeCount = _strokeCount + 1;
  _flashDuration = _flashDuration + (time + nanosecs/1000000000.0 - _lastTime + _lastNanosecs/1000000000.0);

  float maxTime;
  float maxFlashDuration;
  if (_params.duplicates.time_millisec){
    // convert milliseconds to decimal seconds
    maxTime = float(_params.duplicates.maxTime) / 1000.0;
    maxFlashDuration = float(_params.duplicates.maxFlashDuration / 1000.0);
  } else {
    maxTime = float(_params.duplicates.maxTime);
    maxFlashDuration = float(_params.duplicates.maxFlashDuration);
  }
    
  //
  // Has the time changed enough that we don't even have to worry about it?
  // If so, we're OK.
  //
  if (time + nanosecs/1000000000.0 - _lastTime + _lastNanosecs/1000000000.0 > maxTime){
    _lastLat = lat;
    _lastLon = lon;
    _lastTime = time;
    _strokeCount = 1;
    _flashDuration = 0;
    _lastNanosecs = nanosecs;
    return 0;
  }

  // have we exceeded our maximum stroke count?
  if (_strokeCount > _params.duplicates.maxCount){
    _lastLat = lat;
    _lastLon = lon;
    _lastTime = time;
    _strokeCount = 1;
    _flashDuration = 0;
    _lastNanosecs = nanosecs;
    return 0;
  }

  // have we exceeded our maximum flash duration?
  if (_flashDuration > maxFlashDuration){
    _lastLat = lat;
    _lastLon = lon;
    _lastTime = time;
    _strokeCount = 1;
    _flashDuration = 0;
    _lastNanosecs = nanosecs;
    return 0;
  }

  //
  // Need to test distance - numerically more intense but that's life
  //
  double r,theta;
  PJGLatLon2RTheta(lat, lon, _lastLat, _lastLon, &r, &theta);
  if (r > _params.duplicates.maxDist){
    _lastLat = lat;
    _lastLon = lon;
    _lastTime = time;
    _strokeCount = 1;
    _flashDuration = 0;
    _lastNanosecs = nanosecs;
    return 0;
  }

  //
  // If we got here :
  // * It's not the first stroke,
  // * It's within the temporal limit, and
  // * It's within the distance limit, and
  // * The stroke count is <= maxCount, and
  // * The flash duration <= maxFlashDuration, so we consider
  // it a duplicate - return -1 and don't record this lat/lon.

  if (_params.debug >= Params::DEBUG_VERBOSE){
    cerr << endl;
    cerr << "Rejecting stroke as duplicate\n";
    cerr << "  Time difference(sec): " << time + nanosecs/1000000000.0 - _lastTime + _lastNanosecs/1000000000.0 << endl;
    cerr << "  Distance: " << r << endl;
    cerr << "  Stroke Count: " << _strokeCount << endl;
    cerr << "  Flash duration(sec): " << _flashDuration << endl;
  }

  return -1;

}

int Ltg2Spdb::_decode_alblm(const char *line)
{
  // Decode alaska BLM data-- 15 potential fields
  // format is:OBJECTID,STROKETYPE,NETWORKCODE,UTCDATETIME,LOCALDATETIME,MILLISECONDS,LATITUDE,LONGITUDE,AMPLITUDE,GDOP,ERRSEMIMAJOR,ERRSEMIMINOR,ERRELIPSEANGLE,STRIKETIME,STRIKESEQNUMBER
     
  //
  // Create space for string field data
  //
  char    *outStrs[15];
 
  for (int i = 0; i < 15; i++)
  {
    outStrs[i] = new char[64];
    outStrs[i][0] = '\0';
  }

  //
  // Separate the comma delimited fields
  //
  STRparse_delim(line, outStrs, 960,",", 15, 64);
  
  LTG_extended_t strike;
 
  LTG_init_extended(&strike);
 
 
  if( strcmp( outStrs[1],"STROKETYPE") == 0)
  {
    // comment line, no data
    return -1;
  }
  else if ( strcmp( outStrs[1],"GROUND_STROKE") == 0)
  {
    strike.type = LTG_GROUND_STROKE;
  }
  else if ( strcmp( outStrs[1], "CLOUD_STROKE") == 0)
  {
    strike.type = LTG_CLOUD_STROKE;
  }
  else 
  {
    strike.type = LTG_TYPE_UNKNOWN;
  }

  //
  // Get the strike time
  //
  int year, month, day, hour, min, sec;
  sscanf(outStrs[3],"%d/%d/%d %d:%d:%d",&month,&day,&year,&hour,&min,&sec);

  double milliSecs;
  sscanf(outStrs[5],"%lf", &milliSecs);

  DateTime strikeTime(year,month,day, hour,min, sec);
  strike.time = (si32) strikeTime.utime();
  strike.nanosecs = milliSecs;
  
  //
  // Get the latitude and longitude
  //
  sscanf(outStrs[6],"%f", &strike.latitude);
  sscanf(outStrs[7],"%f", &strike.longitude);
  cerr << "strike.latitude " << strike.latitude << endl;
  cerr << "strike.longitude " << strike.longitude << endl;

  // check bounding box?

  if (_params.checkBoundingBox) 
  {
    if (strike.latitude < _params.boundingBox.min_lat ||
        strike.latitude > _params.boundingBox.max_lat ||
        strike.longitude < _params.boundingBox.min_lon ||
        strike.longitude > _params.boundingBox.max_lon) 
    {
      if (_params.debug >= Params::DEBUG_VERBOSE) 
      {
        cerr << "WARNING - Alaska BLM " << endl;
        cerr << "  Data outside bounding box" << endl;
        cerr << "  Data line: " << line << endl;
      }
      return -1;
    }
  }

  sscanf(outStrs[8],"%f", &strike.amplitude);

  sscanf(outStrs[10], "%f", &strike.semi_major_axis);
  sscanf(outStrs[11], "%f", &strike.semi_minor_axis);
  sscanf(outStrs[12], "%f", &strike.ellipse_angle);


  //
  // Mem cleanup
  // 
  for (int i = 0; i < 15; i++)
  {
    delete(outStrs[i]);
  }

  // Check for duplicates?
  if (_params.duplicates.check)
  {
    if ( _checkNearDuplicate( strike ) )
    {
      if (_params.debug >= Params::DEBUG_VERBOSE) 
      {
  	cerr << "Strike for " << strike.latitude << ", ";
  	cerr << strike.longitude << " at " << utimstr(strike.time);
  	cerr << " rejected as near duplicate." << endl;
      }
      return -1;
    }
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) 
  {
    LTG_print_extended(stderr, &strike);
  }
  
  _addStrike(strike);
  
  return 0;
}
 


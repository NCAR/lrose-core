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
// ClassIngest.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// March 2014.
//
// Code originally by Terri Betancourt.
//
///////////////////////////////////////////////////////////////
//
// ClassIngest reads automated sounding observations in CLASS
// format, and writes them to an SPDB data base
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/toolsa_macros.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/TaStr.hh>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <didss/DataFileNames.hh>
#include <Spdb/SoundingPut.hh>
#include "ClassIngest.hh"
using namespace std;

const unsigned int ClassIngest::NFIELDS_IN = 6;
const char* ClassIngest::HEIGHT_LABEL = "Alt";
const char* ClassIngest::U_WIND_LABEL = "Uwind";
const char* ClassIngest::V_WIND_LABEL = "Vwind";
const char* ClassIngest::PRESSURE_LABEL = "Press";
const char* ClassIngest::REL_HUM_LABEL = "RH";
const char* ClassIngest::TEMPERATURE_LABEL = "Temp";
const char* ClassIngest::DELIMETER = " ,:\n\r";
const double ClassIngest::MISSING_VALUE = 999.0;
const char* ClassIngest::INDEX_FILENAME = "ClassIngest.index";

// Constructor
// Params::
/*
// for REALTIME mode
ClassIngest::ClassIngest(char *sounding_url, enum mode_t _params_mode,
            params_debug, 
          params_input_dir,
          //startTime, endTime,
          _params.expire_secs,
          //_params.take_siteID_from_file,
          //_params.specified_siteID,
          //_params._spdb_urls[ii],
          //_params.spdb_urls_n,
          params_file_match_string,
          params_file_suffix,
          params_strict_subdir_check,
          params_max_realtime_valid_age,
          params_latest_data_info_avail)

{
  _init(params_debug);
      // realtime mode - no latest_data_info file
    
  _input = new DsInputPath(_progName,
          params_debug ,
          params_input_dir,
          params_max_realtime_valid_age,
          PMU_auto_register,
          params_latest_data_info_avail,
          true);
    
    if (params_strict_subdir_check) {
      _input->setStrictDirScan(true);
    }
    
    if (strlen(params_file_suffix) > 0) {
      _input->setSearchExt(params_file_suffix);
    }
    
    if (strlen(params_file_match_string) > 0) {
      _input->setSubString(params_file_match_string);
    }
}

// for FILELIST mode
ClassIngest::ClassIngest(char *sounding_url, args_inputFileList, params_debug)
{
  _init(params_debug);


  _input = new DsInputPath(_progName,
          params_debug ,
          args_inputFileList);
}
*/

// for  ARCHIVE mode
// endTime = requested Time
// startTime = requested Time - max lookback time
// 
ClassIngest::ClassIngest(char *sounding_url,
          bool params_debug, 
          char *params_input_dir,
          time_t startTime, time_t endTime) 
{

  //isOK = true;

  // set programe name

  //_progName = "ClassIngest";
  //ucopyright((char *) _progName.c_str());

  _init(params_debug); 


    // archive mode - start time to end time
    
  // DsInputPath will find all files in the input_dir that
  // are between startTime and endTime
  _input = new DsInputPath(_progName,
          params_debug, //  ,
          params_input_dir,
          startTime, endTime);

  // get command line args

  //if (_args.parse(argc, argv, _progName)) {
  //  cerr << "ERROR: " << _progName << endl;
  //  cerr << "Problem with command line args" << endl;
  //  isOK = FALSE;
  //  return;
  //}

  // get TDRP params
  
  //_paramsPath = (char *) "unknown";
  //if (_params.loadFromArgs(argc, argv, _args.override.list,
	//		   &_paramsPath)) {
  //  cerr << "ERROR: " << _progName << endl;
  //  cerr << "Problem with TDRP parameters" << endl;
  //  isOK = FALSE;
  //  return;
  //}

  // init process mapper registration

  //PMU_auto_init((char *) _progName.c_str(),
	//	_params.instance,
	//	PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

ClassIngest::~ClassIngest()

{

  // unregister process

  //PMU_auto_unregister();

}

void ClassIngest::_init(bool params_debug) {

  _debug = params_debug;

  // set programe name

  _progName = "ClassIngest";
  ucopyright((char *) _progName.c_str());
}

//////////////////////////////////////////////////
// Run

int ClassIngest::readSoundingText() // RunIt ()
{

  // register with procmap
  
  //PMU_auto_register("Run");

  /* file input object
  
  DsInputPath *input = NULL; // Set to NULL to get around compiler warnings.
  
  if (_params.mode == Params::FILELIST) {
    
    // FILELIST mode
    
    input = new DsInputPath(_progName,
			    _debug ,
			    _args.inputFileList);
    
  } else if (_params.mode == Params::ARCHIVE) {
    
    // archive mode - start time to end time
    
    input = new DsInputPath(_progName,
			    _debug ,
			    _params.input_dir,
			    _args.startTime, _args.endTime);
    
  } else if (_params.mode == Params::REALTIME) {
    
    // realtime mode - no latest_data_info file
    
    input = new DsInputPath(_progName,
			    _debug ,
			    _params.input_dir,
			    _params.max_realtime_valid_age,
			    PMU_auto_register,
			    _params.latest_data_info_avail,
			    true);
    
    if (_params.strict_subdir_check) {
      input->setStrictDirScan(true);
    }
    
    if (strlen(_params.file_suffix) > 0) {
      input->setSearchExt(_params.file_suffix);
    }
    
    if (strlen(_params.file_match_string) > 0) {
      input->setSubString(_params.file_match_string);
    }
    
  }
  */
  // loop through available files
  
  char *inputPath;
  if (_input == NULL) {
      cerr << "WARNING - ClassIngest::Run" << endl;
      cerr << "  Error: file input object is NULL." << endl;
      return -1;
  }
  inputPath = _input->next();
  while (inputPath != NULL) {
    
    if (_processFile(inputPath)) {
      cerr << "WARNING - ClassIngest::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
    }
    inputPath = _input->next();
  } // while
    
  return 0;

}

////////////////////
// process the file

int ClassIngest::_processFile(const char *filePath)
  
{

  int iret = 0;

  if (_debug) {
    cerr << "=================================================" << endl;
    cerr << "Processing file: " << filePath << endl;
  }

  bool dateOnly;
  if (DataFileNames::getDataTime(filePath, _dataTime, dateOnly)) {
    cerr << "WARNING - ClassIngest::_processFile" << endl;
    cerr << "  Cannot get time from file name: " << filePath << endl;
    cerr << "  Will use current time instead" << endl;
    _dataTime = time(NULL);
  }

  // procmap registration

  char procmapString[BUFSIZ];
  Path path(filePath);
  sprintf(procmapString, "Processing file <%s>", path.getFile().c_str());
  //PMU_force_register(procmapString);

  // create spdb object for sounding

  SoundingPut sounding;
  //vector<string *> urls;
  //for (int ii = 0; ii < _params.spdb_urls_n; ii++) {
  //  string *url = new string(_params._spdb_urls[ii]);
  //  urls.push_back(url);
  //  if (_debug) {
  //    cerr << "Output to URL: " << *url << endl;
  //  }
  //}
  //if (_params.take_siteID_from_file) {
  //  sounding.init(urls, Sounding::SONDE_ID, "CLASS");
  //} else {
  //  sounding.init(urls, Sounding::SONDE_ID, "CLASS", _params.specified_siteID);
  //}
  sounding.setMissingValue(MISSING_VALUE);

  // free up string vector
  
  //for (size_t ii = 0; ii < urls.size(); ii++) {
  //  delete urls[ii];
  //}

  // open file

  FILE *in;

  if ((in = ta_fopen_uncompress(filePath, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - ClassIngest::_processFile" << endl;
    cerr << "  Cannot open sounding file: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // Read the file

  if (_readHeader(in, sounding)) {
    cerr << "ERROR - ClassIngest::_processFile" << endl;
    cerr << "  Cannot read header" << endl;
    fclose(in);
    return -1;
  }

  if (_findColumns(in)) {
    cerr << "ERROR - ClassIngest::_processFile" << endl;
    cerr << "  Cannot find columns" << endl;
    fclose(in);
    return -1;
  }

  if (_findFirstData(in)) {
    cerr << "ERROR - ClassIngest::_processFile" << endl;
    cerr << "  Cannot find first data" << endl;
    fclose(in);
    return -1;
  }

  if (_readData(in)) {
    cerr << "ERROR - ClassIngest::_processFile" << endl;
    cerr << "  Cannot read data" << endl;
    fclose(in);
    return -1;
  }

  // Close the file
  
  fclose(in);

  /* write out the sounding

  if (_writeSounding(sounding)) {
    cerr << "ERROR - ClassIngest::_processFile" << endl;
    cerr << "  Writing to SPDB" << endl;
    iret = -1;
  }
  */
  // done

  if (_debug) {
    cerr << "  Done with file: " << filePath << endl;
  }

  return iret;
   
}

///////////////////////////////////////////////////////////////////////
// Read in file header

int ClassIngest::_readHeader(FILE *in, SoundingPut &sounding)

{
  string text;
  
  // set the siteId

  int siteId; //  = _params.specified_siteID;
  //if (_params.take_siteID_from_file){
    if (_getHeaderText(in, "Launch Site Type/Site ID", text) == 0) {
      vector<string> toks;
      TaStr::tokenize(text, ", ", toks);
      if (toks.size() == 2) {
        if (_debug) {
          cerr << "StationId: " << toks[1] << endl;
        }
        siteId = Spdb::hash4CharsToInt32(toks[1].c_str());
      }
    }
  //}
  sounding.setSiteId(siteId);
  
  // Get/set the launch location

  double lat = 0.0;
  double lon = 0.0;
  double alt = 0.0;
  if (_getHeaderText(in, "Launch Location", text) == 0) {
    vector<string> toks;
    TaStr::tokenize(text, ", ", toks);
    if (toks.size() >= 3) {
      alt = atof(toks[toks.size()-1].c_str());
      lat = atof(toks[toks.size()-2].c_str());
      lon = atof(toks[toks.size()-3].c_str());
    }
  }
  if (lat == 0.0 && lon == 0.0 && alt == 0.0) {
    cerr << "ERROR - ClassIngest::_readHeader" << endl;
    cerr << "  no lat/lon/alt available from file" << endl;
    return -1;
  }
  sounding.setLocation(lat, lon, alt);
  
  // get the launch time
  
  _launchTime = 0L;
  if (_getHeaderText(in, "GMT Launch Time", text) == 0) {
    date_time_t T;
    if (6 == sscanf(text.c_str(),
                    "%d, %d, %d, %d:%d:%d",
                    &T.year, &T.month, &T.day, &T.hour, &T.min, &T.sec)){
      uconvert_to_utime(&T);
      _launchTime = T.unix_time;
    }
  }

  // Set the site name from the Project ID

  if (_getHeaderText(in, "Project ID", text) == 0) {
    sounding.setSiteName(text);
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////
// Get text from header line, given the label
//
// returns 0 on success, -1 on failure

int ClassIngest::_getHeaderText(FILE *in, const char* label, string &text)
{

  long  posStart, posCurrent = -1L;
  bool  found = false;
  char  line[BUFSIZ];
  char *lptr = NULL; // Set to NULL to avoid compiler warnings.

  // Hang onto the starting file position

  if ((posStart = ftell(in)) == -1L) {
    return -1;
  }

  // Read each line until we either find what we're looking for
  // or we come back around to our starting point

  while (posCurrent != posStart) {
    
    lptr = fgets(line, BUFSIZ, in);
    posCurrent = ftell(in);

    if (feof(in)){
      // We've reached the end of file -- wrap around
      rewind(in);
      posCurrent = ftell(in);
    } else {
      // See if this is the line we want
      if (strstr(line, label) != NULL) {
        // This is it
        found = true;
        break;
      }
    }
  }

  if (found) {

    // jump past label

    lptr += 35;

    // move forward to first printing character

    while (*lptr != '\n') {
      if (isblank(*lptr)) {
        lptr++;
      } else {
        break;
      }
    }

    // strip line feed

    for (size_t ii = 0; ii < strlen(lptr); ii++) {
      if (lptr[ii] == '\n') {
        lptr[ii] = '\0';
      }
    }
    
    // assign to return string

    text.assign(lptr);

    if (_debug) {
      cerr << "Header label: " << label << endl;
      cerr << "       value: " << text << endl;
    }

    if (text.size() > 0) {
      return 0;
    } else {
      return -1;
    }

  }

  return -1;

}

///////////////////////////////////////////////////////////////////////
// Get column location in file

int ClassIngest::_findColumns(FILE *in)
{

  int column = 0;
  char *field, line[BUFSIZ];
  
  // Find the labels line.  It must start with the label "Time".

  if (fgets(line, BUFSIZ, in) == NULL) {
    return -1;
  }
  field = strtok(line, DELIMETER);
  while((field == NULL) ||
         ((field != NULL) && (strcmp(field, "Time") != 0))) {
    if (fgets(line, BUFSIZ, in) == NULL) {
      return -1;
    }
    field = strtok(line, DELIMETER);
  }

  // Look for all the labels we need and note the column for each.

  while(field != NULL) {

    if (strcmp(field, U_WIND_LABEL) == 0) {
      columnData[column] = &uwind[0];
      if (_debug ) {
        fprintf(stderr, "   Field '%s' found in column %d\n", 
                U_WIND_LABEL, column);
      }
    } else if (strcmp(field, V_WIND_LABEL) == 0) {
      columnData[column] = &vwind[0];
      if (_debug ) {
        fprintf(stderr, "   Field '%s' found in column %d\n", 
                V_WIND_LABEL, column);
      }
    } else if (strcmp(field, HEIGHT_LABEL) == 0) {
      columnData[column] = &height[0];
      if (_debug ) {
        fprintf(stderr, "   Field '%s' found in column %d\n", 
                HEIGHT_LABEL, column);
      }
    } else if (strcmp(field, PRESSURE_LABEL) == 0) {
      columnData[column] = &pressure[0];
      if (_debug ) {
        fprintf(stderr, "   Field '%s' found in column %d\n", 
                PRESSURE_LABEL, column);
      }
    } else if (strcmp(field, TEMPERATURE_LABEL) == 0) {
      columnData[column] = &temperature[0];
      if (_debug ) {
        fprintf(stderr, "   Field '%s' found in column %d\n", 
                TEMPERATURE_LABEL, column);
      }
    } else if (strcmp(field, REL_HUM_LABEL) == 0) {
      columnData[column] = &relHum[0];
      if (_debug ) {
        fprintf(stderr, "   Field '%s' found in column %d\n", 
                REL_HUM_LABEL, column);
      }
    }

    field = strtok(NULL, DELIMETER);
    column++;

  } // while

  // Make sure we found all the fields we were interested in

  if (columnData.size() != NFIELDS_IN) {
    return -1;
  }
 
  return 0;

}

///////////////////////////////////////////////////////////////////////
// Find the first data in the file

int ClassIngest::_findFirstData(FILE *in)
{

  char *field, line[BUFSIZ];
  long fpos;

  // Hang onto the current file position.

  if ((fpos = ftell(in)) == -1L) {
    return -1;
  }

  // Look for the first line of data.  There might be non-data lines between
  // the labels line and the first line of data.

  if (fgets(line, BUFSIZ, in) == NULL) {
    return -1;
  }
  field = strtok(line, DELIMETER);
  
  // Check the first two chars of field since a digit may be negative and
  // thus, start with '-'.

  while((field == NULL) ||
        ((field != NULL) && (!isdigit(*field) && !isdigit(*(field+1))))) {

    // Get the current file position.

    if ((fpos = ftell(in)) == -1L) {
      return -1;
    }

    if (fgets(line, BUFSIZ, in) == NULL) {
      return -1;
    }
    field = strtok(line, DELIMETER);
  }

  // Reset the file pointer to the beginning of this line of data.

  fseek(in, fpos, SEEK_SET);
  return 0;

}

///////////////////////////////////////////////////////////////////////
// Read in actual data

int ClassIngest::_readData(FILE *in)

{
  int targetCol, icol, status = 0; // Set to 0 to avoid compiler warnings.
  double value, *dataArray;
  char line[BUFSIZ], lineCopy[BUFSIZ], *fptr;
  const char *BLANK = " ";

  map<int, double*, less<int> >::iterator item;

  // Bad altitude indicator is 99999.0 in the CLASS format.

  const float BAD_ALT  = 99999.0;

  // Read in each line of data.

  _numPoints = 0;
  while (fgets(line, BUFSIZ, in) != NULL) {

    // Get each field of interest

    for(item=columnData.begin(); item != columnData.end(); item ++) {

      // Get the nth column value from the line 
      // for this field of interest 
      // and stick it in the appropriate data array

      status = 1;
      targetCol = (*item).first;
      strncpy(lineCopy, line, BUFSIZ);

      // Scan for the zero'th column

      fptr = strtok(lineCopy, BLANK);
      sscanf(fptr, "%lf", &value);

      // Scan for the target column

      for(icol=0; icol < targetCol && fptr; icol++) {
        fptr = strtok(NULL, BLANK);
        sscanf(fptr, "%lf", &value);
      }

      if (fptr) {

        // We've found the target column we're looking for
        // Get a handle to the data array for this field of interest

        dataArray = (*item).second;

        // If this is a bad altitude value, ignore this line
        // because we key the data from altitudes

        if (value == BAD_ALT) {

          if (dataArray != &height[0]) {

            // This should never happen
            // Something went really wrong in determining which column
            // to read for the altitude values

            cerr << "ERROR - ClassIngest::_readData" << endl;
            cerr << "  Inconsistency in parsing sounding file" << endl;
            return -1;

          }
          status = (int)BAD_ALT;
          break;

        } else {

          // For all other fields of interest (besides alt),
          // store the data value. Check for some common (if
          // erroneously used) values for the missing data value,
          // and replace them with the one they were supposed to use.
          
          if ((value == 9999.0) ||
              (value == -9999.0) ||
              (value == 99999.0) ||
              (value == -99999.0) ||
              (value == -999.0)){
            value = MISSING_VALUE;
          }
          dataArray[_numPoints] = value;
        }
      }

    } // item

    if (status != (int)BAD_ALT) {
      _numPoints++;
    }

  } // while

  // Make sure we found some data

  if (_numPoints <= 0) {
    cerr << "ERROR - ClassIngest::_readData" << endl;
    cerr << "  No data points in the sounding file" << endl;
    return -1;
  } else {
    return 0;
  }

}

////////////////////////
// write out sounding

int ClassIngest::_writeSounding(SoundingPut &sounding)

{

  int status;
  double *wwind = NULL;
  time_t when = 0L;

  // Set the sounding time to the time in the file if we got it,
  // otherwise to the file name time.
  
  if (_launchTime == 0L){
    when = _dataTime;
  } else {
    when = _launchTime;
  }

  // Set the data values in the Sounding class
  
  status = sounding.set(when, _numPoints, height, uwind, vwind, wwind,
                        pressure, relHum, temperature);

  if (status != 0) {
    cerr << "ERROR - ClassIngest::_writeSounding" << endl;
    cerr << "  Could not set the sounding values." << endl;
    return -1;
  }

  // Tell the user about the sounding data we're getting ready to write

  if (_debug) {
    fprintf(stderr,
            "   Set sounding data for '%s'\n"
            "   Launched from (lat, lon) (%lf, %lf)\n",
            sounding.getSiteName().c_str(), sounding.getLat(), sounding.getLon());

    double min, max, avg;
    sounding.getStats(Sounding::ALTITUDE, &min, &max, &avg);
    
    fprintf(stderr,
            "   HEIGHT:  (min, max, avg) (%9.2lf%9.2lf%9.2lf)\n", 
            min, max, avg);
    sounding.getStats(Sounding::U_WIND, &min, &max, &avg);
    fprintf(stderr, 
            "   U_WIND:  (min, max, avg) (%9.2lf%9.2lf%9.2lf)\n", 
            min, max, avg);
    sounding.getStats(Sounding::V_WIND, &min, &max, &avg);
    fprintf(stderr, 
            "   V_WIND:  (min, max, avg) (%9.2lf%9.2lf%9.2lf)\n", 
            min, max, avg);
    sounding.getStats(Sounding::PRESSURE, &min, &max, &avg);
    fprintf(stderr, 
            "   PRESS :  (min, max, avg) (%9.2lf%9.2lf%9.2lf)\n", 
            min, max, avg);
    sounding.getStats(Sounding::REL_HUMIDITY, &min, &max, &avg);
    fprintf(stderr, 
            "   RELHUM:  (min, max, avg) (%9.2lf%9.2lf%9.2lf)\n", 
            min, max, avg);
    sounding.getStats(Sounding::TEMPERATURE, &min, &max, &avg);
    fprintf(stderr, 
            "   TEMP  :  (min, max, avg) (%9.2lf%9.2lf%9.2lf)\n", 
            min, max, avg);
  }

  // Write the sounding to the database
  
  //status = sounding.writeSounding(when, when + _params.expire_secs);

  //if (status  != 0) {
  //  cerr << "ERROR - ClassIngest::_writeSounding" << endl;
  //  cerr << "Could not write to the sounding database." << endl;
  //  return -1;
  //}

  //if (_debug) {
  //  cerr << "Wrote the sounding for time: " << DateTime::strm(_dataTime) << endl;
  //}
  
  return 0;

}

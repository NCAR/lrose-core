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
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <regex>
#include <string>
#include <algorithm>
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
const char* ClassIngest::U_WIND2_LABEL = "Ucmp";
const char* ClassIngest::V_WIND_LABEL = "Vwind";
const char* ClassIngest::V_WIND2_LABEL = "Vcmp";
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

  _init(params_debug); 


    // archive mode - start time to end time
    
  // DsInputPath will find all files in the input_dir that
  // are between startTime and endTime
  _input = new DsInputPath(_progName,
          params_debug, //  ,
          params_input_dir,
          startTime, endTime);

  return;

}

// destructor

ClassIngest::~ClassIngest()

{

}

DateTime ClassIngest::getLaunchTime() {
  return _launchTime;
}

string   ClassIngest::getSourceName() {
  return _projectId;
}

void ClassIngest::_init(bool params_debug) {

  _debug = params_debug;

  // set programe name

  _progName = "ClassIngest";

}

//////////////////////////////////////////////////
// Run

int ClassIngest::readSoundingText()
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
  
  int iret = 1;
  char *inputPath;
  if (_input == NULL) {
      cerr << "WARNING - ClassIngest::readSoundingText" << endl;
      cerr << "  Error: file input object is NULL." << endl;
      return 0;
  }
  bool noFilesRead = true;
  inputPath = _input->next();
  while (inputPath != NULL) {
    
    if (_processFile(inputPath)) {
      cerr << "WARNING - ClassIngest::readSoundingText" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
      iret = -1;
    } 
    noFilesRead = false;
    inputPath = _input->next();
  } // while
   
  if (noFilesRead) {
    cerr << "WARNING - ClassIngest::readSoundingText" << endl;
    cerr << "  No sounding files read; try changing the look back time " << endl;
    iret = -1;
  } 
  return iret;

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
  //

  //FILE *in;

  std::ifstream sounding_file;
  try {
    sounding_file.open(filePath, std::ios::in);
  } catch (const ifstream::failure& e) {



  //if ((in = ta_fopen_uncompress(filePath, "r")) == NULL) {
    //int errNum = errno;
    cerr << "ERROR - ClassIngest::_processFile" << endl;
    cerr << "  Cannot open sounding file: " << filePath << endl;
    cerr << "  " << e.what() << endl; // strerror(errNum) << endl;
    return -1;
  }



  // Read the file

  // ifstream& solo_script, std::iostream& javascript

  if (_readHeader(sounding_file, sounding)) {
    cerr << "ERROR - ClassIngest::_processFile" << endl;
    cerr << "  Cannot read header" << endl;
    sounding_file.close();
    return -1;
  }

  if (_findColumns(sounding_file)) {
    cerr << "ERROR - ClassIngest::_processFile" << endl;
    cerr << "  Cannot find columns" << endl;
    sounding_file.close();
    return -1;
  }
/*
  if (_findFirstData(sounding_file)) {
    cerr << "ERROR - ClassIngest::_processFile" << endl;
    cerr << "  Cannot find first data" << endl;
    sounding_file.close();
    return -1;
  }
*/
  if (_readData(sounding_file)) {
    cerr << "ERROR - ClassIngest::_processFile" << endl;
    cerr << "  Cannot read data" << endl;
    sounding_file.close();
    return -1;
  }


  // Close the file
  
  sounding_file.close();

  // write out the sounding data

  if (_debug) {
    _writeSoundingData();
  }

  // done

  if (_debug) {
    cerr << "  Done with file: " << filePath << endl;
  }

  return iret;
   
}

///////////////////////////////////////////////////////////////////////
// Read in file header

bool ClassIngest::_process_HeaderText(string line, std::iostream& javascript) {
  bool recognized = false;
            // "Launch Site Type/Site ID",
      const std::regex pieces_regex("(Release|Launch) Site Type/Site ID:[\\s]+([\\w]+[,\\s\\w]*)"); // [,[:space:][:alnum:]]*)"); // [\\s]"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
      std::smatch pieces_match;
          //string mytest2 = "Release Site Type Site ID:    Moulton, AL";
      if (std::regex_match(line, pieces_match, pieces_regex)) {
              //std::cout << line << '\n';
        for (size_t i = 0; i < pieces_match.size(); ++i) {
          std::ssub_match sub_match = pieces_match[i];
          std::string piece = sub_match.str();
          if (_debug) std::cerr << "  submatch " << i << ": " << piece << '\n';
        }   
        string command = pieces_match[1];
              //format_it(command);
        string siteName = pieces_match[2];
        if (_debug) cerr << command << " : " << siteName << endl;
        javascript << siteName << endl;
        recognized = true;
      } else {
            //std::cout << "regex_match returned false\n";
      } 
  return recognized;
}

// TODO: how to return the location
bool ClassIngest::_process_Location(string line, std::iostream& javascript) {
  bool recognized = false;
  // "Release Location (lon,lat,alt):    087 25.55'W, 34 29.04'N, -87.426, 34.484, 199.0",
  const std::regex pieces_regex("(Release|Launch) Location \\(lon,lat,alt\\):[\\s]+([\\d]+)\\s([\\d]+\\.[\\d]+)'(W|E), ([\\d]+) ([\\d]+\\.[\\d]+)'(N|S),([+-.,\\s\\d]+)"); // " ()'[N|S], ([:digit:]), ([:num"); // [,[:space:][:alnum:]]*)"); // [\\s]"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
  std::smatch pieces_match;

  string mytest2 = "Release Location (lon,lat,alt):    087 25.55'W, 34 29.04'N, -87.426, 34.484, 199.0";
  if (std::regex_match(line, pieces_match, pieces_regex)) {
              //std::cout << line << '\n';
    for (size_t i = 0; i < pieces_match.size(); ++i) {
      std::ssub_match sub_match = pieces_match[i];
      std::string piece = sub_match.str();
      if (_debug) std::cerr << "  submatch " << i << ": " << piece << '\n';
    }   
    string lat_lon_alt = pieces_match[8];
    if (_debug) cerr << "found lon, lat, alt : " << lat_lon_alt << endl;
    javascript << lat_lon_alt << endl;
              // TODO: tokenize submatch[8] which contains final three numeric values
    recognized = true;
  } else {
            //std::cout << "regex_match returned false\n";
  } 
  return recognized;
}    


bool ClassIngest::_process_LaunchTime(string line, std::iostream& javascript) {
  bool recognized = false;
  // "UTC Release Time (y,m,d,h,m,s):    2016, 03, 31, 00:01:00",
  //  if (_getHeaderText(in, "GMT Launch Time", text) == 0) 
  const std::regex pieces_regex("(UTC|GMT) (Release|Launch) Time \\(y,m,d,h,m,s\\):[\\s]+([:,\\s\\d]+)");
  std::smatch pieces_match;

  string mytest2 = "UTC Release Time (y,m,d,h,m,s):    2016, 03, 31, 00:01:00";
  if (std::regex_match(line, pieces_match, pieces_regex)) {
              //std::cout << line << '\n';
    for (size_t i = 0; i < pieces_match.size(); ++i) {
      std::ssub_match sub_match = pieces_match[i];
      std::string piece = sub_match.str();
      if (_debug) std::cerr << "  submatch " << i << ": " << piece << '\n';
    }   
    string dateTime = pieces_match[3];
    if (_debug) cerr << "Launch Time : " << dateTime << endl;
    javascript << dateTime << endl;
              // TODO: return date and time in javascript stream 
    recognized = true;
  } else {
            //std::cout << "regex_match returned false\n";
  } 
        
  return recognized;
}    

bool ClassIngest::_process_ProjectId(string line, std::iostream& javascript) {
  bool recognized = false;
  // "Project ID:                        VORTEX-SE_2016",

  const std::regex pieces_regex("Project ID:[\\s]+([-_[:alnum:]]+)");
  std::smatch pieces_match;

  string mytest2 = "Project ID:                        VORTEX-SE_2016";
  if (std::regex_match(line, pieces_match, pieces_regex)) {
              //std::cout << line << '\n';
    for (size_t i = 0; i < pieces_match.size(); ++i) {
      std::ssub_match sub_match = pieces_match[i];
      std::string piece = sub_match.str();
      if (_debug) std::cerr << "  submatch " << i << ": " << piece << '\n';
    }   
              //string command = pieces_match[1];
              //format_it(command);
    string siteName = pieces_match[1];
    if (_debug) cerr << "Project ID : " << siteName << endl;
    javascript << siteName << endl;
              // TODO: return date and time in javascript stream 
    recognized = true;
  } else {
            //std::cout << "regex_match returned false\n";
  } 
        
  return recognized;
}    



bool ClassIngest::_process_columnData(string line, std::iostream& javascript) {
  bool recognized = false;
  //  list of floating point numbers ...

  const std::regex pieces_regex("[-+]?[0-9]*\\.?[0-9]+"); // ("[^[:digit:]]-+[:digit:].[:digit:]+]+"); // Temp)[\\s]+[.]*");
  std::smatch pieces_match;

  //string mytest2 = "    0.0  978.4  21.5  18.1  81.1   -1.4    3.8   4.0 160.0 999.0  -87.426  34.484 999.0  13.6 ";
  if (std::regex_search(line, pieces_match, pieces_regex)) {
    recognized = true;
  } else {
    //std::cout << "regex_match returned false\n";
  }         
  return recognized;
} 

int ClassIngest::_readHeader(ifstream& sounding_file, SoundingPut &sounding) {
  if (!sounding_file.is_open()) {
    return -1;
  }

  // loop through the lines, and try the regex until done

  std::string line;
  //std::string siteName;
  std::stringstream javascript;
  int numRecognized = 0;
  int nNeeded = 4;

  while (getline(sounding_file, line) && (numRecognized < nNeeded)) {
    if (_debug) std::cerr << "|" << line << "| \n";
    bool recognized = _process_HeaderText(line, javascript);
    if (recognized) {
      // set the siteId
      int siteId; 
      std::string text;
      getline(javascript, text);
      siteId = Spdb::hash4CharsToInt32(text.c_str());
      sounding.setSiteId(siteId);
      if (_debug) {
        cerr << "siteId " << line << " converted to " << siteId << endl;
      }
      numRecognized += 1;
    } else if (_process_Location(line, javascript)) {
      // Get/set the launch location

      double lat = 0.0;
      double lon = 0.0;
      double alt = 0.0;
      std::string text;
      getline(javascript, text);
      
      vector<string> toks;
      TaStr::tokenize(text, ", ", toks);
      if (toks.size() >= 3) {
        alt = atof(toks[toks.size()-1].c_str());
        lat = atof(toks[toks.size()-2].c_str());
        lon = atof(toks[toks.size()-3].c_str());
      }

      if (lat == 0.0 && lon == 0.0 && alt == 0.0) {
        cerr << "ERROR - ClassIngest::_readHeader" << endl;
        cerr << "  no lat/lon/alt available from file" << endl;
        return -1;
      }
      if (_debug) {
        cerr << "lat, lon, alt = " << lat << "," << lon << "," << alt << endl;
      }
      sounding.setLocation(lat, lon, alt);
      numRecognized += 1;      
    } else if (_process_LaunchTime(line, javascript)) { 
      // get the launch time
      _launchTime = 0L;
      string dateTime;
      date_time_t T;
      getline(javascript, dateTime);
      if (6 == sscanf(dateTime.c_str(),
                      "%d, %d, %d, %d:%d:%d",
                      &T.year, &T.month, &T.day, &T.hour, &T.min, &T.sec)){
        uconvert_to_utime(&T);
        _launchTime = T.unix_time;
      }
      if (_debug) {
        cerr << "launch time " << _launchTime << endl;
      }
      numRecognized += 1;      
    } else if (_process_ProjectId(line, javascript)) {
        // Set the site name from the Project ID
      std::string text;
      getline(javascript, text);
      if (_debug) {
        cerr << "project ID = " << text << endl;
      }
      numRecognized += 1;
    //if (_getHeaderText(in, "Project ID", text) == 0) {
      //sounding.setSiteName(text);
      _projectId = text;
    //}
    } else {
      if (_debug) cerr << "line not recognized " << endl;
    }
    if (_debug) cerr << "numRecognized = " << numRecognized << " nNeeded = " << nNeeded << endl;

  }

  if (numRecognized < nNeeded) return -1;
  return 0;
}

///////////////////////////////////////////////////////////////////////
// Get text from header line, given the label
//
// returns 0 on success, -1 on failure

int ClassIngest::_getHeaderText(ifstream& sounding_file, const char* label, string &text)
{

  return -1;

}

///////////////////////////////////////////////////////////////////////
// Get column location in file

int ClassIngest::_findColumns(ifstream& sounding_file)
{

  int column = 0;
  char *field; // , line[BUFSIZ];
  
  // Find the labels line.  It must start with the label "Time".
  if (!sounding_file.is_open()) {
    return -1;
  }
  
  // Don't use regex, just use tokenizer on line. 
  // Remember, we need the column number as well!

  std::string line;
  bool recognized = false;

  getline(sounding_file, line);
  while ((sounding_file.good()) && !recognized) {
    if (_debug) std::cout << "|" << line << "| \n";
    field = strtok(const_cast<char*>(line.c_str()), DELIMETER);
    if (strcmp(field, "Time") == 0) {
      recognized = true;
    } else {
      getline(sounding_file, line);
    }
  }
  if (!recognized) return -1;

  //  do we not record the time data?
  //columnData[column] = &time[0];

  // Look for all the labels we need and note the column for each.

  while(field != NULL) {

    if ((strcmp(field, U_WIND_LABEL) == 0) || 
      (strcmp(field, U_WIND2_LABEL) == 0)) {
      columnData[column] = &uwind[0];
      if (_debug ) {
        fprintf(stderr, "   Field '%s' found in column %d\n", 
                U_WIND_LABEL, column);
      }
    } else if ((strcmp(field, V_WIND_LABEL) == 0) ||
      (strcmp(field, V_WIND2_LABEL) == 0)) {
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

int ClassIngest::_findFirstData(ifstream& sounding_file)
{

  return 0;

}

///////////////////////////////////////////////////////////////////////
// Read in actual data

int ClassIngest::_readData(ifstream& sounding_file)

{
  // Read in each line of data.

  if (!sounding_file.is_open()) {
    return -1;
  }

  _numPoints = 0;
  std::string line;
  std::stringstream javascript;
  bool recognized = false;
    while (getline(sounding_file, line)) { // } && !recognized) {
      if (_debug) std::cerr << "|" << line << "| \n";
      recognized = _process_columnData(line, javascript);
      if (recognized) {
         // extract selected columns of data ...
       _extractSelectedData(line);
     }
   }

  // Make sure we found some data

  if (_numPoints <= 0) {
    cerr << "ERROR - ClassIngest::_readData" << endl;
    cerr << "  No data points in the sounding file" << endl;
    return -1;
  } else {
    return 0;
  }
}

int ClassIngest::_extractSelectedData(std::string line) {

  int targetCol, icol, status = 0; // Set to 0 to avoid compiler warnings.
  double value, *dataArray;
  char lineCopy[BUFSIZ], *fptr;
  const char *BLANK = " ";

  map<int, double*, less<int> >::iterator item;

  // Bad altitude indicator is 99999.0 in the CLASS format.

  const float BAD_ALT  = 99999.0;

    // Get each field of interest

    for(item=columnData.begin(); item != columnData.end(); item ++) {

      // Get the nth column value from the line 
      // for this field of interest 
      // and stick it in the appropriate data array

      status = 1;
      targetCol = (*item).first;
      strncpy(lineCopy, line.c_str(), BUFSIZ);

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

/*
  // Make sure we found some data

  if (_numPoints <= 0) {
    cerr << "ERROR - ClassIngest::_readData" << endl;
    cerr << "  No data points in the sounding file" << endl;
    return -1;
  } else {
    return 0;
  }
*/  
    return 0;
}

void ClassIngest::_writeSoundingData() {

  map<int, double*, less<int> >::iterator item;
  for(item=columnData.begin(); item != columnData.end(); item ++) {
    cerr << "column " << item->first << " : ";
    double *values = item->second;
    for (int i=0; i< _numPoints; i++) {
      cerr << values[i] << " ";
    }
    cerr << endl;
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
  
  return 0;

}

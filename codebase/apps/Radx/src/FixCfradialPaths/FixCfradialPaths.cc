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
///////////////////////////////////////////////////////////////
// FixCfradialPaths.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2010
//
///////////////////////////////////////////////////////////////

#include "FixCfradialPaths.hh"
#include <Radx/RadxVol.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/NcfRadxFile.hh>
#include <dsserver/DsLdataInfo.hh>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <cerrno>
using namespace std;

// Constructor

FixCfradialPaths::FixCfradialPaths(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "FixCfradialPaths";
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list, &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

}

// destructor

FixCfradialPaths::~FixCfradialPaths()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int FixCfradialPaths::Run()
{

  if (_params.mode == Params::ARCHIVE) {
    return _runArchive();
  } else if (_params.mode == Params::FILELIST) {
    return _runFilelist();
  } else {
    return _runRealtime();
  }
}

//////////////////////////////////////////////////
// Run in filelist mode

int FixCfradialPaths::_runFilelist()
{

  // loop through the input file list

  int iret = 0;

  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {

    string inputPath = _args.inputFileList[ii];
    if (_processFile(inputPath)) {
      iret = -1;
    }

  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int FixCfradialPaths::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - FixCfradialPaths::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - FixCfradialPaths::_runFilelist()" << endl;
    cerr << "  No files found, dir: " << _params.input_dir << endl;
    return -1;
  }
  
  // loop through the input file list
  
  int iret = 0;
  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_processFile(paths[ii])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode

int FixCfradialPaths::_runRealtime()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  // watch for new data to arrive

  LdataInfo ldata(_params.input_dir,
                  _params.debug >= Params::DEBUG_VERBOSE);
  
  int iret = 0;

  while (true) {
    ldata.readBlocking(_params.max_realtime_data_age_secs,
                       1000, PMU_auto_register);
    
    const string path = ldata.getDataPath();
    if (_processFile(path)) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Process a file
// Returns 0 on success, -1 on failure

int FixCfradialPaths::_processFile(const string &filePath)
{

  if (_params.debug) {
    cerr << "============================================" << endl;
    cerr << "DEBUG - FixCfradialPaths" << endl;
    cerr << "  Processing file: " << filePath << endl;
  }
  
  RadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  RadxVol vol;
  if (inFile.readFromPath(filePath, vol)) {
    cerr << "ERROR - FixCfradialPaths::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }

  _startTime = vol.getStartTimeSecs();
  _endTime = vol.getEndTimeSecs();
  _startMillisecs = (int) (vol.getStartNanoSecs() / 1.0e6 + 0.5);
  _endMillisecs = (int) (vol.getEndNanoSecs() / 1.0e6 + 0.5);

  if (_params.debug) {
    cerr << "  Start time: " << RadxTime::strm(_startTime) << endl;
    cerr << "  End   time: " << RadxTime::strm(_endTime) << endl;
  }

  // compute the rename time

  _renameTime = _startTime;
  _renameMillisecs = _startMillisecs;
  if (_params.output_filename_mode == Params::END_TIME_ONLY) {
    _renameTime = _endTime;
    _renameMillisecs = _endMillisecs;
  }

  // compute the new filename

  string newName;
  if (_computeNewName(vol, filePath, newName)) {
    cerr << "ERROR - FixCfradialPaths::Run" << endl;
    cerr << "  Cannot compute new name" << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "New file name: " << newName << endl;
  }

  // rename file

  if (_params.file_rename == Params::RENAME_IN_PLACE) {
    if (_renameInPlace(filePath, newName)) {
      cerr << "ERROR - FixCfradialPaths::Run" << endl;
      cerr << "  Cannot rename file" << endl;
      return -1;
    }
  } else if (_params.file_rename == Params::COPY_FILE) {
    if (_copyFile(filePath, newName, _params.copy_dir)) {
      cerr << "ERROR - FixCfradialPaths::Run" << endl;
      cerr << "  Cannot copy file" << endl;
      return -1;
    }
  } else if (_params.file_rename == Params::CREATE_SYMBOLIC_LINK) {
    if (_createSymbolicLink(filePath, newName, _params.link_dir)) {
      cerr << "ERROR - FixCfradialPaths::Run" << endl;
      cerr << "  Cannot create symbolic link" << endl;
      return -1;
    }
  } else {
    cerr << "NOTE: no action taken" << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up read

void FixCfradialPaths::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }
  
  // read in times times and metadata only

  file.setReadMetadataOnly(true);

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }
  
}

//////////////////////////////////////////////////
// compute new file name

int FixCfradialPaths::_computeNewName(const RadxVol &vol,
                                      const string &filePath, 
                                      string &newName)
  
{
  
  RadxPath path(filePath);
  string dir = path.getDirectory();
  string oldName = path.getFile();
  size_t cfradPos = oldName.find("cfrad");
  if (cfradPos == string::npos) {
    cerr << "ERROR - FixCfradialPaths::_computeNewName" << endl;
    cerr << "  Cannot find 'cfrad' substring in file name" << endl;
    cerr << "  fileName: " << oldName << endl;
    return -1;
  }

  // set up RadxFile to compute file name

  NcfRadxFile wFile;
  _setupWritePath(wFile);

  // compute new path
  
  RadxTime startTime(vol.getStartTimeSecs());
  int startMillisecs = (int) (vol.getStartNanoSecs() / 1.0e6 + 0.5);
  if (startMillisecs > 999) {
    startTime.set(vol.getStartTimeSecs() + 1);
    startMillisecs -= 1000;
  }
  RadxTime endTime(vol.getEndTimeSecs());
  int endMillisecs = (int) (vol.getEndNanoSecs() / 1.0e6 + 0.5);
  if (endMillisecs > 999) {
    endTime.set(vol.getEndTimeSecs() + 1);
    endMillisecs -= 1000;
  }
  RadxTime fileTime = startTime;
  int fileMillisecs = startMillisecs;
  if (_params.output_filename_mode == Params::END_TIME_ONLY) {
    fileTime = endTime;
    fileMillisecs = endMillisecs;
  }

  string writePath = wFile.computeWritePath(vol,
                                            startTime, startMillisecs,
                                            endTime, endMillisecs,
                                            fileTime, fileMillisecs,
                                            dir);

  RadxPath wPath(writePath);
  newName = wPath.getFile();

  return 0;

}

//////////////////////////////////////////////////
// set up write

void FixCfradialPaths::_setupWritePath(RadxFile &wFile)
{
  
  if (_params.output_filename_mode == Params::START_TIME_ONLY) {
    wFile.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_TIME_ONLY);
  } else if (_params.output_filename_mode == Params::END_TIME_ONLY) {
    wFile.setWriteFileNameMode(RadxFile::FILENAME_WITH_END_TIME_ONLY);
  } else {
    wFile.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_AND_END_TIMES);
  }

  if (strlen(_params.output_filename_prefix) > 0) {
    wFile.setWriteFileNamePrefix(_params.output_filename_prefix);
  }
  if (strlen(_params.output_filename_suffix) > 0) {
    wFile.setWriteFileNameSuffix(_params.output_filename_suffix);
  }

  wFile.setWriteInstrNameInFileName(_params.include_instrument_name_in_file_name);
  wFile.setWriteSiteNameInFileName(_params.include_site_name_in_file_name);
  wFile.setWriteSubsecsInFileName(_params.include_subsecs_in_file_name);
  wFile.setWriteScanTypeInFileName(_params.include_scan_type_in_file_name);
  wFile.setWriteScanNameInFileName(_params.include_scan_name_in_file_name);
  wFile.setWriteRangeResolutionInFileName(_params.include_range_resolution_in_file_name);
  wFile.setWriteVolNumInFileName(_params.include_vol_num_in_file_name);
  wFile.setWriteHyphenInDateTime(_params.use_hyphen_in_file_name_datetime_part);
  
}

//////////////////////////////////////////////////
// rename file in place

int FixCfradialPaths::_renameInPlace(const string &filePath,
                                     const string &newName)

{

  RadxPath path(filePath);
  string dateDir = path.getDirectory();
  RadxPath datePath(dateDir);
  string outDir = datePath.getDirectory();

  // compute subdir
  
  string subDirPath = _computeSubdirPath(outDir);

  // make sure subdir exists
  
  if (ta_makedir_recurse(subDirPath.c_str())) {
    int errNum = errno;
    cerr << "ERROR - FixCfradialPaths::_renameInPlace" << endl;
    cerr << "  Cannot make subdir: " << subDirPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // compute new path

  char newPath[1024];
  sprintf(newPath, "%s%s%s",
          subDirPath.c_str(), PATH_DELIM, newName.c_str());

  // rename file

  if (_params.debug) {
    cerr << "Renaming file in place from:" << endl;
    cerr << "  " << filePath << endl;
    cerr << "to:" << endl;
    cerr << "  " << newPath << endl;
  }

  if (_params.test_only) {
    return 0;
  }

  if (rename(filePath.c_str(), newPath)) {
    int errNum = errno;
    cerr << "ERROR - FixCfradialPaths::_renameInPlace" << endl;
    cerr << "  Cannot rename file in place" << endl;
    cerr << "  oldPath: " << filePath << endl;
    cerr << "  newPath: " << newPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // in realtime mode, write latest data info file
  
  if (_params.mode == Params::REALTIME) {
    DsLdataInfo ldata(_params.input_dir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(subDirPath, newPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(_renameTime)) {
      cerr << "WARNING - FixCfradialPaths::_renameInPlace" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << outDir << endl;
    }
  }

  return 0;


}

//////////////////////////////////////////////////
// copy file

int FixCfradialPaths::_copyFile(const string &filePath,
                                const string &newName,
                                const string &outDir)
  
{

  // compute subdir

  string subDirPath = _computeSubdirPath(outDir);

  // make sure subdir exists
  
  if (ta_makedir_recurse(subDirPath.c_str())) {
    int errNum = errno;
    cerr << "ERROR - FixCfradialPaths::_copyFile" << endl;
    cerr << "  Cannot make subdir: " << subDirPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // compute new path

  char newPath[1024];
  sprintf(newPath, "%s%s%s",
          subDirPath.c_str(), PATH_DELIM, newName.c_str());

  // copy the file
  
  if (_params.debug) {
    cerr << "Copying file from:" << endl;
    cerr << "  " << filePath << endl;
    cerr << "to:" << endl;
    cerr << "  " << newPath << endl;
  }

  if (_params.test_only) {
    return 0;
  }

  if (filecopy_by_name(newPath, filePath.c_str())) {
    int errNum = errno;
    cerr << "ERROR - FixCfradialPaths::_copyFile" << endl;
    cerr << "  Cannot copy file" << endl;
    cerr << "  oldPath: " << filePath << endl;
    cerr << "  newPath: " << newPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // in realtime mode, write latest data info file
  
  if (_params.mode == Params::REALTIME) {
    DsLdataInfo ldata(_params.input_dir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(subDirPath, newPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(_renameTime)) {
      cerr << "WARNING - FixCfradialPaths::_copyFile" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << outDir << endl;
    }
  }

  return 0;


}

//////////////////////////////////////////////////
// create symbolic link

int FixCfradialPaths::_createSymbolicLink(const string &filePath,
                                          const string &newName,
                                          const string &outDir)

{

  // compute subdir
  
  string subDirPath = _computeSubdirPath(outDir);
  
  // make sure subdir exists
  
  if (ta_makedir_recurse(subDirPath.c_str())) {
    int errNum = errno;
    cerr << "ERROR - FixCfradialPaths::_copyFile" << endl;
    cerr << "  Cannot make subdir: " << subDirPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // compute new path

  char newPath[1024];
  sprintf(newPath, "%s%s%s",
          subDirPath.c_str(), PATH_DELIM, newName.c_str());
  
  // create link command

  char command[1024];
  sprintf(command, "cd %s; ln -s %s %s",
          subDirPath.c_str(),
          filePath.c_str(),
          newName.c_str());

  // run link command

  if (_params.debug) {
    cerr << "Creating sym link with command:" << endl;
    cerr << "  " << command << endl;
  }

  if (_params.test_only) {
    return 0;
  }

  if (system(command)) {
    int errNum = errno;
    cerr << "ERROR - FixCfradialPaths::_copyFile" << endl;
    cerr << "  Cannot make link using 'system'" << endl;
    cerr << "  command: " << command << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // in realtime mode, write latest data info file
  
  if (_params.mode == Params::REALTIME) {
    DsLdataInfo ldata(_params.input_dir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(subDirPath, newPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(_renameTime)) {
      cerr << "WARNING - FixCfradialPaths::_copyFile" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << outDir << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// compute output subdir

string FixCfradialPaths::_computeSubdirPath(const string &outDir)

{

  RadxTime rtime(_renameTime);
  char subDir[1024];

  if (_params.append_year_dir_to_output_dir) {
    sprintf(subDir, "%s%s%.4d%s%.4d%.2d%.2d",
            outDir.c_str(), PATH_DELIM,
            rtime.getYear(), PATH_DELIM,
            rtime.getYear(), rtime.getMonth(), rtime.getDay());
  } else if (_params.append_day_dir_to_output_dir) {
    sprintf(subDir, "%s%s%.4d%.2d%.2d",
            outDir.c_str(), PATH_DELIM,
            rtime.getYear(), rtime.getMonth(), rtime.getDay());
  } else {
    sprintf(subDir, "%s", outDir.c_str());
  }

  return subDir;

}

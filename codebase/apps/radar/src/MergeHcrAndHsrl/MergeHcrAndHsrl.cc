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
// MergeHcrAndHsrl.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2018
//
///////////////////////////////////////////////////////////////
//
// Merges field data from HCR and HSRL instruments.
// Writes combined data into CfRadial files.
//
////////////////////////////////////////////////////////////////

#include "MergeHcrAndHsrl.hh"
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxRay.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DataFileNames.hh>
#include <toolsa/pmu.h>
using namespace std;

// Constructor

MergeHcrAndHsrl::MergeHcrAndHsrl(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "MergeHcrAndHsrl";
  
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

  // process the params

  if (_checkParams()) {
    OK = FALSE;
    return;
  }

}

// destructor

MergeHcrAndHsrl::~MergeHcrAndHsrl()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MergeHcrAndHsrl::Run()
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
// Process the params, set up field lists

int MergeHcrAndHsrl::_checkParams()
{

  int iret = 0;
  return iret;

}

//////////////////////////////////////////////////
// Run in filelist mode

int MergeHcrAndHsrl::_runFilelist()
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

int MergeHcrAndHsrl::_runArchive()
{

  // get the files to be processed
  
  RadxTimeList tlist;
  tlist.setDir(_params.hcr_data_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - MergeHcrAndHsrl::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, HCR dir: "
         << _params.hcr_data_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }
  
  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - MergeHcrAndHsrl::_runFilelist()" << endl;
    cerr << "  No files found, HCR dir: "
         << _params.hcr_data_dir << endl;
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

int MergeHcrAndHsrl::_runRealtime()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  // watch for new data to arrive
  
  LdataInfo ldata(_params.hcr_data_dir,
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

int MergeHcrAndHsrl::_processFile(const string &hcrPath)
{
  
  if (_params.debug) {
    cerr << "INFO - MergeHcrAndHsrl::_processFile" << endl;
    cerr << "  HCR input path: " << hcrPath << endl;
  }
  
  // read in hcr file
  
  NcfRadxFile hcrFile;
  _setupHcrRead(hcrFile);
  RadxVol hcrVol;
  if (hcrFile.readFromPath(hcrPath, hcrVol)) {
    cerr << "ERROR - MergeHcrAndHsrl::_processFile" << endl;
    cerr << "  Cannot read in hcr file: " << hcrPath << endl;
    cerr << hcrFile.getErrStr() << endl;
    return -1;
  }
  hcrVol.convertToFl32();
  
  time_t hcrTime = hcrVol.getEndTimeSecs();
  bool dateOnly;
  if (DataFileNames::getDataTime(hcrPath, hcrTime, dateOnly)) {
    cerr << "ERROR - MergeHcrAndHsrl::_processFile" << endl;
    cerr << "  Cannot get time from file path: " << hcrPath << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Time for hcr file: " << RadxTime::strm(hcrTime) << endl;
  }

  // merge the hcr and seconday volumes, using the hcr
  // volume to hold the merged data
  
  // loop through all rays in hcr vol

  const vector<RadxRay *> &hcrRays = hcrVol.getRays();
  for (size_t ii = 0; ii < hcrRays.size(); ii++) {
    RadxRay *hcrRay = hcrRays[ii];
    // find the matching HSRL ray
    RadxRay *hsrlRay = _findHsrlRay(hcrRay);
    if (hsrlRay != NULL) {
      // merge
      _mergeRay(hcrRay, hsrlRay);
    }
  } // ii

  // finalize the volume

  hcrVol.setPackingFromRays();
  hcrVol.loadVolumeInfoFromRays();
  hcrVol.loadSweepInfoFromRays();
  hcrVol.remapToPredomGeom();
  
  // write out merged file

  if (_params.output_encoding == Params::ENCODING_INT16) {
    _hsrlVol.convertToSi16();
  }
  if (_writeVol(hcrVol)) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up read for hcr data

void MergeHcrAndHsrl::_setupHcrRead(RadxFile &file)
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }
  
  for (int ii = 0; ii < _params.hcr_fields_n; ii++) {
    file.addReadField(_params._hcr_fields[ii].input_field_name);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===== SETTING UP READ FOR HCR FILES =====" << endl;
    file.printReadRequest(cerr);
    cerr << "=============================================" << endl;
  }
  
}

/////////////////////////////////////////////////////
// Get HSRL ray for hcr ray
// Returns NULL if no suitable ray found

RadxRay *MergeHcrAndHsrl::_findHsrlRay(RadxRay *hcrRay)
  
{

  // do we have a good volume?
  
  RadxTime hcrRayTime = hcrRay->getRadxTime();
  if (_readHsrlVol(hcrRayTime)) {
    return NULL;
  }

  RadxTime hcrTime = hcrRay->getRadxTime();
  double hcrEl = hcrRay->getElevationDeg();
  
  const vector<RadxRay *> &hsrlRays = _hsrlVol.getRays();
  for (size_t jj = _hsrlRayIndex; jj < hsrlRays.size(); jj++) {
    
    RadxRay *hsrlRay = hsrlRays[jj];
    RadxTime hsrlTime = hsrlRay->getRadxTime();
    double hsrlEl = hsrlRay->getElevationDeg();
    
    double diffTime = fabs(hcrTime - hsrlTime);
    double diffEl = fabs(hcrEl - hsrlEl);
    
    if (diffTime <= _params.ray_match_time_tolerance_sec &&
        diffEl <= _params.ray_match_elevation_tolerance_deg) {

      if (_params.debug >= Params::DEBUG_EXTRA) {
        cerr << "Matched rays - hcr time,el hsrl time,el, delta time,el: "
             << hcrTime.asString(3) << ", "
             << hcrEl << ", "
             << hsrlTime.asString(3) << ", "
             << hsrlEl << ", "
             << diffTime << ", "
             << diffEl << endl;
      }
      
      _hsrlRayIndex = jj;
      return hsrlRay;

    }

  } // jj

  return NULL;

}

//////////////////////////////////////////////////
// set up read for hsrl data

void MergeHcrAndHsrl::_setupHsrlRead(RadxFile &file)
{
  
  if (_params.debug) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setVerbose(true);
  }

  for (int ii = 0; ii < _params.hsrl_fields_n; ii++) {
    file.addReadField(_params._hsrl_fields[ii].input_field_name);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===== SETTING UP READ FOR HSRL FILES =====" << endl;
    file.printReadRequest(cerr);
    cerr << "===============================================" << endl;
  }
  
}

/////////////////////////////////////////////////////
// Read HSRL volume
// Returns 0 on success, -1 on failure

int MergeHcrAndHsrl::_readHsrlVol(RadxTime &searchTime)

{

  // do we already have a suitable volume?
  
  if (searchTime >= _hsrlVolStartTime &&
      searchTime <= _hsrlVolEndTime) {
    // already have the correct file
    return 0;
  }

  // search around the search time
  
  RadxTimeList tlist;
  tlist.setDir(_params.hsrl_data_dir);
  RadxTime searchStartTime(searchTime.utime() - _params.file_match_time_tolerance_sec * 2);
  RadxTime searchEndTime(searchTime.utime() + _params.file_match_time_tolerance_sec * 2);
  tlist.setModeInterval(searchStartTime.utime(), searchEndTime.utime());
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    tlist.printRequest(cerr);
  }
  if (tlist.compile()) {
    cerr << "ERROR - MergeHcrAndHsrl::_readHsrlVol" << endl;
    cerr << "  Cannot compile hsrl file time list" << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &pathList = tlist.getPathList();
  if (pathList.size() < 1) {
    cerr << "WARNING - MergeHcrAndHsrl::_readHsrlVol()" << endl;
    cerr << "  No suitable hsrl file found" << endl;
    cerr << "  Search start time: " << searchStartTime.asString(3) << endl;
    cerr << "  Search end time: " << searchEndTime.asString(3) << endl;
    return -1;
  }

  // read in the file times, looking for a suitable file

  _hsrlVol.clear();
  _hsrlPath.clear();
  _hsrlRayIndex = 0;
  
  for (size_t ipath = 0; ipath < pathList.size(); ipath++) {

    NcfRadxFile file;
    if (_params.debug) {
      file.setDebug(true);
    }
    if (_params.debug >= Params::DEBUG_EXTRA) {
      file.setVerbose(true);
    }
    file.setReadTimesOnly(true);

    RadxVol vol;
    if (file.readFromPath(pathList[ipath], vol)) {
      cerr << "WARNING - MergeHcrAndHsrl::_readHsrlVol" << endl;
      cerr << "  Cannot read in hsrl file: " << pathList[ipath] << endl;
      cerr << file.getErrStr() << endl;
      continue;
    }
    
    if (searchTime >= vol.getStartRadxTime() &&
        searchTime <= vol.getEndRadxTime()) {
      _hsrlPath = pathList[ipath];
      break;
    }
    
  } // ipath
  
  if (_hsrlPath.size() < 1) {
    // no suitable file found
    return -1;
  }

  // read in hsrl file
  
  if (_params.debug) {
    cerr << "Found hsrl file: " << _hsrlPath << endl;
  }
  NcfRadxFile hsrlFile;
  _setupHsrlRead(hsrlFile);
  if (hsrlFile.readFromPath(_hsrlPath, _hsrlVol)) {
    cerr << "ERROR - MergeHcrAndHsrl::_readHsrlVol" << endl;
    cerr << "  Cannot read in hsrl file: " << _hsrlPath << endl;
    cerr << hsrlFile.getErrStr() << endl;
    return -1;
  }
  _hsrlVol.convertToFl32();

  _hsrlVolStartTime = _hsrlVol.getStartRadxTime();
  _hsrlVolEndTime = _hsrlVol.getEndRadxTime();

  return 0;

}
  
//////////////////////////////////////////////////////////////
// merge hcr and seconday rays
//
// Returns 0 on success, -1 on failure

void MergeHcrAndHsrl::_mergeRay(RadxRay *hcrRay,
                                const RadxRay *hsrlRay)
  
{
  
  // rename fields on hcr ray
  
  for (size_t ifld = 0; ifld < hcrRay->getNFields(); ifld++) {
    RadxField *pField = hcrRay->getField(ifld);
    for (int ii = 0; ii < _params.hcr_fields_n; ii++) {
      string inputName = _params._hcr_fields[ii].input_field_name;
      if (inputName == pField->getName()) {
        pField->setName(_params._hcr_fields[ii].output_field_name);
        break;
      }
    } // ii
  } // ifld
  hcrRay->loadFieldNameMap();

  // compute lookup in case geometry differs
  
  RadxRemap remap;
  bool geomDiffers =
    remap.checkGeometryIsDifferent(hsrlRay->getStartRangeKm(),
                                   hsrlRay->getGateSpacingKm(),
                                   hcrRay->getStartRangeKm(),
                                   hcrRay->getGateSpacingKm());
  if (geomDiffers) {
    remap.prepareForInterp(hsrlRay->getNGates(),
                           hsrlRay->getStartRangeKm(),
                           hsrlRay->getGateSpacingKm(),
                           hcrRay->getStartRangeKm(),
                           hcrRay->getGateSpacingKm());
  }
  
  const vector<RadxField *> &hsrlFields = hsrlRay->getFields();
  int nGatesHcr = hcrRay->getNGates();

  for (size_t ifield = 0; ifield < hsrlFields.size(); ifield++) {
    
    const RadxField *hsrlField = hsrlFields[ifield];

    // get output field name
    
    string outputName = hsrlField->getName();
    for (int ii = 0; ii < _params.hsrl_fields_n; ii++) {
      string inputName = _params._hsrl_fields[ii].input_field_name;
      if (inputName == outputName) {
        outputName = _params._hsrl_fields[ii].output_field_name;
        break;
      }
    }
    
    // make a copy of the field

    RadxField *fieldCopy = new RadxField(*hsrlField);

    // rename to output name
    
    fieldCopy->setName(outputName);
    
    // ensure geometry is correct, remap if needed
    
    if (geomDiffers) {
      fieldCopy->remapRayGeom(remap, true);
    }
    fieldCopy->setNGates(nGatesHcr);
      
    // add to ray

    hcrRay->addField(fieldCopy);

  } // ifield

}

//////////////////////////////////////////////////
// set up write

void MergeHcrAndHsrl::_setupWrite(RadxFile &file)
{

  if (_params.debug) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_AND_END_TIMES);
  file.setWriteCompressed(true);
  file.setCompressionLevel(_params.compression_level);
  file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  file.setNcFormat(RadxFile::NETCDF4);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===== SETTING UP WRITE FOR MERGED FILES =====" << endl;
    file.printWriteRequest(cerr);
    cerr << "===============================================" << endl;
  }
  
}

//////////////////////////////////////////////////
// write out the volume

int MergeHcrAndHsrl::_writeVol(RadxVol &vol)
{

  // output file
  
  NcfRadxFile outFile;
  _setupWrite(outFile);
  
  // write to dir
  
  if (outFile.writeToDir(vol, _params.output_dir,
                         _params.append_day_dir_to_output_dir,
                         _params.append_year_dir_to_output_dir)) {
    cerr << "ERROR - MergeHcrAndHsrl::_writeVol" << endl;
    cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
    cerr << outFile.getErrStr() << endl;
    return -1;
  }

  // write latest data info file if requested 
  
  if (_params.mode == Params::REALTIME) {
    string outputPath = outFile.getPathInUse();
    DsLdataInfo ldata(_params.output_dir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(_params.output_dir, outputPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(vol.getEndTimeSecs())) {
      cerr << "WARNING - MergeHcrAndHsrl::_writeVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  return 0;

}


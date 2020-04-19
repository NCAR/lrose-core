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
#include <radar/FilterUtils.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DataFileNames.hh>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
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

  // create map for applying flag

  _flagMap.clear();
  for (int ii = 0; ii < _params.hcr_fields_n; ii++) {
    _flagMap[_params._hcr_fields[ii].input_field_name] =
      _params._hcr_fields[ii].apply_flag;
  }

  // create set for flag checking

  _flagSet.clear();
  for (int ii = 0; ii < _params.hcr_valid_flag_values_n; ii++) {
    _flagSet.insert(_params._hcr_valid_flag_values[ii]);
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

  time_t hcrTime = hcrVol.getEndTimeSecs();
  bool dateOnly;
  if (DataFileNames::getDataTime(hcrPath, hcrTime, dateOnly)) {
    cerr << "ERROR - MergeHcrAndHsrl::_processFile" << endl;
    cerr << "  Cannot get time from file path: " << hcrPath << endl;
    return -1;
  }

  // convert all fields except flag to fl32

  string flagFieldName(_params.hcr_flag_field_name);
  const vector<RadxRay *> &hcrRays = hcrVol.getRays();
  for (size_t iray = 0; iray < hcrRays.size(); iray++) {
    RadxRay *hcrRay = hcrRays[iray];
    vector<RadxField *> hFlds = hcrRay->getFields(Radx::FIELD_RETRIEVAL_DATA);
    for (size_t ifld = 0; ifld < hFlds.size(); ifld++) {
      RadxField *hField = hFlds[ifld];
      string fieldName = hField->getName();
      if (fieldName == flagFieldName) {
        hField->convertToSi16(1.0, 0.0);
      } else {
        hField->convertToFl32();
      }
    } // ifld
  } // iray
      
  if (_params.debug) {
    cerr << "Time for hcr file: " << RadxTime::strm(hcrTime) << endl;
  }

  // apply flag field as needed to HCR volume

  if (_needFlagField) {
    _applyFlagField(hcrVol);
  }

  // determine whether the antenna is pointing or scanning

  _getScanningMode(hcrVol);

  // merge the hcr and seconday volumes, using the hcr
  // volume to hold the merged data
  
  _mergeHsrlRays(hcrVol);

  // finalize the volume

  hcrVol.setPackingFromRays();
  hcrVol.loadVolumeInfoFromRays();
  hcrVol.loadSweepInfoFromRays();
  hcrVol.remapToPredomGeom();

  // write out merged file

  if (_writeVol(hcrVol)) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up read for hcr data

void MergeHcrAndHsrl::_setupHcrRead(RadxFile &file)
{
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setDebug(true);
  }

  // determine whether (a) we need to read in the flag field and
  // (b) whether we need to write it out

  _needFlagField = false;
  _writeFlagField = false;

  // create map for applying flag

  for (int ii = 0; ii < _params.hcr_fields_n; ii++) {
    file.addReadField(_params._hcr_fields[ii].input_field_name);
    if (_params._hcr_fields[ii].apply_flag) {
      _needFlagField = true;
    }
    if (strcmp(_params._hcr_fields[ii].input_field_name,
               _params.hcr_flag_field_name) == 0) {
      _writeFlagField = true;
    }
  }
  if (_needFlagField) {
    file.addReadField(_params.hcr_flag_field_name);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===== SETTING UP READ FOR HCR FILES =====" << endl;
    file.printReadRequest(cerr);
    cerr << "=============================================" << endl;
  }
  
}

//////////////////////////////////////////////////
// apply the flag field to fields in hcr rays

void MergeHcrAndHsrl::_applyFlagField(RadxVol &hcrVol)
{
  

  // loop through all rays in hcr vol
  
  const vector<RadxRay *> &hcrRays = hcrVol.getRays();
  int nRaysMissingFlag = 0;
  for (size_t iray = 0; iray < hcrRays.size(); iray++) {
    
    RadxRay *hcrRay = hcrRays[iray];

    // get the flag field
    
    RadxField *flagField = hcrRay->getField(_params.hcr_flag_field_name);
    if (flagField == NULL) {
      nRaysMissingFlag++;
      continue;
    }
    const Radx::si16 *flagData = flagField->getDataSi16();
    Radx::si16 flagMiss = flagField->getMissingSi16();
    
    // loop through the HCR fields
    
    vector<RadxField *> hFlds = hcrRay->getFields(Radx::FIELD_RETRIEVAL_DATA);
    for (size_t ifld = 0; ifld < hFlds.size(); ifld++) {

      // do not apply flag to itself
      RadxField *hField = hFlds[ifld];
      if (hField == flagField) {
        continue;
      }

      // check if we need to apply the flag
      bool applyFlag = _flagMap[hField->getName()];
      if (!applyFlag) {
        continue;
      }

      // apply the flag to non-missing data
      
      Radx::fl32 *hData = hField->getDataFl32();
      Radx::fl32 hMiss = hField->getMissingFl32();
      
      for (size_t ii = 0; ii < hField->getNPoints(); ii++) {
        Radx::si16 flagVal = flagData[ii];
        if (flagVal == flagMiss) {
          continue;
        }
        Radx::fl32 dataVal = hData[ii];
        if (dataVal == hMiss) {
          continue;
        }
        if (_flagSet.find(flagVal) == _flagSet.end()) {
          hData[ii] = hMiss;
        }
      } // ii
      
    } // ifld

  } // iray

  if (nRaysMissingFlag > 0) {
    cerr << "WARNING - flag field missing on some rays: "
         << _params.hcr_flag_field_name << endl;
    cerr << "          n rays missing: " << nRaysMissingFlag << endl;
  }
  
}

//////////////////////////////////////////////////
// determine if antenna is pointing or scanning

void MergeHcrAndHsrl::_getScanningMode(RadxVol &hcrVol)
{

  // load up elevation angles

  vector<double> elevs;
  const vector<RadxRay *> &hcrRays = hcrVol.getRays();
  for (size_t iray = 0; iray < hcrRays.size(); iray++) {
    RadxRay *hcrRay = hcrRays[iray];
    elevs.push_back(hcrRay->getElevationDeg());
  }
  
  // compute sdev of elevation

  vector<double> elevSdev;
  FilterUtils::computeSdev(elevs, elevSdev,
                           _params.n_dwells_for_hcr_elev_sdev,
                           -9999.0);

  // set pointing flag
  
  _hcrIsPointing.resize(elevSdev.size());
  for (size_t ii = 0; ii < elevSdev.size(); ii++) {
    if (elevSdev[ii] > _params.max_hcr_elev_sdev_for_pointing) {
      _hcrIsPointing[ii] = false;
    } else {
      _hcrIsPointing[ii] = true;
    }
  }

}

//////////////////////////////////////////////////
// merge HSRL data into HCR volume
// merge the hcr and seconday volumes, using the hcr
// volume to hold the merged data

void MergeHcrAndHsrl::_mergeHsrlRays(RadxVol &hcrVol)
{
  
  // loop through all rays in hcr vol
  
  const vector<RadxRay *> &hcrRays = hcrVol.getRays();
  for (size_t iray = 0; iray < hcrRays.size(); iray++) {

    RadxRay *hcrRay = hcrRays[iray];

    // rename fields on hcr ray

    vector<RadxField *> pFlds = hcrRay->getFields(Radx::FIELD_RETRIEVAL_ALL);
    for (size_t ifld = 0; ifld < pFlds.size(); ifld++) {
      RadxField *pField = pFlds[ifld];
      for (int ifield = 0; ifield < _params.hcr_fields_n; ifield++) {
        string inputName = _params._hcr_fields[ifield].input_field_name;
        if (inputName == pField->getName()) {
          pField->setName(_params._hcr_fields[ifield].output_field_name);
          break;
        }
      } // ifield
    } // ifld

    // update the field name mapping
    
    hcrRay->loadFieldNameMap();

    // find the matching HSRL ray in time

    RadxRay *hsrlRay = _matchHsrlRayInTime(hcrRay);
    if (hsrlRay == NULL) {
      // no hsrl ray matched in time
      // add empty fields hsrl to ray
      _addEmptyHsrlFieldsToRay(hcrRay);
      continue;
    }

    // are they pointing in the same direction?

    double hcrEl = hcrRay->getElevationDeg();
    double hsrlEl = hsrlRay->getElevationDeg();
    
    if ((hcrEl > 60.0 && hsrlEl < 60.0) ||
        (hcrEl < -60.0 && hsrlEl > -60.0)) {
      // no hsrl ray matched in pointing
      // add empty fields hsrl to ray
      _addEmptyHsrlFieldsToRay(hcrRay);
      continue;
    }
      
    // if hcr is scanning, set the ray elevation
    // to the HSRL value, since we want to 
    // preserve the HSRL data integrity
    // also set the HCR data to missing
    
    if (!_hcrIsPointing[iray]) {
      hcrRay->setElevationDeg(hsrlEl);
      hcrRay->setGatesToMissing(0, hcrRay->getNGates());
    }
    
    // merge hsrl data into hcr ray

    _mergeRay(hcrRay, hsrlRay);

  } // iray

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
  for (size_t jj = 0; jj < hsrlRays.size(); jj++) {
    
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
      
      return hsrlRay;

    }

  } // jj

  return NULL;

}

/////////////////////////////////////////////////////
// Match HSRL ray with hcr ray in time
// Returns NULL if no suitable ray found

RadxRay *MergeHcrAndHsrl::_matchHsrlRayInTime(RadxRay *hcrRay)
  
{

  // do we have a good volume?
  
  RadxTime hcrRayTime = hcrRay->getRadxTime();
  if (_readHsrlVol(hcrRayTime)) {
    return NULL;
  }
  
  // find closest ray in time match
  // within the specified tolerance

  const vector<RadxRay *> &hsrlRays = _hsrlVol.getRays();
  double minDiffTime = 1.0e9;
  ssize_t minIndex = -1;

  RadxTime hcrTime = hcrRay->getRadxTime();
  RadxTime hsrlTime = hsrlRays[0]->getRadxTime();

  for (size_t jj = 0; jj < hsrlRays.size(); jj++) {
    
    RadxRay *hsrlRay = hsrlRays[jj];
    hsrlTime = hsrlRay->getRadxTime();

    double diffTime = fabs(hcrTime - hsrlTime);
    if (diffTime <= _params.ray_match_time_tolerance_sec) {
      if (diffTime < minDiffTime) {
        minIndex = jj;
        minDiffTime = diffTime;
      }
    }

  }

  if (minIndex < 0) {
    return NULL;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Matched rays in time - hcr time, hsrl time: "
         << hcrTime.asString(3) << ", "
         << hsrlTime.asString(3) << ", "
         << minDiffTime << endl;
  }
  
  return hsrlRays[minIndex];

}

//////////////////////////////////////////////////
// set up read for hsrl data

void MergeHcrAndHsrl::_setupHsrlRead(RadxFile &file)
{
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setDebug(true);
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

  // have we already searched this period

  if (_searchFailureLowerLimit.utime() > 0 &&
      _searchFailureUpperLimit.utime() > 0) {
    if (searchTime >= _searchFailureLowerLimit &&
        searchTime <= _searchFailureUpperLimit) {
      return -1;
    }
  }

  // search around the search time
  
  RadxTimeList tlist;
  tlist.setDir(_params.hsrl_data_dir);
  RadxTime searchStartTime(searchTime.utime() -
                           _params.file_match_time_tolerance_sec * 2);
  RadxTime searchEndTime(searchTime.utime() +
                         _params.file_match_time_tolerance_sec * 2);
  tlist.setModeInterval(searchStartTime.utime(), searchEndTime.utime());
  if (_params.debug >= Params::DEBUG_EXTRA) {
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
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - MergeHcrAndHsrl::_readHsrlVol()" << endl;
      cerr << "  No suitable hsrl file found" << endl;
      cerr << "  Search start time: " << searchStartTime.asString(3) << endl;
      cerr << "  Search end time: " << searchEndTime.asString(3) << endl;
    }
    // set search failure limits so we don't search this period again
    _searchFailureLowerLimit.set(searchTime.utime() -
                                 _params.file_match_time_tolerance_sec);
    _searchFailureUpperLimit.set(searchTime.utime() +
                                 _params.file_match_time_tolerance_sec);
    return -1;
  }

  // read in the file times, looking for a suitable file

  _hsrlVol.clear();
  _hsrlPath.clear();
  
  for (size_t ipath = 0; ipath < pathList.size(); ipath++) {

    NcfRadxFile file;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      file.setDebug(true);
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
    // set search failure limits so we don't search this period again
    _searchFailureLowerLimit.set(searchTime.utime() -
                                 _params.file_match_time_tolerance_sec);
    _searchFailureUpperLimit.set(searchTime.utime() +
                                 _params.file_match_time_tolerance_sec);
    return -1;
  }

  // clear the search faiure limits

  _searchFailureLowerLimit.set(-1);
  _searchFailureUpperLimit.set(-1);

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

  // save the names and units of the HSRL fields
  // so that we can use them for missing rays

  _hsrlFieldNames.clear();
  _hsrlFieldLongNames.clear();
  _hsrlFieldUnits.clear();

  _hsrlFieldNames.resize(_params.hsrl_fields_n);
  _hsrlFieldLongNames.resize(_params.hsrl_fields_n);
  _hsrlFieldUnits.resize(_params.hsrl_fields_n);
  
  if (_hsrlVol.getNRays() > 0) {
    const RadxRay *ray1 = _hsrlVol.getRays()[0];
    vector<RadxField *> hflds = ray1->getFields();
    for (size_t ifld = 0; ifld < hflds.size(); ifld++) {
      const RadxField *hField = hflds[ifld];
      for (int ii = 0; ii < _params.hsrl_fields_n; ii++) {
        string inputName = _params._hsrl_fields[ii].input_field_name;
        if (inputName == hField->getName()) {
          _hsrlFieldNames[ii] = _params._hsrl_fields[ii].output_field_name;
          _hsrlFieldLongNames[ii] = hField->getLongName();
          _hsrlFieldUnits[ii] = hField->getUnits();
        }
      } // ii
    } // ifld
  } // if (_hsrlVol.getNRays() > 0)

  return 0;
  
}
  
//////////////////////////////////////////////////////////////
// merge hcr and seconday rays
// Returns 0 on success, -1 on failure

void MergeHcrAndHsrl::_mergeRay(RadxRay *hcrRay,
                                RadxRay *hsrlRay)
  
{
  
  // adjust HSRL ray geom to account for elevation being non-vertical

  double elCorr = fabs(sin(hsrlRay->getElevationDeg() * DEG_TO_RAD));
  hsrlRay->setRangeGeom(hsrlRay->getStartRangeKm() * elCorr,
                        hsrlRay->getGateSpacingKm() * elCorr);
  
  // compute lookup for matching in range
  
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
  
  vector<RadxField *> hsrlFields = hsrlRay->getFields();
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

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "+";
  }

}

//////////////////////////////////////////////////////////////
// Add empty hsrl fields to hcr ray

void MergeHcrAndHsrl::_addEmptyHsrlFieldsToRay(RadxRay *hcrRay)
  
{

  // loop through specified HSRL fields
  
  for (size_t ifield = 0; ifield < _hsrlFieldNames.size(); ifield++) {
    
    RadxField *empty = new RadxField(_hsrlFieldNames[ifield],
                                     _hsrlFieldUnits[ifield]);
    empty->setLongName(_hsrlFieldLongNames[ifield]);
    empty->setTypeFl32(Radx::missingFl32);
    empty->addDataMissing(hcrRay->getNGates());

    hcrRay->addField(empty);
    
  } // ifield
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-";
  }

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
  file.setCompressionLevel(4);
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


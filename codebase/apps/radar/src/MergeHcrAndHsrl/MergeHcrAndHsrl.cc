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
#include <Radx/RadxVol.hh>
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

  // Search for hsrl files

  time_t searchTime = hcrTime;
  RadxTimeList tlist;
  tlist.setDir(_params.hsrl_data_dir);
  tlist.setModeClosest(searchTime, _params.file_match_time_tolerance_sec);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    tlist.printRequest(cerr);
  }
    
  if (tlist.compile()) {
    cerr << "ERROR - MergeHcrAndHsrl::_processFile()" << endl;
    cerr << "  Cannot compile hsrl file time list" << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }
  const vector<string> &pathList = tlist.getPathList();
  if (pathList.size() < 1) {
    cerr << "WARNING - MergeHcrAndHsrl::_processFile()" << endl;
    cerr << "  No suitable hsrl file found" << endl;
    cerr << "  Hcr file: " << hcrPath << endl;
    return -1;
  }
  
  // read in hsrl file, using first path in list
  
  string hsrlPath = pathList[0];
  if (_params.debug) {
    cerr << "Found hsrl file: " << hsrlPath << endl;
  }
  NcfRadxFile hsrlFile;
  _setupHsrlRead(hsrlFile);
  RadxVol hsrlVol;
  if (hsrlFile.readFromPath(hsrlPath, hsrlVol)) {
    cerr << "ERROR - MergeHcrAndHsrl::_processFile" << endl;
    cerr << "  Cannot read in hsrl file: " << hsrlPath << endl;
    cerr << hsrlFile.getErrStr() << endl;
    return -1;
  }
  
  // merge the hcr and seconday volumes, using the hcr
  // volume to hold the merged data
  
  if (_mergeVol(hcrVol, hsrlVol)) {
    cerr << "ERROR - MergeHcrAndHsrl::_processFile" << endl;
    cerr << "  Merge failed" << endl;
    cerr << "  Hcr file: " << hcrPath << endl;
    cerr << "  Hsrl file: " << hsrlPath << endl;
    return -1;
  }

  // finalize the volume

  hcrVol.setPackingFromRays();
  hcrVol.loadVolumeInfoFromRays();
  hcrVol.loadSweepInfoFromRays();
  hcrVol.remapToPredomGeom();
  
  // write out file

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

//////////////////////////////////////////////////////////////
// merge the hcr and hsrl volumes, using the hcr
// volume to hold the merged data
//
// Returns 0 on success, -1 on failure

int MergeHcrAndHsrl::_mergeVol(RadxVol &hcrVol,
                               const RadxVol &hsrlVol)

{

  // loop through all rays in hcr vol

  const vector<RadxRay *> &pRays = hcrVol.getRays();
  int searchStart = 0;
  vector<RadxRay *> mergedRays;
  vector<RadxRay *> unusedRays;

  for (size_t ii = 0; ii < pRays.size(); ii++) {
    
    RadxRay *pRay = pRays[ii];
    RadxTime pTime = pRay->getRadxTime();
    double pEl = pRay->getElevationDeg();

    // find matching ray in hsrl volume
    
    const vector<RadxRay *> &sRays = hsrlVol.getRays();
    bool found = false;
    for (size_t jj = searchStart; jj < sRays.size(); jj++) {
      
      RadxRay *sRay = sRays[jj];
      RadxTime sTime = sRay->getRadxTime();
      double sEl = sRay->getElevationDeg();
      
      double diffTime = fabs(pTime - sTime);
      double diffEl = fabs(pEl - sEl);

      if (diffTime <= _params.ray_match_time_tolerance_sec &&
          diffEl <= _params.ray_match_elevation_tolerance_deg) {
        // rays match, merge the rays
        _mergeRay(*pRay, *sRay);
        mergedRays.push_back(pRay);
        found = true;
        if (_params.debug >= Params::DEBUG_EXTRA) {
          cerr << "Matched ray - time el el2 dEl dTime: "
               << pTime.asString(3) << ", "
               << pEl << ", "
               << sEl << ", "
               << diffEl << ", "
               << diffTime << endl;
        }
        break;
      }
      
    } // jj

    if (!found) {
      unusedRays.push_back(pRay);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "====>>> missed merge, time el: "
             << pTime.asString(3) << ", "
             << pEl << endl;
      }
    }

  } // ii

  // clean up unused rays

  hcrVol.removeBadRays(mergedRays, unusedRays);

  return 0;

}

//////////////////////////////////////////////////////////////
// merge hcr and seconday rays
//
// Returns 0 on success, -1 on failure

void MergeHcrAndHsrl::_mergeRay(RadxRay &hcrRay,
                                const RadxRay &hsrlRay)
  
{

  // rename fields on hcr ray
  
  for (size_t ifld = 0; ifld < hcrRay.getNFields(); ifld++) {
    RadxField *pField = hcrRay.getField(ifld);
    for (int ii = 0; ii < _params.hcr_fields_n; ii++) {
      string inputName = _params._hcr_fields[ii].input_field_name;
      if (inputName == pField->getName()) {
        pField->setName(_params._hcr_fields[ii].output_field_name);
        break;
      }
    } // ii
  } // ifld
  hcrRay.loadFieldNameMap();

  // compute lookup in case geometry differs
  
  RadxRemap remap;
  bool geomDiffers =
    remap.checkGeometryIsDifferent(hsrlRay.getStartRangeKm(),
                                   hsrlRay.getGateSpacingKm(),
                                   hcrRay.getStartRangeKm(),
                                   hcrRay.getGateSpacingKm());
  if (geomDiffers) {
    remap.prepareForInterp(hsrlRay.getNGates(),
                           hsrlRay.getStartRangeKm(),
                           hsrlRay.getGateSpacingKm(),
                           hcrRay.getStartRangeKm(),
                           hcrRay.getGateSpacingKm());
  }
  
  const vector<RadxField *> &sFields = hsrlRay.getFields();
  int nGatesHcr = hcrRay.getNGates();

  for (size_t ifield = 0; ifield < sFields.size(); ifield++) {
    
    const RadxField *sField = sFields[ifield];

    // get output field name
    
    string outputName = sField->getName();
    Params::output_encoding_t outputEncoding = Params::ENCODING_INT16;
    for (int ii = 0; ii < _params.hsrl_fields_n; ii++) {
      string inputName = _params._hsrl_fields[ii].input_field_name;
      if (inputName == outputName) {
        outputName = _params._hsrl_fields[ii].output_field_name;
        outputEncoding = _params._hsrl_fields[ii].output_encoding;
        break;
      }
    }
    
    // make a copy of the field

    RadxField *sCopy = new RadxField(*sField);

    // rename to output name
    
    sCopy->setName(outputName);
    
    // ensure geometry is correct, remap if needed
    
    if (geomDiffers) {
      sCopy->remapRayGeom(remap, true);
    }
    sCopy->setNGates(nGatesHcr);
      
    // convert type

    switch (outputEncoding) {
      case Params::ENCODING_FLOAT32:
        sCopy->convertToFl32();
        break;
      case Params::ENCODING_INT32:
        sCopy->convertToSi32();
        break;
      case Params::ENCODING_INT08:
        sCopy->convertToSi08();
        break;
      case Params::ENCODING_INT16:
      default:
        sCopy->convertToSi16();
        break;
    } // switch

    // add to ray

    hcrRay.addField(sCopy);

  } // ifield

}


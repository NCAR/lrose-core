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
// RadxDiffFields.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2016
//
///////////////////////////////////////////////////////////////
//
// RadxDiffFields computes statistics about the difference
// between fields in Radx files.
// The fields can be in the same file, or in different files.
// The results are written to SPDB as XML.
//
///////////////////////////////////////////////////////////////

#include "RadxDiffFields.hh"
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaArray.hh>
#include <didss/LdataInfo.hh>
#include <rapmath/trig.h>
#include <dsserver/DsLdataInfo.hh>
#include <Spdb/DsSpdb.hh>
#include <rapformats/WxObs.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxXml.hh>
#include <algorithm>
using namespace std;

// Constructor

RadxDiffFields::RadxDiffFields(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "RadxDiffFields";
  
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

  // set up field diffs

  for (int ii = 0; ii < _params.field_pairs_n; ii++) {
    const Params::field_pair_t &pair = _params._field_pairs[ii];
    FieldDiff *diff = new FieldDiff(pair);
    _fields.push_back(diff);
  }

}

// destructor

RadxDiffFields::~RadxDiffFields()

{

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    delete _fields[ii];
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RadxDiffFields::Run()
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

int RadxDiffFields::_runFilelist()
{

  // loop through the input file list

  int iret = 0;

  for (int ifile = 0; ifile < (int) _args.inputFileList.size(); ifile++) {

    string inputPath = _args.inputFileList[ifile];
    if (_processPrimaryFile(inputPath)) {
      iret = -1;
    }

  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int RadxDiffFields::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.primary_input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - RadxDiffFields::_runArchive()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.primary_input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxDiffFields::_runArchive()" << endl;
    cerr << "  No files found, dir: " << _params.primary_input_dir << endl;
    return -1;
  }
  
  // loop through the input file list
  
  int iret = 0;
  for (size_t ipath = 0; ipath < paths.size(); ipath++) {
    if (_processPrimaryFile(paths[ipath])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode

int RadxDiffFields::_runRealtime()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  // watch for new data to arrive

  LdataInfo ldata(_params.primary_input_dir,
                  _params.debug >= Params::DEBUG_VERBOSE);
  
  int iret = 0;

  while (true) {
    ldata.readBlocking(_params.max_realtime_data_age_secs,
                       1000, PMU_auto_register);
    
    const string path = ldata.getDataPath();
    if (_processPrimaryFile(path)) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Process a file
// Returns 0 on success, -1 on failure

int RadxDiffFields::_processPrimaryFile(const string &filePath)
{

  if (_params.debug) {
    cerr << "INFO - RadxDiffFields::_processPrimaryFile" << endl;
    cerr << "  Input path: " << filePath << endl;
  }
  
  // read in primary file
  
  GenericRadxFile inFile;
  _setupPrimaryRead(inFile);
  RadxVol vol;
  if (inFile.readFromPath(filePath, vol)) {
    cerr << "ERROR - RadxDiffFields::_processPrimaryFile" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();
  
  // convert all data to floats

  vol.convertToFl32();

  // create set of secondary directories

  set<string> secondaryDirs;
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    const Params::field_pair_t &pair = _fields[ii]->param;
    if (!pair.fields_are_in_same_file) {
      secondaryDirs.insert(pair.secondary_input_dir);
    }
  }

  // merge fields from secondary files

  for (set<string>::iterator ii = secondaryDirs.begin();
       ii != secondaryDirs.end(); ii++) {
    if (_addSecondaryFields(vol, *ii)) {
      cerr << "ERROR - RadxDiffFields::_processPrimaryFile" << endl;
      cerr << "  Cannot read fields from secondary dir: " << *ii << endl;
      return -1;
    }
  }

  // load up diffs between fields

  if (_loadDiffs(vol)) {
    cerr << "ERROR - RadxDiffFields::_processPrimaryFile" << endl;
    cerr << "  Cannot load diffs" << endl;
    return -1;
  }

  // compute stats on diffs

  _computeDiffStats();

  // write out

  if (_writeResults(vol)) {
    cerr << "ERROR - RadxDiffFields::_processPrimaryFile" << endl;
    cerr << "  Cannot write results to SPDB" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up primary read

void RadxDiffFields::_setupPrimaryRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.set_fixed_angle_limits) {
    file.setReadFixedAngleLimits(_params.lower_fixed_angle_limit,
                                 _params.upper_fixed_angle_limit);
  }

  for (int ii = 0; ii < _params.field_pairs_n; ii++) {
    const Params::field_pair_t &pair = _params._field_pairs[ii];
    file.addReadField(pair.primary_field_name);
    if (pair.fields_are_in_same_file) {
      file.addReadField(pair.secondary_field_name);
    }
  }

  if (_params.apply_condition_check) {
    file.addReadField(_params.condition_field_name);
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }
  
}

//////////////////////////////////////////////////
// set up secondary read

void RadxDiffFields::_setupSecondaryRead(RadxFile &file,
                                         const string &secondaryDir)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.set_fixed_angle_limits) {
    file.setReadFixedAngleLimits(_params.lower_fixed_angle_limit,
                                 _params.upper_fixed_angle_limit);
  }

  for (int ii = 0; ii < _params.field_pairs_n; ii++) {
    const Params::field_pair_t &pair = _params._field_pairs[ii];
    string dir = pair.secondary_input_dir;
    if (dir == secondaryDir) {
      file.addReadField(pair.secondary_field_name);
    }
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }
  
}

//////////////////////////////////////////////////
// Add secondary fields to vol

int RadxDiffFields::_addSecondaryFields(RadxVol &primaryVol,
                                        const string &secondaryDir)
{

  // Search for file from which to merge data
  
  time_t searchTime = primaryVol.getStartTimeSecs();
  RadxTimeList tlist;
  tlist.setDir(secondaryDir);
  tlist.setModeClosest(searchTime, _params.secondary_file_time_tolerance_sec);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    tlist.printRequest(cerr);
  }
  
  if (tlist.compile()) {
    cerr << "ERROR - RadxDiffFields::_addSecondaryFields()" << endl;
    cerr << "  Cannot compile secondary file time list" << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }
  const vector<string> &pathList = tlist.getPathList();
  if (pathList.size() < 1) {
    cerr << "ERROR - RadxDiffFields::_addSecondaryFields()" << endl;
    cerr << "  No suitable secondary file found" << endl;
    cerr << "  Primary file: " << primaryVol.getPathInUse() << endl;
    return -1;
  }
  
  // read in secondary file, using first path in list
  
  string secondaryPath = pathList[0];
  if (_params.debug) {
    cerr << "Found secondary file: " << secondaryPath << endl;
  }
  GenericRadxFile secondaryFile;
  _setupSecondaryRead(secondaryFile, secondaryDir);
  RadxVol secondaryVol;
  if (secondaryFile.readFromPath(secondaryPath, secondaryVol)) {
    cerr << "ERROR - RadxDiffFields::_addSecondaryFields()" << endl;
    cerr << "  Cannot read in secondary file: " << secondaryPath << endl;
    cerr << secondaryFile.getErrStr() << endl;
    return -1;
  }
  secondaryVol.convertToFl32();
  
  // merge the primary and secondary volumes, using the primary
  // volume to hold the secondary data
  
  if (_mergeVol(primaryVol, secondaryVol)) {
    cerr << "ERROR - RadxDiffFields::_addSecondaryFields()" << endl;
    cerr << "  Merge failed" << endl;
    cerr << "  Primary file: " << primaryVol.getPathInUse() << endl;
    cerr << "  Secondary file: " << secondaryPath << endl;
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// Perform the merge
// Returns 0 on success, -1 on failure

int RadxDiffFields::_mergeVol(const RadxVol &primaryVol, RadxVol &secondaryVol)

{

  // loop through all rays in primary vol

  const vector<RadxRay *> &pRays = primaryVol.getRays();
  int searchStart = 0;
  vector<RadxRay *> secondaryRays;

  for (size_t ii = 0; ii < pRays.size(); ii++) {

    RadxRay *pRay = pRays[ii];
    double pTime = (double) pRay->getTimeSecs() + pRay->getNanoSecs() / 1.0e9;
    double pAz = pRay->getAzimuthDeg();   
    double pEl = pRay->getElevationDeg();

    int pMilli = pRay->getNanoSecs() / 1.0e6;
    char pMStr[16];
    sprintf(pMStr, "%.3d", pMilli);

    // find matching ray in secondary volume

    const vector<RadxRay *> &sRays = secondaryVol.getRays();
    for (size_t jj = searchStart; jj < sRays.size(); jj++) {
      
      RadxRay *sRay = sRays[jj];
      double sTime = (double) sRay->getTimeSecs() + sRay->getNanoSecs() / 1.0e9;
      double sAz = sRay->getAzimuthDeg();   
      double sEl = sRay->getElevationDeg();

      int sMilli = sRay->getNanoSecs() / 1.0e6;
      char sMStr[16];
      sprintf(sMStr, "%.3d", sMilli);

      double diffTime = fabs(pTime - sTime);
      double dAz = pAz - sAz;
      if (dAz < -180) {
        dAz += 360.0;
      } else if (dAz > 180) {
        dAz -= 360.0;
      }
      double diffAz = fabs(dAz);
      double diffEl = fabs(pEl - sEl);

      if (diffTime <= _params.secondary_ray_time_tolerance_sec &&
          diffAz <= _params.secondary_ray_azimuth_tolerance_deg &&
          diffEl <= _params.secondary_ray_elevation_tolerance_deg) {
        // same ray, merge the rays
        _mergeRay(*pRay, *sRay);
        secondaryRays.push_back(pRay);
        if (_params.debug >= Params::DEBUG_EXTRA) {
          cerr << "Matched ray - time az el az2 el2 dEl dAz dTime: "
               << RadxTime::strm((time_t) pTime) << "." << pMStr << ", "
               << pAz << ", " << pEl << ", "
               << sAz << ", " << sEl << ", "
               << diffEl << ", "
               << diffAz << ", "
               << diffTime << endl;
        }
        break;
      }
      
    } // jj

  } // ii

  return 0;

  RadxRay primaryRay, secondaryRay;

  // check ray range geom

  return 0;

}

//////////////////////////////////////////////////////////////
// merge primary and seconday rays
//
// Returns 0 on success, -1 on failure

void RadxDiffFields::_mergeRay(RadxRay &primaryRay,
                               const RadxRay &secondaryRay)
  
{

  // compute lookup in case geometry differs

  RadxRemap remap;
  bool geomDiffers =
    remap.checkGeometryIsDifferent(secondaryRay.getStartRangeKm(),
                                   secondaryRay.getGateSpacingKm(),
                                   primaryRay.getStartRangeKm(),
                                   primaryRay.getGateSpacingKm());
  if (geomDiffers) {
    remap.prepareForInterp(secondaryRay.getNGates(),
                           secondaryRay.getStartRangeKm(),
                           secondaryRay.getGateSpacingKm(),
                           primaryRay.getStartRangeKm(),
                           primaryRay.getGateSpacingKm());
  }
  
  const vector<RadxField *> &sFields = secondaryRay.getFields();
  int nGatesPrimary = primaryRay.getNGates();
  
  for (size_t ifield = 0; ifield < sFields.size(); ifield++) {
    
    const RadxField *sField = sFields[ifield];
    
    // modify the field name
    
    string secondaryName = "sec_";
    secondaryName += sField->getName();
    
    // make a copy of the field

    RadxField *sCopy = new RadxField(*sField);

    // set name
    
    sCopy->setName(secondaryName);
    
    // ensure geometry is correct, remap if needed
    
    if (geomDiffers) {
      sCopy->remapRayGeom(remap, true);
    }
    sCopy->setNGates(nGatesPrimary);
      
    // add to ray

    primaryRay.addField(sCopy);

  } // ifield
  
  primaryRay.loadFieldNameMap();

}

//////////////////////////////////////////////////
// load up the diffs on the volume
// returns 0 on success, -1 on error

int RadxDiffFields::_loadDiffs(RadxVol &vol)
{

  // clear results vector

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->clear();
  }
  
  // loop through the rays, loading up the diffs
  
  for (size_t iray = 0; iray < vol.getNRays(); iray++) {
    
    const RadxRay *ray = vol.getRays()[iray];

    if (_loadDiffs(ray)) {
      return -1;
    }
    
  }

  return 0;

}

//////////////////////////////////////////////////
// load up the diffs on a ray
// returns 0 on success, -1 on error

int RadxDiffFields::_loadDiffs(const RadxRay *ray)
{

  // get condition field

  const RadxField *cField = NULL;
  if (_params.apply_condition_check) {
    cField = ray->getField(_params.condition_field_name);
    if (cField == NULL) {
      cerr << "ERROR - RadxDiffFields::_loadDiffs" << endl;
      cerr << "  Cannot find condition field: "
           << _params.condition_field_name << endl;
      return -1;
    }
  }
  
  // loop through fields

  for (size_t ii = 0; ii < _fields.size(); ii++) {

    FieldDiff &field = *_fields[ii];

    const RadxField *pField = ray->getField(field.primaryName);
    const RadxField *sField = ray->getField(field.secondaryName);
    
    if (pField == NULL) {
      cerr << "ERROR - RadxDiffFields::_loadDiffs" << endl;
      cerr << "  Cannot find primary field: " << field.primaryName << endl;
      return -1;
    }

    if (sField == NULL) {
      cerr << "ERROR - RadxDiffFields::_loadDiffs" << endl;
      cerr << "  Cannot find secondary field: " << field.secondaryName << endl;
      return -1;
    }

    _loadDiffs(field, *pField, *sField, cField);

  }

  return 0;

}

//////////////////////////////////////////////////
// load up the diffs for the field pair

void RadxDiffFields::_loadDiffs(FieldDiff &fieldDiff,
                                const RadxField &pField,
                                const RadxField &sField,
                                const RadxField *cField)
  
{

  Radx::fl32 pMiss = pField.getMissingFl32();
  Radx::fl32 sMiss = sField.getMissingFl32();
  Radx::fl32 cMiss = Radx::getMissingFl32();

  const Radx::fl32 *pData = pField.getDataFl32();
  const Radx::fl32 *sData = sField.getDataFl32();
  const Radx::fl32 *cData = NULL;

  if (cField != NULL) {
    cMiss = cField->getMissingFl32();
    cData = cField->getDataFl32();
  }
  
  size_t nGates = pField.getNPoints();
  if (nGates > sField.getNPoints()) {
    nGates = sField.getNPoints();
  }

  for (size_t ii = 0; ii < nGates; ii++) {

    // check for condition

    if (cData != NULL) {
      Radx::fl32 cVal = cData[ii];
      if (!std::isfinite(cVal) ||
          cVal == cMiss ||
          cVal < _params.condition_field_min_value ||
          cVal > _params.condition_field_max_value) {
        continue;
      }
    }

    Radx::fl32 pVal = pData[ii];
    Radx::fl32 sVal = sData[ii];

    if (std::isfinite(pVal) && std::isfinite(sVal) &&
        pVal != pMiss && sVal != sMiss) {
      double diff = pVal - sVal;
      fieldDiff.diffs.push_back(diff);
    }

  } // ii

}

//////////////////////////////////////////////////
// compute the stats on the diffs

void RadxDiffFields::_computeDiffStats()

{

  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    
    FieldDiff &field = *_fields[ifield];
    if (field.diffs.size() < 1) {
      continue;
    }

    // mean

    double sum = 0.0;
    int nn = 0;
    for (size_t ii = 0; ii < field.diffs.size(); ii++) {
      sum += field.diffs[ii];
      nn++;
    }

    field.diffMean = sum / (double) nn;

    // percentiles

    sort(field.diffs.begin(), field.diffs.end());
    
    for (int ii = 0; ii < _params.result_percentiles_n; ii++) {
      double perc = _computePerc(field, _params._result_percentiles[ii]);
      field.diffPercentiles.push_back(perc);
    }

  } // ifield

}

/////////////////////////////////////////////////////////////
// compute a percentile value from the sorted bias data

double RadxDiffFields::_computePerc(FieldDiff &field,
                                    double percent)

{

  // get center of percentile location in the array
  
  double nVals = field.diffs.size();
  int pos = (int) ((percent / 100.0) * nVals + 0.5);
  if (pos < 0) {
    pos = 0;
  } else if (pos > (int) field.diffs.size() - 1) {
    pos = field.diffs.size() - 1;
  }
  
  double percentileVal = field.diffs[pos];

  // compute mean for 1% on either side

  int nMargin = (int) (nVals / 100.0 + 0.5);
  int istart = pos - nMargin;
  if (istart < 0) {
    istart = 0;
  }
  int iend = pos + nMargin;
  if (iend > (int) field.diffs.size() - 1) {
    iend = field.diffs.size() - 1;
  }

  double sum = 0.0;
  double count = 0.0;
  for (int ii = istart; ii <= iend; ii++) {
    sum += field.diffs[ii];
    count++;
  }

  if (count > 0) {
    percentileVal = sum / count;
  }

  return percentileVal;

}

//////////////////////////////////////////////////
// write the results to SPDB

int RadxDiffFields::_writeResults(const RadxVol &vol)

{

  DsSpdb spdb;
  RadxPath vpath(vol.getPathInUse());
  RadxTime vtime(vol.getStartTimeSecs());

  string xml;
  xml += RadxXml::writeStartTag("RadxDiffFields", 0);

  xml += RadxXml::writeString("file", 1, vpath.getFile());
  xml += RadxXml::writeString("time", 1, vtime.asString(6));
  
  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {

    FieldDiff &field = *_fields[ifield];
    string name = field.primaryName;
    string tagStart = name + "_";

    xml += RadxXml::writeBoolean(tagStart + "isRhi", 1, vol.checkIsRhi());
    xml += RadxXml::writeString(tagStart + "secondaryName", 1,
                                field.param.secondary_field_name);
    xml += RadxXml::writeInt(tagStart + "nPts", 1, field.diffs.size());
    xml += RadxXml::writeDouble(tagStart + "diffMean", 1, field.diffMean);

    if ((int) field.diffPercentiles.size() == _params.result_percentiles_n) {
      for (int ii = 0; ii < _params.result_percentiles_n; ii++) {
        int percent = _params._result_percentiles[ii];
        double val = field.diffPercentiles[ii];
        char tag[1024];
        sprintf(tag, "%sdiffPerc%.2d", tagStart.c_str(), percent);
        xml += RadxXml::writeDouble(tag, 1, val);
      } // ii
    } // if (field.diffPercentiles.size() ....
      
  } // ifield

  xml += RadxXml::writeEndTag("RadxDiffFields", 0);

  if (_params.debug) {
    cerr << "Results XML: " << endl;
    cerr << xml << endl;
  }

  if (_params.write_results_to_spdb) {

    // create the chunk
    
    int dataType = 0;
    int dataType2 = 0;
    
    spdb.addPutChunk(dataType,
                     vtime.utime(),
                     vtime.utime(),
                     xml.size() + 1,
                     xml.c_str(),
                     dataType2);
    
    if (_params.debug) {
      cerr << "Writing results to SPDB, url: " << _params.spdb_output_url << endl;
    }
    
    // do the write

    if (spdb.put(_params.spdb_output_url,
                 SPDB_XML_ID, SPDB_XML_LABEL)) {
      cerr << "ERROR - RadxDiffFields::_writeResults" << endl;
      cerr << spdb.getErrStr() << endl;
      return -1;
    }

  } // if (_params.write_results_to_spdb)

  return 0;
  
}

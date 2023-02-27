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
// FixFieldVals.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2010
//
///////////////////////////////////////////////////////////////

#include "FixFieldVals.hh"
#include <Radx/Radx.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxCfactors.hh>
#include <Spdb/DsSpdb.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/TaFile.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <cerrno>
#include <set>
using namespace std;

// Constructor

FixFieldVals::FixFieldVals(int argc, char **argv)
  
{

  OK = TRUE;
  _nWarnCensorPrint = 0;

  // set programe name

  _progName = "FixFieldVals";
  ucopyright((char *) _progName.c_str());
  
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

  // override missing values

  if (_params.override_missing_metadata_values) {
    Radx::setMissingMetaDouble(_params.missing_metadata_double);
    Radx::setMissingMetaFloat(_params.missing_metadata_float);
    Radx::setMissingMetaInt(_params.missing_metadata_int);
    Radx::setMissingMetaChar(_params.missing_metadata_char);
  }
  if (_params.override_missing_field_values) {
    Radx::setMissingFl64(_params.missing_field_fl64);
    Radx::setMissingFl32(_params.missing_field_fl32);
    Radx::setMissingSi32(_params.missing_field_si32);
    Radx::setMissingSi16(_params.missing_field_si16);
    Radx::setMissingSi08(_params.missing_field_si08);
  }

  // initialize field diffs array
  
  for (int ii = 0; ii < _params.comparison_fields_n; ii++) {
    
    string corrFieldName = _params._comparison_fields[ii].correction_field_name;
    string truthFieldName = _params._comparison_fields[ii].truth_field_name;

    _fieldDiffs.push_back(FieldDiff(corrFieldName, truthFieldName));

  }

}

// destructor

FixFieldVals::~FixFieldVals()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int FixFieldVals::Run()
{

  vector<string> inputPaths;
  
  if (_params.mode == Params::FILELIST) {

    inputPaths = _args.inputFileList;

    if (inputPaths.size() < 1) {
      cerr << "ERROR - FixFieldVals::Run()" << endl;
      cerr << "  FILELIST mode - no files found on command line" << endl;
      return -1;
    }
    
  } else {
    
    // get the files to be processed
    
    RadxTimeList tlist;
    tlist.setDir(_params.correction_input_dir);
    tlist.setModeInterval(_args.startTime, _args.endTime);
    if (tlist.compile()) {
      cerr << "ERROR - FixFieldVals::_runArchive()" << endl;
      cerr << "  Cannot compile time list, dir: " << _params.correction_input_dir << endl;
      cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
      cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
      cerr << tlist.getErrStr() << endl;
      return -1;
    }
    
    inputPaths = tlist.getPathList();
    if (inputPaths.size() < 1) {
      cerr << "ERROR - FixFieldVals::Run()" << endl;
      cerr << "  ARCHIVE mode - no files found, dir: "
           << _params.correction_input_dir << endl;
      return -1;
    }
    
  }

  if (_params.processing_stage == Params::ANALYSIS_STAGE) {
    return _analyze(inputPaths);
  } else {
    return _correct(inputPaths);
  }

}

/////////////////////////////////
// analyze the field differences

int FixFieldVals::_analyze(const vector<string> &inputPaths)

{
  
  int iret = 0;
  
  if (_params.debug) {
    cerr << "Running FixFieldVals::_analyze" << endl;
    cerr << "  n input files: " << inputPaths.size() << endl;
  }
  
  int nGood = 0;
  int nError = 0;
  
  // loop through the input file list
  
  RadxVol vol;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    vol.setDebug(true);
  }
  for (int ii = 0; ii < (int) inputPaths.size(); ii++) {
    string inputPath = inputPaths[ii];
    // read input file
    int jret = _readFile(inputPath, vol);
    if (jret == 0) {
      // analyze the file for bias compared with truth files
      if (_analyzeVol(vol)) {
        iret = -1;
        nError++;
        if (_params.debug) {
          cerr << "  ====>> n errors so far: " << nError << endl;
        }
      }
    } else if (jret < 0) {
      iret = -1;
      nError++;
      if (_params.debug) {
        cerr << "  ====>> n errors so far: " << nError << endl;
      }
    }
    // free up
    vol.clear();
  }
  
  if (_params.debug) {
    cerr << "FixFieldVals done" << endl;
    cerr << "====>> n good files processed: " << nGood << endl;
  }

  return iret;

}

//////////////////////////////////////////////////
// Run correction stage

int FixFieldVals::_correct(const vector<string> &inputPaths)
{

  // loop through the input file list

  RadxVol vol;
  int iret = 0;
  for (size_t ii = 0; ii < inputPaths.size(); ii++) {
    // read input file
    int jret = _readFile(inputPaths[ii], vol);
    if (jret == 0) {
      // finalize the volume
      _finalizeVol(vol);
      // write the volume out
      if (_writeVol(vol)) {
        cerr << "ERROR - FixFieldVals::_runArchive" << endl;
        cerr << "  Cannot write volume to file" << endl;
        return -1;
      }
    } else if (jret < 0) {
      iret = -1;
    }
    // free up
    vol.clear();
  }

  return iret;

}

//////////////////////////////////////////////////
// Read in a file
// accounting for special cases such as gematronik
// Returns 0 on success
//         1 if already read,
//         -1 on failure

int FixFieldVals::_readFile(const string &readPath,
                           RadxVol &vol)
{

  PMU_auto_register("Processing file");

  // clear all data on volume object

  vol.clear();

  // check we have not already processed this file
  // in the file aggregation step
  // if not clear _readPaths

  RadxPath rpath(readPath);
  for (auto ii = _readPaths.begin(); ii != _readPaths.end(); ii++) {
    RadxPath tpath(*ii);
    if (rpath.getFile() == tpath.getFile()) {
      if (_params.debug >= Params::DEBUG_EXTRA) {
        cerr << "Skipping file: " << readPath << endl;
        cerr << "  Previously processed" << endl;
      }
      return 1;
    }
  }
  
  _readPaths.clear();

  if (_params.debug) {
    cerr << "INFO - FixFieldVals::Run" << endl;
    cerr << "  Input path: " << readPath << endl;
  }
  
  GenericRadxFile inFile;
  _setupCorrectionRead(inFile);
  
  // read in file

  int iret = 0;
  if (inFile.readFromPath(readPath, vol)) {
    cerr << "ERROR - FixFieldVals::_readFile" << endl;
    cerr << "  path: " << readPath << endl;
    cerr << inFile.getErrStr() << endl;
    iret = -1;
  }
  
  // save read paths used

  vector<string> rpaths = inFile.getReadPaths();
  for (size_t ii = 0; ii < rpaths.size(); ii++) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "  ==>> used file: " << rpaths[ii] << endl;
    }
    _readPaths.insert(rpaths[ii]);
  }

  if (iret) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// Analyze the volume, comparing it to the truth files
// Returns 0 on success, -1 on failure

int FixFieldVals::_analyzeVol(RadxVol &corrVol)
  
{

  // initialize field diffs

  for (size_t ii = 0; ii < _fieldDiffs.size(); ii++) {
    _fieldDiffs[ii].init();
  }
  
  // get truth file path closest in time to file to be corrected

  RadxTimeList truthList;
  truthList.setDir(_params.truth_input_dir);
  truthList.setModeClosest(corrVol.getStartRadxTime(),
                           _params.truth_file_time_margin_secs);
  if (truthList.compile()) {
    cerr << "WARNING - FixFieldVals::_analyzeVol()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.truth_input_dir << endl;
    cerr << "  Search time: " << corrVol.getStartRadxTime().asString() << endl;
    cerr << "  Search margin secs: " << _params.truth_file_time_margin_secs << endl;
    cerr << truthList.getErrStr() << endl;
    return -1;
  }
    
  const vector<string> &truthPaths = truthList.getPathList();
  if (truthPaths.size() < 1) {
    cerr << "WARNING - FixFieldVals::_analyzeVol()" << endl;
    cerr << "  Cannot find files, dir: " << _params.truth_input_dir << endl;
    cerr << "  Search time: " << corrVol.getStartRadxTime().asString() << endl;
    cerr << "  Search margin secs: " << _params.truth_file_time_margin_secs << endl;
    return -1;
  }
  
  string truthPath = truthPaths[0];

  // read in truth file

  GenericRadxFile truthFile;
  _setupTruthRead(truthFile);
  
  // read in file

  RadxVol truthVol;
  if (truthFile.readFromPath(truthPath, truthVol)) {
    cerr << "ERROR - FixFieldVals::_analyzeVol" << endl;
    cerr << "  Cannot read truth path: " << truthPath << endl;
    cerr << truthFile.getErrStr() << endl;
    return -1;
  }

  // compute field diffs
  
  if (_computeFieldDiffs(corrVol, truthVol)) {
    return -1;
  }

  // write diffs out to spdb
  
  // create XML string

  string xml;
  
  xml += TaXml::writeStartTag("FieldDiffs", 0);

  xml += TaXml::writeTime("VolStartTime", 1, corrVol.getStartTimeSecs());
  xml += TaXml::writeTime("VolEndTime", 1, corrVol.getEndTimeSecs());
  
  for (size_t ifield = 0; ifield < _fieldDiffs.size(); ifield++) {
    FieldDiff &fDiff = _fieldDiffs[ifield];
    string fieldTag = "Field" + fDiff.corrName;
    xml += TaXml::writeStartTag(fieldTag, 1);
    xml += TaXml::writeString("CorrName", 2, fDiff.corrName);
    xml += TaXml::writeString("TruthName", 2, fDiff.truthName);
    xml += TaXml::writeDouble("NPts", 2, fDiff.nPts);
    xml += TaXml::writeDouble("MeanDiff", 2, fDiff.meanDiff);
    xml += TaXml::writeEndTag(fieldTag, 1);
  }
  
  xml += TaXml::writeEndTag("FieldDiffs", 0);

  // if (_statusXml.size() > 0) {
  //   xml += _statusXml;
  // }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Writing XML stats to SPDB:" << endl;
    cerr << xml << endl;
  }

  DsSpdb spdb;
  time_t validTime = corrVol.getStartTimeSecs();
  time_t expireTime = corrVol.getEndTimeSecs();
  si32 dataType = Spdb::hash4CharsToInt32(corrVol.getInstrumentName().c_str());
  spdb.addPutChunk(dataType, validTime, expireTime, xml.size() + 1, xml.c_str());
  if (spdb.put(_params.field_bias_spdb_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - StatsMgr::writeStatsToSpdb" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote stats to spdb, url: " << _params.field_bias_spdb_url << endl;
    cerr << "  Valid time: " << RadxTime::strm(validTime) << endl;
  }

  return 0;
  
}

//////////////////////////////////////////////////////////
// Analyze the fields diffs for the corr and truth volumes
// Returns 0 on success, -1 on failure

int FixFieldVals::_computeFieldDiffs(RadxVol &corrVol, RadxVol &truthVol)
  
{

  // loop through the rays in the correction vol

  int startTruthIndex = 0;
  vector<RadxRay *> &corrRays = corrVol.getRays();
  for (size_t ic = 0; ic < corrRays.size(); ic++) {
    RadxRay *corrRay = corrRays[ic];

    // find the matching ray in the truth vol
    
    vector<RadxRay *> &truthRays = truthVol.getRays();
    for (size_t it = startTruthIndex; it < truthRays.size(); it++) {
      RadxRay *truthRay = truthRays[it];

      // check tolerances

      if (corrRay->getNGates() != truthRay->getNGates()) {
        continue;
      }

      double deltaEl = fabs(corrRay->getElevationDeg() - truthRay->getElevationDeg());
      if (deltaEl > _params.truth_ray_el_margin_deg) {
        continue;
      }
      
      double deltaAz = fabs(corrRay->getAzimuthDeg() - truthRay->getAzimuthDeg());
      if (deltaAz > _params.truth_ray_az_margin_deg) {
        continue;
      }
      
      double deltaTime = fabs(corrRay->getRadxTime() - truthRay->getRadxTime());
      if (deltaTime > _params.truth_ray_time_margin_secs) {
        continue;
      }

      // got matching ray

      startTruthIndex = it;

      // loop through the field pairs
      
      for (size_t ifield = 0; ifield < _fieldDiffs.size(); ifield++) {

        FieldDiff &fDiff = _fieldDiffs[ifield];

        RadxField *corrField = corrRay->getField(fDiff.corrName);
        if (corrField == NULL) {
          continue;
        }
        
        RadxField *truthField = truthRay->getField(fDiff.truthName);
        if (truthField == NULL) {
          continue;
        }

        if (corrField->getNPoints() != truthField->getNPoints()) {
          continue;
        }

        // add to sums of diffs
        
        corrField->convertToFl32();
        Radx::fl32 *corrData = corrField->getDataFl32();
        Radx::fl32 corrMiss = corrField->getMissingFl32();
        
        truthField->convertToFl32();
        Radx::fl32 *truthData = truthField->getDataFl32();
        Radx::fl32 truthMiss = truthField->getMissingFl32();

        for (size_t igate = 0; igate < corrField->getNPoints(); igate++) {
          if (corrData[igate] == corrMiss) {
            continue;
          }
          if (truthData[igate] == truthMiss) {
            continue;
          }
          fDiff.sumCorr += corrData[igate];
          fDiff.sumTruth += truthData[igate];
          fDiff.nPts++;
        }
        
      } // ifield
  
    } // it - truth rays

  } // ic - corr rays
  
  // compute the means

  int iret = 0;
  for (size_t ifield = 0; ifield < _fieldDiffs.size(); ifield++) {
    FieldDiff &fDiff = _fieldDiffs[ifield];
    fDiff.computeMeanDiff();
    if (fDiff.nPts < _params.min_npts_for_valid_diff) {
      iret = -1;
    }
  }

  return iret;
  
}

//////////////////////////////////////////////////
// Finalize the volume based on parameters
// Returns 0 on success, -1 on failure

void FixFieldVals::_finalizeVol(RadxVol &vol)
  
{

  // remove unwanted fields
  
  if (_params.exclude_specified_fields) {
    for (int ii = 0; ii < _params.excluded_fields_n; ii++) {
      if (_params.debug) {
        cerr << "Removing field name: " << _params._excluded_fields[ii] << endl;
      }
      vol.removeField(_params._excluded_fields[ii]);
    }
  }

  if (_params.set_output_fields && !_params.write_other_fields_unchanged) {
    vector<string> uniqueFields = vol.getUniqueFieldNameList();
    for (size_t jj = 0; jj < uniqueFields.size(); jj++) {
      string fname = uniqueFields[jj]; 
      bool keep = false;
      for (int ii = 0; ii < _params.output_fields_n; ii++) {
        if (fname == _params._output_fields[ii].input_field_name) {
          keep = true;
          break;
        }
      } // ii
      if (!keep) {
        vol.removeField(fname);
      }
    } // jj
  }

  // remap geometry as applicable

  if (_params.remap_to_predominant_range_geometry) {
    vol.remapToPredomGeom();
  }
  if (_params.remap_to_finest_range_geometry) {
    vol.remapToFinestGeom();
  }

  // set nyquist
  
  if (_params.set_nyquist_velocity) {
    set<string> fieldSet = vol.getUniqueFieldNameSet();
    int count = 0;
    for (int ii = 0; ii < _params.nyquist_fields_n; ii++) {
      const Params::nyquist_field_t &nField = _params._nyquist_fields[ii];
      string fieldName = nField.field_name;
      if (fieldSet.find(fieldName) != fieldSet.end()) {
        if (count == 0) {
          vol.setNyquistMps(nField.nyquist_mps);
        }
        vol.setFieldFolds(fieldName, true,
                          -1.0 * nField.nyquist_mps, nField.nyquist_mps);
        count++;
      } else {
        cerr << "WARNING: field not found for setting nyquist: " << fieldName << endl;
      }
    } // ii
  }

  // set number of gates constant if requested

  if (_params.set_ngates_constant) {
    if (_params.debug) {
      cerr << "DEBUG - setting nGates constant" << endl;
    }
    vol.setNGatesConstant();
  }

  // censor as needed

  if (_params.apply_censoring) {
    if (_params.debug) {
      cerr << "DEBUG - applying censoring" << endl;
    }
    _censorFields(vol);
  }

  if (_params.censor_test_pulse_ring) {
    if (_params.debug) {
      cerr << "DEBUG - censoring the test pulse ring" << endl;
    }
    bool censorAllFields = !_params.specify_fields_to_be_censored;
    vector<string> fieldNames;
    for (int ii = 0; ii < _params.fields_to_be_censored_n; ii++) {
      fieldNames.push_back(_params._fields_to_be_censored[ii]);
    }
    vol.censorRangeRing(_params.test_pulse_min_range_km,
                        _params.test_pulse_max_range_km,
                        true,
                        _params.test_pulse_field_name,
                        _params.test_pulse_margin_km,
                        censorAllFields, fieldNames);
  } // if (_params.censor_test_pulse_ring) {

  // linear transform on fields as required

  if (_params.apply_linear_transforms) {
    if (_params.debug) {
      cerr << "DEBUG - applying linear transforms" << endl;
    }
    _applyLinearTransform(vol);
  }

  // set field type, names, units etc
  
  _convertFields(vol);

  if (_params.set_output_encoding_for_all_fields) {
    _convertAllFields(vol);
  }

  // set global attributes

  _setGlobalAttr(vol);

}

//////////////////////////////////////////////////
// set up read for correction files

void FixFieldVals::_setupCorrectionRead(RadxFile &file)
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
    if (_params.lower_fixed_angle_limit == _params.upper_fixed_angle_limit) {
      // relax strict angle checking since only a single angle is specified
      // which means the user wants the closest angle
      file.setReadStrictAngleLimits(false);
    }
  }

  if (!_params.apply_strict_angle_limits) {
    file.setReadStrictAngleLimits(false);
  }

  if (_params.processing_stage == Params::ANALYSIS_STAGE) {

    for (int ii = 0; ii < _params.comparison_fields_n; ii++) {
      file.addReadField(_params._comparison_fields[ii].correction_field_name);
    }
    
  } else {
  
    if (!_params.write_other_fields_unchanged) {
      
      if (_params.set_output_fields) {
        for (int ii = 0; ii < _params.output_fields_n; ii++) {
          file.addReadField(_params._output_fields[ii].input_field_name);
        }
      }
      
      if (_params.apply_linear_transforms) {
        for (int ii = 0; ii < _params.transform_fields_n; ii++) {
          file.addReadField(_params._transform_fields[ii].input_field_name);
        }
      }
      
      if (_params.apply_censoring) {
        for (int ii = 0; ii < _params.censoring_fields_n; ii++) {
          file.addReadField(_params._censoring_fields[ii].name);
        }
        if (_params.specify_fields_to_be_censored) {
          for (int ii = 0; ii < _params.fields_to_be_censored_n; ii++) {
            file.addReadField(_params._fields_to_be_censored[ii]);
          }
        }
      }
      
    } // if (!_params.write_other_fields_unchanged)

  } // if (_params.process_stage == Params::ANALYSIS_STAGE)

  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }

}

//////////////////////////////////////////////////
// set up read for truth files

void FixFieldVals::_setupTruthRead(RadxFile &file)
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
    if (_params.lower_fixed_angle_limit == _params.upper_fixed_angle_limit) {
      // relax strict angle checking since only a single angle is specified
      // which means the user wants the closest angle
      file.setReadStrictAngleLimits(false);
    }
  }

  if (!_params.apply_strict_angle_limits) {
    file.setReadStrictAngleLimits(false);
  }

  for (int ii = 0; ii < _params.comparison_fields_n; ii++) {
    file.addReadField(_params._comparison_fields[ii].truth_field_name);
  }

  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }

}

//////////////////////////////////////////////////
// apply linear transform to fields as required

void FixFieldVals::_applyLinearTransform(RadxVol &vol)
{

  for (int ii = 0; ii < _params.transform_fields_n; ii++) {
    const Params::transform_field_t &tfld = _params._transform_fields[ii];
    string iname = tfld.input_field_name;
    double scale = tfld.transform_scale;
    double offset = tfld.transform_offset;
    bool fieldFolds = tfld.field_folds;
    double foldingValue = tfld.folding_value;
    vol.applyLinearTransform(iname, scale, offset,
                             fieldFolds, foldingValue);
  } // ii

}

//////////////////////////////////////////////////
// rename fields as required

void FixFieldVals::_convertFields(RadxVol &vol)
{

  if (!_params.set_output_fields) {
    return;
  }

  for (int ii = 0; ii < _params.output_fields_n; ii++) {

    const Params::output_field_t &ofld = _params._output_fields[ii];
    
    string iname = ofld.input_field_name;
    string oname = ofld.output_field_name;
    string lname = ofld.long_name;
    string sname = ofld.standard_name;
    string ounits = ofld.output_units;
    
    Radx::DataType_t dtype = Radx::ASIS;
    switch(ofld.encoding) {
      case Params::OUTPUT_ENCODING_FLOAT32:
        dtype = Radx::FL32;
        break;
      case Params::OUTPUT_ENCODING_INT32:
        dtype = Radx::SI32;
        break;
      case Params::OUTPUT_ENCODING_INT16:
        dtype = Radx::SI16;
        break;
      case Params::OUTPUT_ENCODING_INT08:
        dtype = Radx::SI08;
        break;
      case Params::OUTPUT_ENCODING_ASIS:
        dtype = Radx::ASIS;
      default: {}
    }

    if (ofld.output_scaling == Params::SCALING_DYNAMIC) {
      vol.convertField(iname, dtype, 
                       oname, ounits, sname, lname);
    } else {
      vol.convertField(iname, dtype, 
                       ofld.output_scale, ofld.output_offset,
                       oname, ounits, sname, lname);
    }
    
  }

}

//////////////////////////////////////////////////
// convert all fields to specified output encoding

void FixFieldVals::_convertAllFields(RadxVol &vol)
{

  switch(_params.output_encoding) {
    case Params::OUTPUT_ENCODING_FLOAT32:
      vol.convertToFl32();
      return;
    case Params::OUTPUT_ENCODING_INT32:
      vol.convertToSi32();
      return;
    case Params::OUTPUT_ENCODING_INT16:
      vol.convertToSi16();
      return;
    case Params::OUTPUT_ENCODING_INT08:
      vol.convertToSi08();
      return;
    case Params::OUTPUT_ENCODING_ASIS:
    default:
      return;
  }

}

//////////////////////////////////////////////////
// set up write

void FixFieldVals::_setupWrite(RadxFile &file)
{

  if (_params.debug) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.output_filename_mode == Params::START_TIME_ONLY) {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_TIME_ONLY);
  } else if (_params.output_filename_mode == Params::END_TIME_ONLY) {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_END_TIME_ONLY);
  } else {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_AND_END_TIMES);
  }

  file.setWriteCompressed(true);
  file.setCompressionLevel(4);
  
  // set output format

  file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  file.setNcFormat(RadxFile::NETCDF4);

  if (_params.output_force_ngates_vary) {
    file.setWriteForceNgatesVary(true);
  }

  if (strlen(_params.output_filename_prefix) > 0) {
    file.setWriteFileNamePrefix(_params.output_filename_prefix);
  }
  if (strlen(_params.output_filename_suffix) > 0) {
    file.setWriteFileNameSuffix(_params.output_filename_suffix);
  }

  file.setWriteInstrNameInFileName(_params.include_instrument_name_in_file_name);
  file.setWriteSiteNameInFileName(_params.include_site_name_in_file_name);
  file.setWriteSubsecsInFileName(_params.include_subsecs_in_file_name);
  file.setWriteScanTypeInFileName(_params.include_scan_type_in_file_name);
  file.setWriteScanNameInFileName(_params.include_scan_name_in_file_name);
  file.setWriteRangeResolutionInFileName(_params.include_range_resolution_in_file_name);
  file.setWriteVolNumInFileName(_params.include_vol_num_in_file_name);
  file.setWriteHyphenInDateTime(_params.use_hyphen_in_file_name_datetime_part);

  if (_params.write_using_proposed_standard_name_attr) {
    file.setWriteProposedStdNameInNcf(true);
  }

}

//////////////////////////////////////////////////
// set selected global attributes

void FixFieldVals::_setGlobalAttr(RadxVol &vol)
{

  vol.setDriver("FixFieldVals(NCAR)");

  if (strlen(_params.version_override) > 0) {
    vol.setVersion(_params.version_override);
  }

  if (strlen(_params.title_override) > 0) {
    vol.setTitle(_params.title_override);
  }

  if (strlen(_params.institution_override) > 0) {
    vol.setInstitution(_params.institution_override);
  }

  if (strlen(_params.references_override) > 0) {
    vol.setReferences(_params.references_override);
  }

  if (strlen(_params.source_override) > 0) {
    vol.setSource(_params.source_override);
  }

  if (strlen(_params.history_override) > 0) {
    vol.setHistory(_params.history_override);
  }

  if (strlen(_params.author_override) > 0) {
    vol.setAuthor(_params.author_override);
  }

  if (strlen(_params.comment_override) > 0) {
    vol.setComment(_params.comment_override);
  }

  RadxTime now(RadxTime::NOW);
  vol.setCreated(now.asString());

  if (_params.add_user_specified_global_attributes) {
    for (int ii = 0; ii < _params.user_defined_global_attributes_n; ii++) {
      Params::attr_t attr = _params._user_defined_global_attributes[ii];
      RadxVol::UserGlobAttr::attr_type_t attrType = 
        RadxVol::UserGlobAttr::ATTR_STRING;
      switch (attr.attrType) {
        case Params::ATTR_STRING:
          attrType = RadxVol::UserGlobAttr::ATTR_STRING;
          break;
        case Params::ATTR_INT:
          attrType = RadxVol::UserGlobAttr::ATTR_INT;
          break;
        case Params::ATTR_DOUBLE:
          attrType = RadxVol::UserGlobAttr::ATTR_DOUBLE;
          break;
        case Params::ATTR_INT_ARRAY:
          attrType = RadxVol::UserGlobAttr::ATTR_INT_ARRAY;
          break;
        case Params::ATTR_DOUBLE_ARRAY:
          attrType = RadxVol::UserGlobAttr::ATTR_DOUBLE_ARRAY;
          break;
      } // switch
      vol.addUserGlobAttr(attr.name, attrType, attr.val);
    } // ii
  } // if (_params.add_user_specified_global_attributes) {

}

//////////////////////////////////////////////////
// write out the volume

int FixFieldVals::_writeVol(RadxVol &vol)
{

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);

  string outputDir = _params.corrected_files_output_dir;

  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {
    
    string outPath = outputDir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
  
    if (outFile.writeToPath(vol, outPath)) {
      cerr << "ERROR - FixFieldVals::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
    
  } else {

    // write to dir
  
    if (outFile.writeToDir(vol, outputDir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - FixFieldVals::_writeVol" << endl;
      cerr << "  Cannot write file to dir: " << outputDir << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }

  }

  return 0;

}

////////////////////////////////////////////////////////////////////
// censor fields in vol

void FixFieldVals::_censorFields(RadxVol &vol)

{

  vector<RadxRay *> &rays = vol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    _censorRay(rays[ii]);
  }

}

////////////////////////////////////////////////////////////////////
// censor fields in a ray

void FixFieldVals::_censorRay(RadxRay *ray)

{

  if (!_params.apply_censoring) {
    return;
  }

  // convert fields to floats

  vector<Radx::DataType_t> fieldTypes;
  vector<RadxField *> fields = ray->getFields();
  for (size_t ii = 0; ii < fields.size(); ii++) {
    RadxField *field = fields[ii];
    Radx::DataType_t dtype = field->getDataType();
    fieldTypes.push_back(dtype);
    if (_checkFieldForCensoring(field)) {
      field->convertToFl32();
    }
  }

  // initialize censoring flags to true to
  // turn censoring ON everywhere
  
  vector<int> censorFlag;
  size_t nGates = ray->getNGates();
  for (size_t igate = 0; igate < nGates; igate++) {
    censorFlag.push_back(1);
  }

  // check OR fields
  // if any of these have VALID data, we turn censoring OFF

  int orFieldCount = 0;

  for (int ifield = 0; ifield < _params.censoring_fields_n; ifield++) {

    const Params::censoring_field_t &cfld = _params._censoring_fields[ifield];
    if (cfld.combination_method != Params::LOGICAL_OR) {
      continue;
    }

    RadxField *field = ray->getField(cfld.name);
    if (field == NULL) {
      // field missing, do not censor
      if (_nWarnCensorPrint % 360 == 0) {
        cerr << "WARNING - censoring field missing: " << cfld.name << endl;
        cerr << "  Censoring will not be applied for this field." << endl;
      }
      _nWarnCensorPrint++;
      for (size_t igate = 0; igate < nGates; igate++) {
        censorFlag[igate] = 0;
      }
      continue;
    }
    
    orFieldCount++;
    
    double minValidVal = cfld.min_valid_value;
    double maxValidVal = cfld.max_valid_value;

    const Radx::fl32 *fdata = (const Radx::fl32 *) field->getData();
    for (size_t igate = 0; igate < nGates; igate++) {
      double val = fdata[igate];
      if (val >= minValidVal && val <= maxValidVal) {
        censorFlag[igate] = 0;
      }
    }
    
  } // ifield

  // if no OR fields were found, turn off ALL censoring at this stage

  if (orFieldCount == 0) {
    for (size_t igate = 0; igate < nGates; igate++) {
      censorFlag[igate] = 0;
    }
  }

  // check AND fields
  // if any of these have INVALID data, we turn censoring ON

  for (int ifield = 0; ifield < _params.censoring_fields_n; ifield++) {
    
    const Params::censoring_field_t &cfld = _params._censoring_fields[ifield];
    if (cfld.combination_method != Params::LOGICAL_AND) {
      continue;
    }

    RadxField *field = ray->getField(cfld.name);
    if (field == NULL) {
      continue;
    }
    
    double minValidVal = cfld.min_valid_value;
    double maxValidVal = cfld.max_valid_value;

    const Radx::fl32 *fdata = (const Radx::fl32 *) field->getData();
    for (size_t igate = 0; igate < nGates; igate++) {
      double val = fdata[igate];
      if (val < minValidVal || val > maxValidVal) {
        censorFlag[igate] = 1;
      }
    }
    
  } // ifield

  // check that uncensored runs meet the minimum length
  // those which do not are censored

  int minValidRun = _params.censoring_min_valid_run;
  if (minValidRun > 1) {
    int runLength = 0;
    bool doCheck = false;
    for (int igate = 0; igate < (int) nGates; igate++) {
      if (censorFlag[igate] == 0) {
        doCheck = false;
        runLength++;
      } else {
        doCheck = true;
      }
      // last gate?
      if (igate == (int) nGates - 1) doCheck = true;
      // check run length
      if (doCheck) {
        if (runLength < minValidRun) {
          // clear the run which is too short
          for (int jgate = igate - runLength; jgate < igate; jgate++) {
            censorFlag[jgate] = 1;
          } // jgate
        }
        runLength = 0;
      } // if (doCheck ...
    } // igate
  }

  // apply censoring by setting censored gates to missing for all fields

  for (size_t ifield = 0; ifield < fields.size(); ifield++) {
    RadxField *field = fields[ifield];
    if (_checkFieldForCensoring(field)) {
      Radx::fl32 *fdata = (Radx::fl32 *) field->getData();
      Radx::fl32 fmiss = field->getMissingFl32();
      for (size_t igate = 0; igate < nGates; igate++) {
        if (censorFlag[igate] == 1) {
          fdata[igate] = fmiss;
        }
      } // igate
    } // if (_checkFieldForCensoring(field))
  } // ifield

  // convert back to original types
  
  for (size_t ii = 0; ii < fields.size(); ii++) {
    RadxField *field = fields[ii];
    if (_checkFieldForCensoring(field)) {
      field->convertToType(fieldTypes[ii]);
    }
  }

}

////////////////////////////////////////////////////////////////////
// check if a field should be censored

bool FixFieldVals::_checkFieldForCensoring(const RadxField *field)

{

  if (!_params.apply_censoring) {
    return false;
  }

  if (!_params.specify_fields_to_be_censored) {
    return true;
  }

  string checkName = field->getName();

  for (int ii = 0; ii < _params.fields_to_be_censored_n; ii++) {
    string specifiedName = _params._fields_to_be_censored[ii];
    if (checkName == specifiedName) {
      return true;
    }
  }
    
  return false;

}


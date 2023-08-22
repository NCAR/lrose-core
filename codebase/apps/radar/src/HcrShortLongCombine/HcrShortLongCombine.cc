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
//////////////////////////////////////////////////////////////////////////
// HcrShortLongCombine.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2014
//
//////////////////////////////////////////////////////////////////////////
//
// Combines multiple dwells from CfRadial files, writes out combined
// dwell files. The goal is to summarize dwells in pointing data - for
// example from vertically-pointing instruments. This can make displaying
// the data in a BSCAN quicker and more efficient.
//
//////////////////////////////////////////////////////////////////////////

#include "HcrShortLongCombine.hh"
#include <Radx/RadxRay.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/pmu.h>
using namespace std;

// Constructor

HcrShortLongCombine::HcrShortLongCombine(int argc, char **argv)
  
{

  OK = TRUE;
  _nWarnCensorPrint = 0;

  // set programe name

  _progName = "HcrShortLongCombine";
  
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

  // set dwell stats method

  _globalMethod = _getDwellStatsMethod(_params.dwell_stats_method);
  
  if (_params.set_stats_method_for_individual_fields) {
    for (int ii = 0; ii < _params.stats_method_fields_n; ii++) {
      const Params::stats_method_field_t &paramsMethod = 
        _params._stats_method_fields[ii];
      string fieldName = paramsMethod.field_name;
      RadxField::StatsMethod_t method =
        _getDwellStatsMethod(paramsMethod.stats_method);
      RadxField::NamedStatsMethod namedMethod(fieldName, method);
      _namedMethods.push_back(namedMethod);
    } // ii
  }

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

}

// destructor

HcrShortLongCombine::~HcrShortLongCombine()

{

  _inputFmq.closeMsgQueue();
  _outputFmq.closeMsgQueue();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int HcrShortLongCombine::Run()
{

  int iret = 0;

  switch (_params.mode) {
    case Params::FMQ:
      iret = _runFmq();
      break;
    case Params::ARCHIVE:
      iret = _runArchive();
      break;
    case Params::FILELIST:
    default:
      iret = _runFilelist();
  } // switch

  // if we are writing out on time boundaries, there
  // may be unwritten data, so write it now

  if (_params.write_output_files_on_time_boundaries) {
    if (_writeSplitVol()) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in filelist mode

int HcrShortLongCombine::_runFilelist()
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

int HcrShortLongCombine::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir_short);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - HcrShortLongCombine::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir_short << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - HcrShortLongCombine::_runFilelist()" << endl;
    cerr << "  No files found, dir: " << _params.input_dir_short << endl;
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
// Process a file
// Returns 0 on success, -1 on failure

int HcrShortLongCombine::_processFile(const string &readPath)
{

  PMU_auto_register("Processing file");

  // check we have not already processed this file
  // in the file aggregation step
  
  RadxPath thisPath(readPath);
  for (size_t ii = 0; ii < _readPaths.size(); ii++) {
    RadxPath rpath(_readPaths[ii]);
    if (thisPath.getFile() == rpath.getFile()) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Skipping file: " << readPath << endl;
        cerr << "  Previously processed in aggregation step" << endl;
      }
      return 0;
    }
  }
  
  if (_params.debug) {
    cerr << "INFO - HcrShortLongCombine::Run" << endl;
    cerr << "  Input path: " << readPath << endl;
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file

  RadxVol vol;
  if (inFile.readFromPath(readPath, vol)) {
    cerr << "ERROR - HcrShortLongCombine::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < _readPaths.size(); ii++) {
      cerr << "  ==>> read in file: " << _readPaths[ii] << endl;
    }
  }

  // remove unwanted fields
  
  if (_params.exclude_specified_fields) {
    for (int ii = 0; ii < _params.excluded_fields_n; ii++) {
      if (_params.debug) {
        cerr << "Removing field name: " << _params._excluded_fields[ii] << endl;
      }
      vol.removeField(_params._excluded_fields[ii]);
    }
  }

  // linear transform on fields as required

  if (_params.apply_linear_transforms) {
    _applyLinearTransform(vol);
  }

  // add field folding attribute if needed

  if (_params.set_field_folds_attribute) {
    _setFieldFoldsAttribute(vol);
  }

  // combine the dwells

  if (_params.center_dwell_on_time) {
    _combineDwellsCentered(vol);
  } else {
    _combineDwells(vol);
  }

  // censor as needed

  if (_params.apply_censoring) {
    if (_params.debug) {
      cerr << "DEBUG - applying censoring" << endl;
    }
    _censorFields(vol);
  }

  // set field type, names, units etc
  
  _convertFields(vol);

  if (_params.set_output_encoding_for_all_fields) {
    _convertAllFields(vol);
  }

  // set global attributes

  _setGlobalAttr(vol);

  // write the file

  if (_writeVol(vol)) {
    cerr << "ERROR - HcrShortLongCombine::_processFile" << endl;
    cerr << "  Cannot write volume to file" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up read

void HcrShortLongCombine::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.aggregate_sweep_files_on_read) {
    file.setReadAggregateSweeps(true);
  } else {
    file.setReadAggregateSweeps(false);
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

void HcrShortLongCombine::_applyLinearTransform(RadxVol &vol)
{

  for (int ii = 0; ii < _params.transform_fields_n; ii++) {
    const Params::transform_field_t &tfld = _params._transform_fields[ii];
    string iname = tfld.input_field_name;
    double scale = tfld.transform_scale;
    double offset = tfld.transform_offset;
    vol.applyLinearTransform(iname, scale, offset);
  } // ii

}

////////////////////////////////////////////////////////
// set the field folds attribute on selected fields

void HcrShortLongCombine::_setFieldFoldsAttribute(RadxVol &vol)
{

  for (int ii = 0; ii < _params.field_folds_n; ii++) {
    
    const Params::field_folds_t &fld = _params._field_folds[ii];

    vol.setFieldFolds(fld.field_name,
                      fld.use_nyquist,
                      fld.fold_limit_lower,
                      fld.fold_limit_upper);

  } // ii
  
}

//////////////////////////////////////////////////
// rename fields as required

void HcrShortLongCombine::_convertFields(RadxVol &vol)
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

void HcrShortLongCombine::_convertAllFields(RadxVol &vol)
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

void HcrShortLongCombine::_setupWrite(RadxFile &file)
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

  if (_params.output_compressed) {
    file.setWriteCompressed(true);
    file.setCompressionLevel(_params.compression_level);
  } else {
    file.setWriteCompressed(false);
  }

  if (_params.output_native_byte_order) {
    file.setWriteNativeByteOrder(true);
  } else {
    file.setWriteNativeByteOrder(false);
  }

  // set output format

  switch (_params.output_format) {
    case Params::OUTPUT_FORMAT_UF:
      file.setFileFormat(RadxFile::FILE_FORMAT_UF);
      break;
    case Params::OUTPUT_FORMAT_DORADE:
      file.setFileFormat(RadxFile::FILE_FORMAT_DORADE);
      break;
    case Params::OUTPUT_FORMAT_FORAY:
      file.setFileFormat(RadxFile::FILE_FORMAT_FORAY_NC);
      break;
    case Params::OUTPUT_FORMAT_NEXRAD:
      file.setFileFormat(RadxFile::FILE_FORMAT_NEXRAD_AR2);
      break;
    case Params::OUTPUT_FORMAT_MDV_RADIAL:
      file.setFileFormat(RadxFile::FILE_FORMAT_MDV_RADIAL);
      break;
    default:
    case Params::OUTPUT_FORMAT_CFRADIAL:
      file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  }

  // set netcdf format - used for CfRadial

  switch (_params.netcdf_style) {
    case Params::NETCDF4_CLASSIC:
      file.setNcFormat(RadxFile::NETCDF4_CLASSIC);
      break;
    case Params::NC64BIT:
      file.setNcFormat(RadxFile::NETCDF_OFFSET_64BIT);
      break;
    case Params::NETCDF4:
      file.setNcFormat(RadxFile::NETCDF4);
      break;
    default:
      file.setNcFormat(RadxFile::NETCDF_CLASSIC);
  }

  if (_params.write_individual_sweeps) {
    file.setWriteIndividualSweeps(true);
  }

  if (_params.output_force_ngates_vary) {
    file.setWriteForceNgatesVary(true);
  }

  if (strlen(_params.output_filename_prefix) > 0) {
    file.setWriteFileNamePrefix(_params.output_filename_prefix);
  }

  file.setWriteInstrNameInFileName(_params.include_instrument_name_in_file_name);
  file.setWriteSiteNameInFileName(_params.include_site_name_in_file_name);
  file.setWriteSubsecsInFileName(_params.include_subsecs_in_file_name);
  file.setWriteScanTypeInFileName(_params.include_scan_type_in_file_name);
  file.setWriteVolNumInFileName(_params.include_vol_num_in_file_name);
  file.setWriteHyphenInDateTime(_params.use_hyphen_in_file_name_datetime_part);

}

//////////////////////////////////////////////////
// set selected global attributes

void HcrShortLongCombine::_setGlobalAttr(RadxVol &vol)
{

  vol.setDriver("HcrShortLongCombine(NCAR)");

  if (strlen(_params.radar_name_override) > 0) {
    vol.setInstrumentName(_params.radar_name_override);
  }

  if (strlen(_params.site_name_override) > 0) {
    vol.setSiteName(_params.site_name_override);
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
  } else {
    string history(vol.getHistory());
    history += "\n";
    history += "Dwells combined using HcrShortLongCombine\n";
    vol.setHistory(history);
  }
  
  if (strlen(_params.author_override) > 0) {
    vol.setAuthor(_params.author_override);
  }

  if (strlen(_params.comment_override) > 0) {
    vol.setComment(_params.comment_override);
  }

  RadxTime now(RadxTime::NOW);
  vol.setCreated(now.asString());

}

//////////////////////////////////////////////////
// write out the volume

int HcrShortLongCombine::_writeVol(RadxVol &vol)
{

  // are we writing files on time boundaries

  if (_params.write_output_files_on_time_boundaries) {
    return _writeVolOnTimeBoundary(vol);
  }

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {

    string outPath = _params.output_dir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
  
    if (outFile.writeToPath(vol, outPath)) {
      cerr << "ERROR - HcrShortLongCombine::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
      
  } else {

    // write to dir
  
    if (outFile.writeToDir(vol, _params.output_dir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - HcrShortLongCombine::_writeVol" << endl;
      cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }

  }

  string outputPath = outFile.getPathInUse();

  // write latest data info file if requested 
  
  if (_params.write_latest_data_info) {
    DsLdataInfo ldata(_params.output_dir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(_params.output_dir, outputPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(vol.getEndTimeSecs())) {
      cerr << "WARNING - HcrShortLongCombine::_writeVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// write out the data splitting on time

int HcrShortLongCombine::_writeVolOnTimeBoundary(RadxVol &vol)
{
  
  // check for time gap

  RadxTime newVolStart = vol.getStartRadxTime();
  RadxTime splitVolEnd = _splitVol.getEndRadxTime();
  double gapSecs = newVolStart - splitVolEnd;
  if (gapSecs > _params.output_file_time_interval_secs * 2) {
    if (_params.debug) {
      cerr << "==>> Found time gap between volumes" << endl;
      cerr << "  splitVolEnd: " << splitVolEnd.asString(3) << endl;
      cerr << "  newVolStart: " << newVolStart.asString(3) << endl;
    }
    _writeSplitVol();
    _setNextEndOfVolTime(newVolStart);
    // clear out rays from previous file
    _dwellVol.clearRays();
    // clear any rays before the new vol start
    // these could have been introduced during the merge
  }

  // add rays to the output vol

  _splitVol.copyMeta(vol);
  vector<RadxRay *> &volRays = vol.getRays();
  for (size_t ii = 0; ii < volRays.size(); ii++) {
    RadxRay *ray = volRays[ii];
    if (ray->getRadxTime() > _nextEndOfVolTime) {
      if (_writeSplitVol()) {
        return -1;
      }
    }
    RadxRay *splitRay = new RadxRay(*ray);
    _splitVol.addRay(splitRay);
  } // ii

  return 0;

}

//////////////////////////////////////////////////
// write out the split volume

int HcrShortLongCombine::_writeSplitVol()
{

  // sanity check

  if (_splitVol.getNRays() < 1) {
    return 0;
  }

  // load the sweep information from the rays
  
  _splitVol.loadSweepInfoFromRays();

  // load the volume information from the rays
  
  _splitVol.loadVolumeInfoFromRays();
  
  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  // write out

  if (outFile.writeToDir(_splitVol, _params.output_dir,
                         _params.append_day_dir_to_output_dir,
                         _params.append_year_dir_to_output_dir)) {
    cerr << "ERROR - HcrShortLongCombine::_writeSplitVol" << endl;
    cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
    cerr << outFile.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote file: " << outFile.getPathInUse() << endl;
    cerr << "  StartTime: " << _splitVol.getStartRadxTime().asString(3) << endl;
    cerr << "  EndTime  : " << _splitVol.getEndRadxTime().asString(3) << endl;
  }

  // write latest data info file if requested 
  
  if (_params.write_latest_data_info) {
    string outputPath = outFile.getPathInUse();
    DsLdataInfo ldata(_params.output_dir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(_params.output_dir, outputPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(_splitVol.getEndTimeSecs())) {
      cerr << "WARNING - HcrShortLongCombine::_writeSplitVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  // update next end of vol time

  RadxTime nextVolStart(_splitVol.getEndTimeSecs() + 1);
  _setNextEndOfVolTime(nextVolStart);

  // clear

  _splitVol.clearRays();

  return 0;

}

//////////////////////////////////////////////////
// Compute next end of vol time

void HcrShortLongCombine::_setNextEndOfVolTime(RadxTime &refTime)
{
  _nextEndOfVolTime.set
    (((refTime.utime() / _params.output_file_time_interval_secs) + 1) *
     _params.output_file_time_interval_secs);
  if (_params.debug) {
    cerr << "==>> Next end of vol time: " << _nextEndOfVolTime.asString(3) << endl;
  }
}

/////////////////////////////////////////////////////////////////////////
// Combine the dwells in this volume

int HcrShortLongCombine::_combineDwells(RadxVol &vol)

{
  
  if (_params.debug) {
    cerr << "INFO - combineDwells: nrays left from previous file: "
         << _dwellVol.getNRays() << endl;
  }

  // create a volume for stats
  
  vector<RadxRay *> combRays;
  
  const vector<RadxRay *> &fileRays = vol.getRays();
  for (size_t iray = 0; iray < fileRays.size(); iray++) {
    
    // add rays to stats vol
    
    RadxRay *ray = new RadxRay(*fileRays[iray]);
    if (_dwellVol.getNRays() == 0) {
      _dwellStartTime = ray->getRadxTime();
    }
    _dwellVol.addRay(ray);
    int nRaysDwell = _dwellVol.getNRays();
    _dwellEndTime = ray->getRadxTime();
    double dwellSecs = (_dwellEndTime - _dwellStartTime);
    if (nRaysDwell > 1) {
      dwellSecs *= ((double) nRaysDwell / (nRaysDwell - 1.0));
    }
    
    // dwell time exceeded, so compute dwell ray and add to volume
    
    if (dwellSecs >= _params.dwell_time_secs) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "INFO: _combineDwells, using nrays: " << nRaysDwell << endl;
      }
      RadxRay *dwellRay =
        _dwellVol.computeFieldStats(_globalMethod,
                                    _namedMethods,
                                    _params.dwell_stats_max_fraction_missing);
      if (dwellRay->getRadxTime() >= vol.getStartRadxTime() - 60) {
        combRays.push_back(dwellRay);
      } else {
        RadxRay::deleteIfUnused(dwellRay);
      }
      // clear out stats vol
      _dwellVol.clearRays();
    }
      
  } // iray

  // move combination rays into volume

  vol.clearRays();
  for (size_t ii = 0; ii < combRays.size(); ii++) {
    vol.addRay(combRays[ii]);
  }
  
  // compute volume metadata

  vol.loadSweepInfoFromRays();

  return 0;

}

/////////////////////////////////////////////////////////////////////////
// Combine the dwells centered on time

int HcrShortLongCombine::_combineDwellsCentered(RadxVol &vol)

{
  
  if (_params.debug) {
    cerr << "INFO - combineDwellsCentered: nrays left from previous file: "
         << _dwellVol.getNRays() << endl;
  }

  // create a volume for combined dwells
  
  vector<RadxRay *> combRays;
  
  const vector<RadxRay *> &fileRays = vol.getRays();
  for (size_t iray = 0; iray < fileRays.size(); iray++) {

    RadxRay *ray = new RadxRay(*fileRays[iray]);
    _latestRayTime = ray->getRadxTime();
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "==>> got new ray, latestRayTime: " << _latestRayTime.asString(3) << endl;
    }

    // at the start of reading a volume, we always need 
    // at least 1 ray in the dwell

    if (_dwellVol.getNRays() == 0) {
      _dwellVol.addRay(ray);
      continue;
    }
    
    // set dwell time limits if we have just 1 ray in the dwell so far
    
    if (_dwellVol.getNRays() == 1) {

      _dwellStartTime = _dwellVol.getRays()[0]->getRadxTime();

      RadxTime volStartTime(vol.getStartTimeSecs());
      double dsecs = _latestRayTime - volStartTime;
      double roundedSecs =
        ((int) (dsecs / _params.dwell_time_secs) + 1.0) * _params.dwell_time_secs;
      _dwellMidTime = volStartTime + roundedSecs;
      _dwellEndTime = _dwellMidTime + _params.dwell_time_secs / 2.0;

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> starting new dwell <<==" << endl;
        cerr << "  _dwellStartTime: " << _dwellStartTime.asString(3) << endl;
        cerr << "  _dwellMidTime: " << _dwellMidTime.asString(3) << endl;
        cerr << "  _dwellEndTime: " << _dwellEndTime.asString(3) << endl;
      }
        
    }
    
    // dwell time exceeded, so compute dwell ray stats
    // and add results to volume
    
    if (_latestRayTime > _dwellEndTime) {

      // beyond end of dwell, process this one
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "INFO: _combineDwellsCentered, using nrays: "
             << _dwellVol.getNRays() << endl;
        const vector<RadxRay *> &dwellRays = _dwellVol.getRays();
        for (size_t jray = 0; jray < dwellRays.size(); jray++) {
          const RadxRay *dray = dwellRays[jray];
          cerr << "INFO: using ray at time: "
               << dray->getRadxTime().asString(3) << endl;
        }
      } // debug

      // compute ray for dwell
      
      RadxRay *dwellRay =
        _dwellVol.computeFieldStats(_globalMethod, _namedMethods,
                                    _params.dwell_stats_max_fraction_missing);

      // add it to the combination

      if (dwellRay) {
        if (dwellRay->getRadxTime() >= vol.getStartRadxTime() - 60) {
          dwellRay->setTime(_dwellMidTime);
          combRays.push_back(dwellRay);
        } else {
          RadxRay::deleteIfUnused(dwellRay);
        }
      }

      // clear out stats vol

      _dwellVol.clearRays();

    } // if (_latestRayTime > _dwellEndTime) {
      
    // add the latest ray to the next dwell vol
    
    _dwellVol.addRay(ray);
    
  } // iray

  // move combination rays into volume
  
  vol.clearRays();
  for (size_t ii = 0; ii < combRays.size(); ii++) {
    vol.addRay(combRays[ii]);
  }
  
  // compute volume metadata

  vol.loadSweepInfoFromRays();

  return 0;

}

////////////////////////////////////////////////////////
// set dwell stats method from params

RadxField::StatsMethod_t
  HcrShortLongCombine::_getDwellStatsMethod(Params::dwell_stats_method_t method)
  
{

  switch (method) {

    case Params::DWELL_STATS_MEAN:
      return RadxField::STATS_METHOD_MEAN;
      break;
    case Params::DWELL_STATS_MEDIAN:
      return RadxField::STATS_METHOD_MEDIAN;
      break;
    case Params::DWELL_STATS_DISCRETE_MODE:
      return RadxField::STATS_METHOD_DISCRETE_MODE;
      break;
    case Params::DWELL_STATS_MAXIMUM:
      return RadxField::STATS_METHOD_MAXIMUM;
      break;
    case Params::DWELL_STATS_MINIMUM:
      return RadxField::STATS_METHOD_MINIMUM;
      break;
    case Params::DWELL_STATS_MIDDLE:
    default:
      return RadxField::STATS_METHOD_MIDDLE;

  }

}


//////////////////////////////////////////////////
// Run in FMQ mode

int HcrShortLongCombine::_runFmq()
{

  // Instantiate and initialize the input DsRadar queue and message
  
  if (_params.seek_to_end_of_input_fmq) {
    if (_inputFmq.init(_params.input_fmq_url, _progName.c_str(),
                       _params.debug >= Params::DEBUG_VERBOSE,
                       DsFmq::BLOCKING_READ_WRITE, DsFmq::END )) {
      fprintf(stderr, "ERROR - %s:Dsr2Radx::_run\n", _progName.c_str());
      fprintf(stderr, "Could not initialize radar queue '%s'\n",
	      _params.input_fmq_url);
      return -1;
    }
  } else {
    if (_inputFmq.init(_params.input_fmq_url, _progName.c_str(),
                       _params.debug >= Params::DEBUG_VERBOSE,
                       DsFmq::BLOCKING_READ_WRITE, DsFmq::START )) {
      fprintf(stderr, "ERROR - %s:Dsr2Radx::_run\n", _progName.c_str());
      fprintf(stderr, "Could not initialize radar queue '%s'\n",
	      _params.input_fmq_url);
      return -1;
    }
  }

  // create the output FMQ
  
  if (_outputFmq.init(_params.output_fmq_url,
                      _progName.c_str(),
                      _params.debug >= Params::DEBUG_VERBOSE,
                      DsFmq::READ_WRITE, DsFmq::END,
                      _params.output_fmq_compress,
                      _params.output_fmq_n_slots,
                      _params.output_fmq_buf_size)) {
    cerr << "WARNING - trying to create new fmq" << endl;
    if (_outputFmq.init(_params.output_fmq_url,
                        _progName.c_str(),
                        _params.debug >= Params::DEBUG_VERBOSE,
                        DsFmq::CREATE, DsFmq::START,
                        _params.output_fmq_compress,
                        _params.output_fmq_n_slots,
                        _params.output_fmq_buf_size)) {
      cerr << "ERROR - Radx2Dsr" << endl;
      cerr << "  Cannot open fmq, URL: " << _params.output_fmq_url << endl;
      return -1;
    }
  }
  
  if (_params.output_fmq_compress) {
    _outputFmq.setCompressionMethod(TA_COMPRESSION_GZIP);
  }
  
  if (_params.output_fmq_write_blocking) {
    _outputFmq.setBlockingWrite();
  }
  if (_params.output_fmq_data_mapper_report_interval > 0) {
    _outputFmq.setRegisterWithDmap
      (true, _params.output_fmq_data_mapper_report_interval);
  }

  // read messages from the queue and process them
  
  _nRaysRead = 0;
  _nRaysWritten = 0;
  _needWriteParams = false;
  RadxTime prevDwellRayTime;
  int iret = 0;
  while (true) {

    PMU_auto_register("Reading FMQ");

    bool gotMsg = false;
    if (_readFmqMsg(gotMsg) || !gotMsg) {
      umsleep(100);
      continue;
    }

    // pass message through if not related to beam
    
    if (!(_inputContents & DsRadarMsg::RADAR_BEAM) &&
        !(_inputContents & DsRadarMsg::RADAR_PARAMS) &&
        !(_inputContents & DsRadarMsg::PLATFORM_GEOREF)) {
      if(_outputFmq.putDsMsg(_inputMsg, _inputContents)) {
        cerr << "ERROR - HcrShortLongCombine::_runFmq()" << endl;
        cerr << "  Cannot copy message to output queue" << endl;
        cerr << "  URL: " << _params.output_fmq_url << endl;
        return -1;
      }
    }

    // combine rays if combined time exceeds specified dwell

    const vector<RadxRay *> &raysDwell = _dwellVol.getRays();
    size_t nRaysDwell = raysDwell.size();
    if (nRaysDwell > 1) {
      
      _dwellStartTime = raysDwell[0]->getRadxTime();
      _dwellEndTime = raysDwell[nRaysDwell-1]->getRadxTime();
      double dwellSecs = (_dwellEndTime - _dwellStartTime);
      dwellSecs *= ((double) nRaysDwell / (nRaysDwell - 1.0));
      
      if (dwellSecs >= _params.dwell_time_secs) {

        // dwell time exceeded, so compute dwell ray and add to volume

        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "INFO: _runFmq, using nrays: " << nRaysDwell << endl;
        }
        RadxRay *dwellRay = _dwellVol.computeFieldStats(_globalMethod,
                                                        _namedMethods);

        RadxTime dwellRayTime(dwellRay->getRadxTime());
        double deltaSecs = dwellRayTime - prevDwellRayTime;

        if (_params.debug >= Params::DEBUG_VERBOSE) {
          if (deltaSecs < _params.dwell_time_secs * 0.8 ||
              deltaSecs > _params.dwell_time_secs * 1.2) {
            cerr << "===>> bad dwell time, nRaysDwell, dsecs: "
                 << dwellRay->getRadxTime().asString(3) << ", "
                 << nRaysDwell << ", "
                 << deltaSecs << endl;
          }
        }

        prevDwellRayTime = dwellRayTime;
        
        // write params if needed
        if (_needWriteParams) {
          if (_writeParams(dwellRay)) {
            return -1; 
          }
          _needWriteParams = false;
        }

        // write out ray

        _writeRay(dwellRay);

        // clean up

        RadxRay::deleteIfUnused(dwellRay);
        _dwellVol.clearRays();
        _georefs.clear();

      }

    } // if (nRaysDwell > 1)

  } // while (true)
  
  return iret;

}

////////////////////////////////////////////////////////////////////
// _readFmqMsg()
//
// Read a message from the queue, setting the flags about ray_data
// and _endOfVolume appropriately.
//
// Sets gotMsg to true if message was read

int HcrShortLongCombine::_readFmqMsg(bool &gotMsg) 
  
{
  
  PMU_auto_register("Reading radar queue");
  
  _inputContents = 0;
  if (_inputFmq.getDsMsg(_inputMsg, &_inputContents, &gotMsg)) {
    return -1;
  }
  if (!gotMsg) {
    return -1;
  }
  
  // set radar parameters if avaliable
  
  if (_inputContents & DsRadarMsg::RADAR_PARAMS) {
    _loadRadarParams();
    _needWriteParams = true;
  }

  // If we have radar and field params, and there is good ray data,
  
  RadxRay *ray = NULL;
  if (_inputContents & DsRadarMsg::RADAR_BEAM) {

    if (_inputMsg.allParamsSet()) {
      
      _nRaysRead++;
      
      // crete ray from ray message
      
      ray = _createInputRay();
      
      // debug print
      
      if (_params.debug) {
        if ((_nRaysRead > 0) && (_nRaysRead % 360 == 0)) {
          cerr << "==>>    read nRays, latest time, el, az: "
               << _nRaysRead << ", "
               << utimstr(ray->getTimeSecs()) << ", "
               << ray->getElevationDeg() << ", "
               << ray->getAzimuthDeg() << endl;
        }
      }
      
      // add the ray to the volume as appropriate
      
      _dwellVol.addRay(ray);
      
    }
    
  } // if (_inputContents ...
  
  return 0;

}

////////////////////////////////////////////////////////////////
// load radar params

void HcrShortLongCombine::_loadRadarParams()

{
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "=========>> got RADAR_PARAMS" << endl;
  }

  _rparams = _inputMsg.getRadarParams();
  
  _dwellVol.setInstrumentName(_rparams.radarName);
  _dwellVol.setScanName(_rparams.scanTypeName);
  
  switch (_rparams.radarType) {
    case DS_RADAR_AIRBORNE_FORE_TYPE: {
      _dwellVol.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
      break;
    }
    case DS_RADAR_AIRBORNE_AFT_TYPE: {
      _dwellVol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_FORE);
      break;
    }
    case DS_RADAR_AIRBORNE_TAIL_TYPE: {
      _dwellVol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_TAIL);
      break;
    }
    case DS_RADAR_AIRBORNE_LOWER_TYPE: {
      _dwellVol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_BELLY);
      break;
    }
    case DS_RADAR_AIRBORNE_UPPER_TYPE: {
      _dwellVol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_ROOF);
      break;
    }
    case DS_RADAR_SHIPBORNE_TYPE: {
      _dwellVol.setPlatformType(Radx::PLATFORM_TYPE_SHIP);
      break;
    }
    case DS_RADAR_VEHICLE_TYPE: {
      _dwellVol.setPlatformType(Radx::PLATFORM_TYPE_VEHICLE);
      break;
    }
    case DS_RADAR_GROUND_TYPE:
    default:{
      _dwellVol.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
    }
  }
  
  _dwellVol.setLocation(_rparams.latitude,
                        _rparams.longitude,
                        _rparams.altitude);

  _dwellVol.addWavelengthCm(_rparams.wavelength);
  _dwellVol.setRadarBeamWidthDegH(_rparams.horizBeamWidth);
  _dwellVol.setRadarBeamWidthDegV(_rparams.vertBeamWidth);

}

////////////////////////////////////////////////////////////////////
// add an input ray from an incoming message

RadxRay *HcrShortLongCombine::_createInputRay()

{

  // input data

  const DsRadarBeam &rbeam = _inputMsg.getRadarBeam();
  const DsRadarParams &rparams = _inputMsg.getRadarParams();
  const vector<DsFieldParams *> &fparamsVec = _inputMsg.getFieldParams();

  // create new ray

  RadxRay *ray = new RadxRay;

  // set ray properties

  ray->setTime(rbeam.dataTime, rbeam.nanoSecs);
  ray->setVolumeNumber(rbeam.volumeNum);
  ray->setSweepNumber(rbeam.tiltNum);

  int scanMode = rparams.scanMode;
  if (rbeam.scanMode > 0) {
    scanMode = rbeam.scanMode;
  } else {
    scanMode = rparams.scanMode;
  }

  ray->setSweepMode(_getRadxSweepMode(scanMode));
  ray->setPolarizationMode(_getRadxPolarizationMode(rparams.polarization));
  ray->setPrtMode(_getRadxPrtMode(rparams.prfMode));
  ray->setFollowMode(_getRadxFollowMode(rparams.followMode));

  double elev = rbeam.elevation;
  if (elev > 180) {
    elev -= 360.0;
  }
  ray->setElevationDeg(elev);

  double az = rbeam.azimuth;
  if (az < 0) {
    az += 360.0;
  }
  ray->setAzimuthDeg(az);

  // range geometry

  int nGates = rparams.numGates;
  ray->setRangeGeom(rparams.startRange, rparams.gateSpacing);

  if (scanMode == DS_RADAR_RHI_MODE ||
      scanMode == DS_RADAR_EL_SURV_MODE) {
    ray->setFixedAngleDeg(rbeam.targetAz);
  } else {
    ray->setFixedAngleDeg(rbeam.targetElev);
  }

  ray->setIsIndexed(rbeam.beamIsIndexed);
  ray->setAngleResDeg(rbeam.angularResolution);
  ray->setAntennaTransition(rbeam.antennaTransition);
  ray->setNSamples(rparams.samplesPerBeam);
  
  ray->setPulseWidthUsec(rparams.pulseWidth);
  double prt = 1.0 / rparams.pulseRepFreq;
  ray->setPrtSec(prt);
  ray->setPrtRatio(1.0);
  ray->setNyquistMps(rparams.unambigVelocity);

  ray->setUnambigRangeKm(Radx::missingMetaDouble);
  ray->setUnambigRange();

  ray->setMeasXmitPowerDbmH(rbeam.measXmitPowerDbmH);
  ray->setMeasXmitPowerDbmV(rbeam.measXmitPowerDbmV);

  // platform georeference
  
  if (_inputContents & DsRadarMsg::PLATFORM_GEOREF) {
    const DsPlatformGeoref &platformGeoref = _inputMsg.getPlatformGeoref();
    const ds_iwrf_platform_georef_t &dsGeoref = platformGeoref.getGeoref();
    _georefs.push_back(platformGeoref);
    RadxGeoref georef;
    georef.setTimeSecs(dsGeoref.packet.time_secs_utc);
    georef.setNanoSecs(dsGeoref.packet.time_nano_secs);
    georef.setLongitude(dsGeoref.longitude);
    georef.setLatitude(dsGeoref.latitude);
    georef.setAltitudeKmMsl(dsGeoref.altitude_msl_km);
    georef.setAltitudeKmAgl(dsGeoref.altitude_agl_km);
    georef.setEwVelocity(dsGeoref.ew_velocity_mps);
    georef.setNsVelocity(dsGeoref.ns_velocity_mps);
    georef.setVertVelocity(dsGeoref.vert_velocity_mps);
    georef.setHeading(dsGeoref.heading_deg);
    georef.setRoll(dsGeoref.roll_deg);
    georef.setPitch(dsGeoref.pitch_deg);
    georef.setDrift(dsGeoref.drift_angle_deg);
    georef.setRotation(dsGeoref.rotation_angle_deg);
    georef.setTilt(dsGeoref.tilt_angle_deg);
    georef.setEwWind(dsGeoref.ew_horiz_wind_mps);
    georef.setNsWind(dsGeoref.ns_horiz_wind_mps);
    georef.setVertWind(dsGeoref.vert_wind_mps);
    georef.setHeadingRate(dsGeoref.heading_rate_dps);
    georef.setPitchRate(dsGeoref.pitch_rate_dps);
    georef.setDriveAngle1(dsGeoref.drive_angle_1_deg);
    georef.setDriveAngle2(dsGeoref.drive_angle_2_deg);
    ray->clearGeoref();
    ray->setGeoref(georef);
  }

  // load up fields

  int byteWidth = rbeam.byteWidth;
  
  for (size_t iparam = 0; iparam < fparamsVec.size(); iparam++) {

    // is this an output field or censoring field?

    const DsFieldParams &fparams = *fparamsVec[iparam];
    string fieldName = fparams.name;
    if (_params.set_output_fields && !_isOutputField(fieldName)) {
      continue;
    }

    // convert to floats
    
    Radx::fl32 *fdata = new Radx::fl32[nGates];

    if (byteWidth == sizeof(fl32)) {

      fl32 *inData = (fl32 *) rbeam.data() + iparam;
      fl32 inMissing = (fl32) fparams.missingDataValue;
      Radx::fl32 *outData = fdata;
      
      for (int igate = 0; igate < nGates;
           igate++, inData += fparamsVec.size(), outData++) {
        fl32 inVal = *inData;
        if (inVal == inMissing) {
          *outData = Radx::missingFl32;
        } else {
          *outData = *inData;
        }
      } // igate

    } else if (byteWidth == sizeof(ui16)) {

      ui16 *inData = (ui16 *) rbeam.data() + iparam;
      ui16 inMissing = (ui16) fparams.missingDataValue;
      double scale = fparams.scale;
      double bias = fparams.bias;
      Radx::fl32 *outData = fdata;
      
      for (int igate = 0; igate < nGates;
           igate++, inData += fparamsVec.size(), outData++) {
        ui16 inVal = *inData;
        if (inVal == inMissing) {
          *outData = Radx::missingFl32;
        } else {
          *outData = *inData * scale + bias;
        }
      } // igate

    } else {

      // byte width 1

      ui08 *inData = (ui08 *) rbeam.data() + iparam;
      ui08 inMissing = (ui08) fparams.missingDataValue;
      double scale = fparams.scale;
      double bias = fparams.bias;
      Radx::fl32 *outData = fdata;
      
      for (int igate = 0; igate < nGates;
           igate++, inData += fparamsVec.size(), outData++) {
        ui08 inVal = *inData;
        if (inVal == inMissing) {
          *outData = Radx::missingFl32;
        } else {
          *outData = *inData * scale + bias;
        }
      } // igate

    } // if (byteWidth == 4)

    RadxField *field = new RadxField(fparams.name, fparams.units);
    field->copyRangeGeom(*ray);
    field->setTypeFl32(Radx::missingFl32);
    field->addDataFl32(nGates, fdata);

    ray->addField(field);

    delete[] fdata;

  } // iparam

  return ray;

}

//////////////////////////////////////////////////
// write radar and field parameters

int HcrShortLongCombine::_writeParams(const RadxRay *ray)

{

  // radar parameters
  
  DsRadarMsg msg;
  DsRadarParams &rparams = msg.getRadarParams();
  rparams = _rparams;
  rparams.numFields = ray->getFields().size();
  
  // field parameters - all fields are fl32
  
  vector<DsFieldParams* > &fieldParams = msg.getFieldParams();
  vector<RadxField *> flds = ray->getFields();
  for (size_t ifield = 0; ifield < flds.size(); ifield++) {
    
    const RadxField &fld = *flds[ifield];
    double dsScale = 1.0, dsBias = 0.0;
    int dsMissing = (int) floor(fld.getMissingFl32() + 0.5);
    
    DsFieldParams *fParams = new DsFieldParams(fld.getName().c_str(),
                                               fld.getUnits().c_str(),
                                               dsScale, dsBias, sizeof(fl32),
                                               dsMissing);
    fieldParams.push_back(fParams);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fParams->print(cerr);
    }

  } // ifield

  // put params
  
  int content = DsRadarMsg::FIELD_PARAMS | DsRadarMsg::RADAR_PARAMS;
  if(_outputFmq.putDsMsg(msg, content)) {
    cerr << "ERROR - HcrShortLongCombine::_writeParams()" << endl;
    cerr << "  Cannot write field params to FMQ" << endl;
    cerr << "  URL: " << _params.output_fmq_url << endl;
    return -1;
  }
        
  return 0;

}

//////////////////////////////////////////////////
// write ray

int HcrShortLongCombine::_writeRay(const RadxRay *ray)
  
{

  // write params if needed

  int nGates = ray->getNGates();
  const vector<RadxField *> &fields = ray->getFields();
  int nFields = ray->getFields().size();
  int nPoints = nGates * nFields;
  
  // meta-data
  
  DsRadarMsg msg;
  DsRadarBeam &beam = msg.getRadarBeam();

  beam.dataTime = ray->getTimeSecs();
  beam.nanoSecs = (int) (ray->getNanoSecs() + 0.5);
  beam.referenceTime = 0;

  beam.byteWidth = sizeof(fl32); // fl32

  beam.volumeNum = ray->getVolumeNumber();
  beam.tiltNum = ray->getSweepNumber();
  Radx::SweepMode_t sweepMode = ray->getSweepMode();
  beam.scanMode = _getDsScanMode(sweepMode);
  beam.antennaTransition = ray->getAntennaTransition();
  
  beam.azimuth = ray->getAzimuthDeg();
  beam.elevation = ray->getElevationDeg();
  
  if (sweepMode == Radx::SWEEP_MODE_RHI ||
      sweepMode == Radx::SWEEP_MODE_MANUAL_RHI) {
    beam.targetAz = ray->getFixedAngleDeg();
  } else {
    beam.targetElev = ray->getFixedAngleDeg();
  }

  beam.beamIsIndexed = ray->getIsIndexed();
  beam.angularResolution = ray->getAngleResDeg();
  beam.nSamples = ray->getNSamples();

  beam.measXmitPowerDbmH = ray->getMeasXmitPowerDbmH();
  beam.measXmitPowerDbmV = ray->getMeasXmitPowerDbmV();

  // 4-byte floats
  
  fl32 *data = new fl32[nPoints];
  for (int ifield = 0; ifield < nFields; ifield++) {
    fl32 *dd = data + ifield;
    const RadxField *fld = fields[ifield];
    const Radx::fl32 *fd = (Radx::fl32 *) fld->getData();
    if (fd == NULL) {
      cerr << "ERROR - Radx2Dsr::_writeBeam" << endl;
      cerr << "  NULL data pointer, field name, elev, az: "
           << fld->getName() << ", "
           << ray->getElevationDeg() << ", "
           << ray->getAzimuthDeg() << endl;
      delete[] data;
      return -1;
    }
    for (int igate = 0; igate < nGates; igate++, dd += nFields, fd++) {
      *dd = *fd;
    }
  }
  beam.loadData(data, nPoints * sizeof(fl32), sizeof(fl32));
  delete[] data;
    
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    beam.print(cerr);
  }
  
  // add georeference if applicable

  int contents = (int) DsRadarMsg::RADAR_BEAM;
  if (_georefs.size() > 0) {
    DsPlatformGeoref &georef = msg.getPlatformGeoref();
    // use mid georef
    georef = _georefs[_georefs.size() / 2];
    contents |= DsRadarMsg::PLATFORM_GEOREF;
  }
    
  // put beam
  
  if(_outputFmq.putDsMsg(msg, contents)) {
    cerr << "ERROR - HcrShortLongCombine::_writeBeam()" << endl;
    cerr << "  Cannot write beam to FMQ" << endl;
    cerr << "  URL: " << _params.output_fmq_url << endl;
    return -1;
  }

  // debug print
  
  _nRaysWritten++;
  if (_params.debug) {
    if (_nRaysWritten % 100 == 0) {
      cerr << "====>> wrote nRays, latest time, el, az: "
           << _nRaysWritten << ", "
           << utimstr(ray->getTimeSecs()) << ", "
           << ray->getElevationDeg() << ", "
           << ray->getAzimuthDeg() << endl;
    }
  }
  
  return 0;

}

////////////////////////////
// is this an output field?

bool HcrShortLongCombine::_isOutputField(const string &name)

{

  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    string inputFieldName = _params._output_fields[ifield].input_field_name;
    if (name == inputFieldName) {
      return true;
    }
  }
  
  return false;

}

/////////////////////////////////////////////
// convert from DsrRadar enums to Radx enums

Radx::SweepMode_t HcrShortLongCombine::_getRadxSweepMode(int dsrScanMode)

{

  switch (dsrScanMode) {
    case DS_RADAR_SECTOR_MODE:
    case DS_RADAR_FOLLOW_VEHICLE_MODE:
      return Radx::SWEEP_MODE_SECTOR;
    case DS_RADAR_COPLANE_MODE:
      return Radx::SWEEP_MODE_COPLANE;
    case DS_RADAR_RHI_MODE:
      return Radx::SWEEP_MODE_RHI;
    case DS_RADAR_VERTICAL_POINTING_MODE:
      return Radx::SWEEP_MODE_VERTICAL_POINTING;
    case DS_RADAR_MANUAL_MODE:
      return Radx::SWEEP_MODE_POINTING;
    case DS_RADAR_SURVEILLANCE_MODE:
      return Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
    case DS_RADAR_EL_SURV_MODE:
      return Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE;
    case DS_RADAR_SUNSCAN_MODE:
      return Radx::SWEEP_MODE_SUNSCAN;
    case DS_RADAR_SUNSCAN_RHI_MODE:
      return Radx::SWEEP_MODE_SUNSCAN_RHI;
    case DS_RADAR_POINTING_MODE:
      return Radx::SWEEP_MODE_POINTING;
    default:
      return Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
  }
}

Radx::PolarizationMode_t HcrShortLongCombine::_getRadxPolarizationMode(int dsrPolMode)

{
  switch (dsrPolMode) {
    case DS_POLARIZATION_HORIZ_TYPE:
      return Radx::POL_MODE_HORIZONTAL;
    case DS_POLARIZATION_VERT_TYPE:
      return Radx::POL_MODE_VERTICAL;
    case DS_POLARIZATION_DUAL_TYPE:
      return Radx::POL_MODE_HV_SIM;
    case DS_POLARIZATION_DUAL_HV_ALT:
      return Radx::POL_MODE_HV_ALT;
    case DS_POLARIZATION_DUAL_HV_SIM:
      return Radx::POL_MODE_HV_SIM;
    case DS_POLARIZATION_RIGHT_CIRC_TYPE:
    case DS_POLARIZATION_LEFT_CIRC_TYPE:
      return Radx::POL_MODE_CIRCULAR;
    case DS_POLARIZATION_ELLIPTICAL_TYPE:
    default:
      return Radx::POL_MODE_HORIZONTAL;
  }
}

Radx::FollowMode_t HcrShortLongCombine::_getRadxFollowMode(int dsrMode)

{
  switch (dsrMode) {
    case DS_RADAR_FOLLOW_MODE_SUN:
      return Radx::FOLLOW_MODE_SUN;
    case DS_RADAR_FOLLOW_MODE_VEHICLE:
      return Radx::FOLLOW_MODE_VEHICLE;
    case DS_RADAR_FOLLOW_MODE_AIRCRAFT:
      return Radx::FOLLOW_MODE_AIRCRAFT;
    case DS_RADAR_FOLLOW_MODE_TARGET:
      return Radx::FOLLOW_MODE_TARGET;
    case DS_RADAR_FOLLOW_MODE_MANUAL:
      return Radx::FOLLOW_MODE_MANUAL;
    default:
      return Radx::FOLLOW_MODE_NONE;
  }
}

Radx::PrtMode_t HcrShortLongCombine::_getRadxPrtMode(int dsrMode)

{
  switch (dsrMode) {
    case DS_RADAR_PRF_MODE_FIXED:
      return Radx::PRT_MODE_FIXED;
    case DS_RADAR_PRF_MODE_STAGGERED_2_3:
    case DS_RADAR_PRF_MODE_STAGGERED_3_4:
    case DS_RADAR_PRF_MODE_STAGGERED_4_5:
      return Radx::PRT_MODE_STAGGERED;
    default:
      return Radx::PRT_MODE_FIXED;
  }
}

//////////////////////////////////////////////////
// get Dsr enums from Radx enums

int HcrShortLongCombine::_getDsScanMode(Radx::SweepMode_t mode)

{
  switch (mode) {
    case Radx::SWEEP_MODE_SECTOR:
      return DS_RADAR_SECTOR_MODE;
    case Radx::SWEEP_MODE_COPLANE:
      return DS_RADAR_COPLANE_MODE;
    case Radx::SWEEP_MODE_RHI:
      return DS_RADAR_RHI_MODE;
    case Radx::SWEEP_MODE_VERTICAL_POINTING:
      return DS_RADAR_VERTICAL_POINTING_MODE;
    case Radx::SWEEP_MODE_IDLE:
      return DS_RADAR_IDLE_MODE;
    case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE:
      return DS_RADAR_SURVEILLANCE_MODE;
    case Radx::SWEEP_MODE_SUNSCAN:
      return DS_RADAR_SUNSCAN_MODE;
    case Radx::SWEEP_MODE_POINTING:
      return DS_RADAR_POINTING_MODE;
    case Radx::SWEEP_MODE_MANUAL_PPI:
      return DS_RADAR_MANUAL_MODE;
    case Radx::SWEEP_MODE_MANUAL_RHI:
      return DS_RADAR_MANUAL_MODE;
    case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
    default:
      return DS_RADAR_SURVEILLANCE_MODE;
  }
}

////////////////////////////////////////////////////////////////////
// censor fields in vol

void HcrShortLongCombine::_censorFields(RadxVol &vol)

{

  vector<RadxRay *> &rays = vol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    _censorRay(rays[ii]);
  }
  

}

////////////////////////////////////////////////////////////////////
// censor fields in a ray

void HcrShortLongCombine::_censorRay(RadxRay *ray)

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
    field->convertToFl32();
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
  // except those specified to not be censored

  if (_params.specify_non_censored_fields) {

    // only censor fields that are not excluded

    for (size_t ifield = 0; ifield < fields.size(); ifield++) {
      RadxField *field = fields[ifield];
      bool applyCensoring = true;
      for (int ii = 0; ii < _params.non_censored_fields_n; ii++) {
        string fieldToAvoid = _params._non_censored_fields[ii];
        if (field->getName() == fieldToAvoid) {
          applyCensoring = false;
          break;
        }
      }
      if (applyCensoring) {
        Radx::fl32 *fdata = (Radx::fl32 *) field->getData();
        for (size_t igate = 0; igate < nGates; igate++) {
          if (censorFlag[igate] == 1) {
            fdata[igate] = Radx::missingFl32;
          }
        } // igate
      } else {
        if (_params.debug >= Params::DEBUG_EXTRA) {
          cerr << "Not censoring field: " << field->getName() << endl;
        }
      }
    } // ifield

  } else {

    // censor all fields

    for (size_t ifield = 0; ifield < fields.size(); ifield++) {
      RadxField *field = fields[ifield];
      Radx::fl32 *fdata = (Radx::fl32 *) field->getData();
      for (size_t igate = 0; igate < nGates; igate++) {
        if (censorFlag[igate] == 1) {
          fdata[igate] = Radx::missingFl32;
        }
      } // igate
    } // ifield

  }
    
  // convert back to original types
  
  for (size_t ii = 0; ii < fields.size(); ii++) {
    RadxField *field = fields[ii];
    field->convertToType(fieldTypes[ii]);
  }

}


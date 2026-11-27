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
/**
 * @file App.cc
 */

//------------------------------------------------------------------
#include "App.hh"
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/NcfRadxFile.hh>
#include <didss/DataFileNames.hh>
#include <toolsa/pmu.h>
#include <toolsa/LogMsgStreamInit.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
App::App(void) :
        _appName("Unknown"),
        _start(0),
        _end(0),
        _pathIndex(-1)
{
}

//------------------------------------------------------------------
App::~App(void)
{
}

//------------------------------------------------------------------
void App::setValues(const AppArgs &a, const std::string &appName, 
                        const std::string &parmPath,
                        const AppParams &p)
{
  _start = a.startTime;
  _end = a.endTime;
  _fileList = a.inputFileList;
  _appName = appName;
  _params = AppConfig(p);
}

//------------------------------------------------------------------
bool App::init(void cleanup(int), void outOfStore(void),
                   const vector<string> &inputFields)
{
  PORTsignal(SIGQUIT, cleanup);
  PORTsignal(SIGTERM, cleanup);
  PORTsignal(SIGINT, cleanup);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  // set up debugging state for logging  using params
  LogMsgStreamInit::init(_params.debug_mode == AppParams::DEBUG ||
			 _params.debug_mode == AppParams::DEBUG_VERBOSE,
			 _params.debug_mode == AppParams::DEBUG_VERBOSE,
			 true, true);
  if (_params.debug_triggering)
  {
    LogMsgStreamInit::setThreading(true);
  }

  // build up file list
  if (_params._primaryGroup.isClimo)
  {
    LOG(ERROR) << "The primary path cannot be climo data";
    return false;
  }

  if (_params.mode == AppParams::ARCHIVE)
  {
    RadxTimeList tlist;
    tlist.setDir(_params._primaryGroup.dir);
    tlist.setModeInterval(_start, _end);
    if (tlist.compile())
    {
      LOG(ERROR) << "Cannot compile time list, dir: " 
		 << _params._primaryGroup.dir;
      LOG(ERROR) << "   Start time: " << RadxTime::strm(_start);
      LOG(ERROR) << "   End time: " << RadxTime::strm(_end);
      LOG(ERROR) << tlist.getErrStr();
      return false;
    }
    _paths = tlist.getPathList();
    _pathIndex = 0;
    if (_paths.size() < 1)
    {
      LOG(ERROR) << "No files found, dir: " << _params._primaryGroup.dir;
      return false;
    }
  }
  else if (_params.mode == AppParams::FILELIST)
  {
    // file list is in the args
    _paths = _fileList;
    _pathIndex = 0;
  }
  else
  {
    // set the LdataInfo object for REALTIME
    _ldata = LdataInfo(_params._primaryGroup.dir, _params.debug_triggering);
  }

  // notify the procmap that we are alive
  PMU_auto_init(_appName.c_str(), _params.instance, 60);

  // set the out of storage handler 
  set_new_handler(outOfStore);
	      
  
  // check the inputs to make sure on the list somewhere
  return _params.inputsAccountedFor(inputFields);
}

//------------------------------------------------------------------
void App::finish(void)
{
  PMU_auto_unregister();
}

//------------------------------------------------------------------
bool App::trigger(RadxVol &v, time_t &t, bool &last)
{
  LOG(DEBUG) << "------before trigger-----";

  if (_params.mode == AppParams::ARCHIVE ||
      _params.mode == AppParams::FILELIST)
  {
    if (_pathIndex >= (int)_paths.size())
    {
      LOG(DEBUG) << "---No more files to process--";
      return false;
    }
    string inputPath = _paths[_pathIndex++];
    last = _pathIndex == (int)_paths.size();
    return _processFile(inputPath, v, t);
  }
  else
  {
    _ldata.readBlocking(_params.max_realtime_data_age_secs,  1000,
			PMU_auto_register);
    string inputPath = _ldata.getDataPath();
    last = false;
    return _processFile(inputPath, v, t);
  }
}

//---------------------------------------------------------------
bool App::rewind(void)
{
  bool ret = true;
  if (_params.mode == AppParams::ARCHIVE)
  {
    _pathIndex = 0;
  }
  else if (_params.mode == AppParams::FILELIST)
  {
    _pathIndex = 0;
  }
  else
  {
    LOG(ERROR) << "Cannot rewind in real time mode";
    ret = false;
  }
  return ret;
}

//---------------------------------------------------------------
bool App::write(RadxVol &vol, const time_t &t,
                    const std::string &url)
{
  vol.loadVolumeInfoFromRays();
  vol.loadSweepInfoFromRays();
  vol.setPackingFromRays();

  RadxFile *outFile;
  NcfRadxFile *ncfFile = new NcfRadxFile();
  outFile = ncfFile;
  switch (_params.netcdf_style) {
    case AppParams::NETCDF4_CLASSIC:
      ncfFile->setNcFormat(RadxFile::NETCDF4_CLASSIC);
      break;
    case AppParams::NC64BIT:
      ncfFile->setNcFormat(RadxFile::NETCDF_OFFSET_64BIT);
      break;
    case AppParams::NETCDF4:
      ncfFile->setNcFormat(RadxFile::NETCDF4);
      break;
    default:
      ncfFile->setNcFormat(RadxFile::NETCDF_CLASSIC);
  }

  _setupWrite(*outFile);

  if (outFile->writeToDir(vol, url,
                          _params.append_day_dir_to_output_dir,
                          _params.append_year_dir_to_output_dir))
  {
    LOG(ERROR) << "Cannot write file to dir: " <<  url;
    LOG(ERROR) << outFile->getErrStr();
    delete outFile;
    return false;
  }
  string outputPath = outFile->getPathInUse();
  delete outFile;

  string name = nameWithoutPath(outputPath);
  LOG(DEBUG) << "Wrote output file: " << name;

  // in realtime mode, write latest data info file

  if (_params.write_latest_data_info) {
    LdataInfo ldata(url);
    if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
    {
      ldata.setDebug(true);
    }
    RadxPath rpath(outputPath);
    ldata.setRelDataPath(rpath.getFile());
    ldata.setWriter(_appName);
    if (ldata.write(vol.getEndTimeSecs()))
    {
      LOG(WARNING) << "Cannot write latest data info file to dir: " << url;
    }
  }
  return true;
}

//---------------------------------------------------------------
bool App::write(RadxVol &vol, const time_t &t)
{
  return write(vol, t, _params.output_url);
}

//------------------------------------------------------------------
string App::nameWithoutPath(const string &name)
{
  string::size_type i = name.find_last_of("/");
  string ret = name.substr(i+1);
  return ret;
}

//---------------------------------------------------------------
bool App::retrieveRay(const std::string &name, const RadxRay &ray,
                          std::vector<RayxData> &data, RayxData &r)
{
  // try to find the field in the ray first
  if (retrieveRay(name, ray, r, false))
  {
    return true;
  }

  // try to find the field in the data vector
  for (size_t i=0; i<data.size(); ++i)
  {
    if (data[i].getName() == name)
    {
      // return a copy of that ray
      r = data[i];
      return true;
    }
  }
    
  LOG(ERROR) << "Field " << name << "never found";
  return false;
}  

//---------------------------------------------------------------
bool App::retrieveRay(const std::string &name, const RadxRay &ray,
                          RayxData &r, const bool showError)
{
  // try to find the field in the ray
  const vector<RadxField *> &fields = ray.getFields();
  for (size_t ifield = 0; ifield < fields.size(); ifield++)
  {
    if (fields[ifield]->getName() == name)
    {
      Radx::DataType_t t = fields[ifield]->getDataType();
      if (t != Radx::FL32)
      {
	// need this to pull out values
	fields[ifield]->convertToFl32();
      }
      r = RayxData(name, fields[ifield]->getUnits(),
		  fields[ifield]->getNPoints(), fields[ifield]->getMissing(),
		  ray.getAzimuthDeg(), ray.getElevationDeg(),
		  ray.getGateSpacingKm(), ray.getStartRangeKm(),
		  *fields[ifield]);
      return true;
    }
  }
  if (showError)
  {
    LOG(ERROR) << "Field " << name << "never found";
  }
  return false;
}

//---------------------------------------------------------------
void App::modifyRayForOutput(RayxData &r, const std::string &name,
                                 const std::string &units,
                                 const double missing)
{
  r.changeName(name);
  if (units.empty())
  {
    return;
  }
  r.changeUnits(units);
  r.changeMissing(missing);
}

//---------------------------------------------------------------
void App::updateRay(const RayxData &r, RadxRay &ray)
{
  // add in the one RayxData, then clear out everything else
    
  int nGatesPrimary = ray.getNGates();
  Radx::fl32 *data = new Radx::fl32[nGatesPrimary];

  string name = r.getName();
  r.retrieveData(data, nGatesPrimary);
  ray.addField(name, r.getUnits(), r.getNpoints(), r.getMissing(), data, true);
  vector<string> wanted;
  wanted.push_back(name);
  ray.trimToWantedFields(wanted);
  delete [] data;
}

//---------------------------------------------------------------
void App::updateRay(const vector<RayxData> &raydata, RadxRay &ray)
{
  // take all the data and add to the ray.
  // note that here should make sure not deleting a result (check names)
  if (raydata.empty())
  {
    return;
  }
  int nGatesPrimary = ray.getNGates();
  Radx::fl32 *data = new Radx::fl32[nGatesPrimary];

  string name = raydata[0].getName();
  raydata[0].retrieveData(data, nGatesPrimary);
  ray.addField(name, raydata[0].getUnits(), raydata[0].getNpoints(),
	       raydata[0].getMissing(), data, true);
  vector<string> wanted;
  wanted.push_back(name);
  ray.trimToWantedFields(wanted);

  // now add in all the other ones
  for (int i=1; i<(int)raydata.size(); ++i)
  {
    raydata[i].retrieveData(data, nGatesPrimary);
    ray.addField(raydata[i].getName(), raydata[i].getUnits(),
		 raydata[i].getNpoints(),
		 raydata[i].getMissing(), data, true);
  }
  delete [] data;
}

//---------------------------------------------------------------
bool App::_processFile(const string &path,
                           RadxVol &vol, time_t &t)
{
  string name = nameWithoutPath(path);

  LOG(DEBUG) << "Primary file: " <<  name;
  
  RadxFile primaryFile;
  _setupRead(primaryFile);

  if (primaryFile.readFromPath(path, vol))
  {
    LOG(ERROR) << "Cannot read in primary file: " << name;
    LOG(ERROR) << primaryFile.getErrStr();
    return false;
  }

  t = vol.getEndTimeSecs();
  bool dateOnly;
  if (DataFileNames::getDataTime(path, t, dateOnly))
  {
    LOG(ERROR) << "Cannot get time from file path: " << name;
    return false;
  }

  LOG(DEBUG) << "Time for primary file: " << RadxTime::strm(t);

  // Search for secondary files

  for (size_t igroup = 0; igroup < _params._secondaryGroups.size(); igroup++) {
    
    _activeGroup = _params._secondaryGroups[igroup];

    string secondaryPath;
    if (_activeGroup.isClimo)
    {
      secondaryPath = _activeGroup.dir + "/" + _activeGroup.climoFile;
    }
    else
    {
      time_t searchTime = t + _activeGroup.fileTimeOffset;

      RadxTimeList tlist;
      tlist.setDir(_activeGroup.dir);
      tlist.setModeClosest(searchTime, _activeGroup.fileTimeTolerance);

      if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
      {
	tlist.printRequest(cerr);
      }
    
      if (tlist.compile())
      {
	LOG(ERROR) << "Cannot compile secondary file time list";
	LOG(ERROR) << tlist.getErrStr().c_str();
	return false;
      }
      const vector<string> &pathList = tlist.getPathList();
      if (pathList.size() < 1) {
	LOG(WARNING) << "No suitable secondary file found, " 
		     << "Primary file:" << path << ", Secondary path:"
		     << _activeGroup.dir;
	return false;
      }
      else
      {
	secondaryPath = pathList[0];
      }
    }

    string secondaryName = nameWithoutPath(secondaryPath);
    LOG(DEBUG) << "Found secondary file: " << secondaryName;

    RadxFile secondaryFile;
    _setupSecondaryRead(secondaryFile);

    RadxVol secondaryVol;
    if (secondaryFile.readFromPath(secondaryPath, secondaryVol))
    {
      LOG(ERROR) << "Cannot read in secondary file: " << secondaryName;
      LOG(ERROR) << secondaryFile.getErrStr();
      return false;
    }

    // merge the primary and seconday volumes, using the primary
    // volume to hold the merged data
    
    if (!_mergeVol(vol, secondaryVol))
    {
      LOG(ERROR) << "Merge failed, primary:" << name
		 << "secondary:" << secondaryName;
      return false;
    }

  } // igroup

  
  // remove some bad stuff we never will want
  vol.removeTransitionRays();
  vol.trimSurveillanceSweepsTo360Deg();
  

  LOG(DEBUG) << "-------Triggered " << RadxTime::strm(t) << " ----------";
  return true;
}

//------------------------------------------------------------------
void App::_setupRead(RadxFile &file)
{
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.setDebug(true);
  }
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.setVerbose(true);
    file.printReadRequest(cerr);
  }

  if (_params.read_set_fixed_angle_limits)
  {
    file.setReadFixedAngleLimits(_params.read_lower_fixed_angle,
                                 _params.read_upper_fixed_angle);
  }

  for (size_t ii = 0; ii < _params._primaryGroup.names.size(); ii++) {
    file.addReadField(_params._primaryGroup.names[ii]);
  }

  if (_params.ignore_antenna_transitions) {
    file.setReadIgnoreTransitions(true);
  }

  file.setReadAggregateSweeps(false);

  if (_params.ignore_idle_scan_mode_on_read) {
    file.setReadIgnoreIdleMode(true);
  } else {
    file.setReadIgnoreIdleMode(false);
  }

  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
  }

  
  LOG(DEBUG_VERBOSE) << "===== SETTING UP READ FOR PRIMARY FILES =====";
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.printReadRequest(cerr);
  }
  LOG(DEBUG_VERBOSE) << "=============================================";
}



//------------------------------------------------------------------
void App::_setupSecondaryRead(RadxFile &file)
{
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.setDebug(true);
  }
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.setVerbose(true);
  }

  if (_params.read_set_fixed_angle_limits)
  {
    file.setReadFixedAngleLimits(_params.read_lower_fixed_angle,
                                 _params.read_upper_fixed_angle);
  }

  for (size_t ii = 0; ii < _activeGroup.names.size(); ii++)
  {
    file.addReadField(_activeGroup.names[ii]);
  }

  file.setReadAggregateSweeps(false);

  LOG(DEBUG_VERBOSE) << "===== SETTING UP READ FOR SECONDARY FILES =====";
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.printReadRequest(cerr);
  }
  LOG(DEBUG_VERBOSE) << "===============================================";
}

//------------------------------------------------------------------
void App::_setupWrite(RadxFile &file)
{
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.setDebug(true);
  }

  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.setVerbose(true);
  }
  
  if (_params.output_filename_mode == AppParams::START_TIME_ONLY) {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_TIME_ONLY);
  } else if (_params.output_filename_mode == AppParams::END_TIME_ONLY) {
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

  file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);

}

//------------------------------------------------------------------
bool App::_mergeVol(RadxVol &primaryVol,
                        const RadxVol &secondaryVol)
{
  // loop through all rays in primary vol

  const vector<RadxRay *> &pRays = primaryVol.getRays();
  int searchStart = 0;

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
    bool found = false;
    for (size_t jj = searchStart; jj < sRays.size(); jj++)
    {
      RadxRay *sRay = sRays[jj];
      double sTime = (double) sRay->getTimeSecs() + sRay->getNanoSecs() / 1.0e9;
      double sAz = sRay->getAzimuthDeg();   
      double sEl = sRay->getElevationDeg();
      
      int sMilli = sRay->getNanoSecs() / 1.0e6;
      char sMStr[16];
      sprintf(sMStr, "%.3d", sMilli);

      double diffTime;
      if (!_activeGroup.isClimo)
      {
	diffTime = fabs(pTime - sTime);
      }
      else
      {
	diffTime = 0;
      }
      double dAz = pAz - sAz;
      if (dAz < -180)
      {
        dAz += 360.0;
      }
      else if (dAz > 180)
      {
        dAz -= 360.0;
      }
      double diffAz = fabs(dAz);
      double diffEl = fabs(pEl - sEl);

      if (diffTime <= _activeGroup.rayTimeTolerance &&
          diffAz <= _activeGroup.rayAzTolerance &&
          diffEl <= _activeGroup.rayElevTolerance)
      {
        // same ray, merge the rays
        _mergeRay(*pRay, *sRay);
        found = true;
	LOG(DEBUG_VERBOSE) << "Matched ray - time "
			   << RadxTime::strm((time_t)pTime) + "." + pMStr
			   << " az,el=" << pAz << "," << pEl 
			   << " az2,el2=" << sAz << "," << sEl
			   << " dEl=" << diffEl
			   << " dAz=" << diffAz
			   << " dTime=" << diffTime;
	break;
      }
    } // jj

    if (!found)
    {
      LOG(DEBUG_VERBOSE) << "===>>> missed merge, time="
			 << RadxTime::strm((time_t) pTime) + "." + pMStr
			 << " az,el:" << pAz << "," << pEl;
    }
  } // ii

  return true;

}

//------------------------------------------------------------------
void App::_mergeRay(RadxRay &primaryRay,
                        const RadxRay &secondaryRay)
{

  primaryRay.loadFieldNameMap();

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
    
    RadxField sField(*sFields[ifield]);

    // get output field name

    string outputName = sField.getName();

    // ensure geometry is correct

    if (geomDiffers)
    {
      sField.remapRayGeom(remap);
    }
    sField.setNGates(nGatesPrimary);

    Radx::DataType_t dType = sField.getDataType();
    switch (dType) {
      case Radx::UI08:
      case Radx::SI08: {
        primaryRay.addField(outputName,
                            sField.getUnits(),
                            sField.getNPoints(),
                            sField.getMissingSi08(),
                            (const Radx::si08 *) sField.getData(),
                            sField.getScale(),
                            sField.getOffset(),
                            true);
        break;
      }
      case Radx::UI32:
      case Radx::SI32: {
        primaryRay.addField(outputName,
                            sField.getUnits(),
                            sField.getNPoints(),
                            sField.getMissingSi32(),
                            (const Radx::si32 *) sField.getData(),
                            sField.getScale(),
                            sField.getOffset(),
                                     true);
        break;
      }
      case Radx::FL32: {
        primaryRay.addField(outputName,
                            sField.getUnits(),
                            sField.getNPoints(),
                            sField.getMissingFl32(),
                            (const Radx::fl32 *) sField.getData(),
                            true);
        break;
      }
      case Radx::FL64: {
        primaryRay.addField(outputName,
                            sField.getUnits(),
                            sField.getNPoints(),
                            sField.getMissingFl64(),
                            (const Radx::fl64 *) sField.getData(),
                                     true);
        break;
      }
      case Radx::UI16:
      case Radx::SI16:
      default: {
        primaryRay.addField(outputName,
                            sField.getUnits(),
                            sField.getNPoints(),
                            sField.getMissingSi16(),
                            (const Radx::si16 *) sField.getData(),
                            sField.getScale(),
                            sField.getOffset(),
                            true);
      }
    } // switch
  } // ifield

}


/**
 * @file RadxAppVolume.cc
 */

//------------------------------------------------------------------
#include <radar/RadxAppVolume.hh>
#include <didss/DataFileNames.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxRay.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxPath.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>

//------------------------------------------------------------------
static string _nameWithoutPath(const string &name)
{
  string::size_type i = name.find_last_of("/");
  string ret = name.substr(i+1);
  return ret;
}


//----------------------------------------------------------------
static bool _checkArgs(int argc, char **argv, time_t &t0, time_t &t1,
		       bool &archive, bool &error)
{
  archive = false;
  error = false;
  bool t0_set=false, t1_set=false;

  int y, m, d, h, min, sec;

  for (int i=1; i<argc; )
  {
    if (!strcmp(argv[i], "-start"))
    {
      if (i + 1 >= argc)
      {
	LOG(ERROR) << "-start without subsequent time specification";
	error = true;
	return false;
      }
      if (sscanf(argv[i+1], "%4d %2d %2d %2d %2d %2d", &y, &m, &d, 
		 &h, &min, &sec) != 6)
      {
	LOG(ERROR) << "Parsing '" << argv[i+1] << "' as 'yyyy mm dd hh mm ss'";
	error = true;
	return false;
      }
      else
      {
	DateTime dt(y, m, d, h, min, sec);
	t0 = dt.utime();
	t0_set = true;
      }
      i = i+2;
    }      
    else if (!strcmp(argv[i], "-end"))
    {
      if (i + 1 >= argc)
      {
	LOG(ERROR) << "-end without subsequent time specification";
	error = true;
	return false;
      }
      if (sscanf(argv[i+1], "%4d %2d %2d %2d %2d %2d", &y, &m, &d, 
		 &h, &min, &sec) != 6)
      {
	LOG(ERROR) << "Parsing '" << argv[i+1] << "' as 'yyyy mm dd hh mm ss'";
	error = true;
	return false;
      }
      else
      {
	DateTime dt(y, m, d, h, min, sec);
	t1 = dt.utime();
	t1_set = true;
      }
      i = i+2;
    }      
    else if (!strcmp(argv[i], "-interval"))
    {
      if (i+2 >= argc)
      {
	LOG(ERROR) << "-interval without 2 subsequent times";
	error = true;
	return false;
      }
	  
      if (sscanf(argv[i+1], "%4d%2d%2d%2d%2d%2d", &y, &m, &d, 
		 &h, &min, &sec) != 6)
      {
	LOG(ERROR) << "Parsing '" << argv[i+1] << "' as 'yyyymmddhhmmss'";
	error = true;
	return false;
      }
      else
      {
	DateTime dt(y, m, d, h, min, sec);
	t0 = dt.utime();
	t0_set = true;
      }

      if (sscanf(argv[i+2], "%4d%2d%2d%2d%2d%2d", &y, &m, &d, 
		 &h, &min, &sec) != 6)
      {
	LOG(ERROR) << "Parsing '" << argv[i+2] << "' as 'yyyymmddhhmmss'";
	error = true;
	return false;
      }
      else
      {
	DateTime dt(y, m, d, h, min, sec);
	t1 = dt.utime();
	t1_set = true;
      }
      i = i+3;
    }
    else
    {
      i++;
    }
  }
  if ((t0_set && !t1_set) || (t1_set && !t0_set))
  {
    LOG(ERROR) << "Did not set both start and end time";
    error = true;
    return false;
  }
  archive = (t0_set && t1_set);
  return true;
}


//------------------------------------------------------------------
RadxAppVolume::RadxAppVolume(void) :
  _rays(NULL), _sweeps(NULL), _ray(NULL),
  _parms(NULL), _pathIndex(-1), _realtime(false),
  _ok(false)
{
}

//------------------------------------------------------------------
RadxAppVolume::RadxAppVolume(const RadxAppParms *parms,
			     int argc, char **argv):
  _rays(NULL), _sweeps(NULL), _ray(NULL),
  _parms(parms), _pathIndex(-1), _realtime(false),
  _ok(true)
{
  bool error, isArchive;
  time_t t0 = 0, t1 = 0;
  _checkArgs(argc, argv, t0, t1, isArchive, error);
  if (error)
  {
    LOG(ERROR) << "ERROR parsing args";
    exit(1);
  }
  if (isArchive)
  {
    vector<string> dirs;
    if (_parms->ymd_subdirectories)
    {
      DateTime dt0(t0);
      DateTime dt1(t1);
      string ymd0 = dt0.getDateStrPlain();
      string ymd1 = dt1.getDateStrPlain();

      int iymd0, iymd1;
      sscanf(ymd0.c_str(), "%d", &iymd0);
      sscanf(ymd1.c_str(), "%d", &iymd1);
      for (int i=iymd0; i<=iymd1; ++i)
      {
	char buf[1000];
	sprintf(buf, "%s/%08d", _parms->_inputs._primaryGroup.dir.c_str(), i);
	dirs.push_back(buf);
      }
    }
    else
    {
      dirs.push_back(_parms->_inputs._primaryGroup.dir);
    }

    _paths.clear();
    for (size_t i=0; i<dirs.size(); ++i)
    {
      RadxTimeList tlist;
      tlist.setDir(dirs[i]);
      tlist.setModeInterval(t0, t1);
      if (tlist.compile())
      {
	LOG(ERROR) << "Cannot compile time list, dir: " << dirs[i];
	LOG(ERROR) << "   Start time: " << RadxTime::strm(t0);
	LOG(ERROR) << "   End time: " << RadxTime::strm(t1);
	LOG(ERROR) << tlist.getErrStr();
	_ok = false;
	return;
      }
      vector<string> pathsi = tlist.getPathList();
      for (size_t j=0; j<pathsi.size(); ++j)
	_paths.push_back(pathsi[j]);
    }
    _pathIndex = 0;
    if (_paths.size() < 1)
    {
      LOG(ERROR) << "No files found, dir: "
		 << _parms->_inputs._primaryGroup.dir;
      _ok = false;
      return;
    }
  }
  else if (_parms->_isFileList)
  {
    // file list is in the args
    _paths = _parms->_fileList;
    _pathIndex = 0;
  }
  else
  {
    // set the LdataInfo object for REALTIME
    _realtime = true;
    _ldata = LdataInfo(_parms->_inputs._primaryGroup.dir,
		       _parms->debug_triggering);
  }
}

//------------------------------------------------------------------
RadxAppVolume::RadxAppVolume(const RadxAppVolume &v) :
  _vol(v._vol),
  _time(v._time),
  _parms(v._parms),
  _activeGroup(v._activeGroup),
  _pathIndex(v._pathIndex),
  _paths(v._paths),
  _realtime(v._realtime),
  _ldata(v._ldata),
  _ok(v._ok)
{
  _rays = &_vol.getRays();
  _sweeps = &_vol.getSweeps();
  if (!_rays->empty())
  {
    // just to have something around
    _ray = (*_rays)[0];
  }
  else
  {
    _ray = NULL;
  }
}


//------------------------------------------------------------------
RadxAppVolume &RadxAppVolume::operator=(const RadxAppVolume &v)
{
  if (this == &v)
  {
    return *this;
  }
  
  VolumeData::operator=(v);
  _vol = v._vol;
  _time = v._time;
  _parms = v._parms;
  _activeGroup = v._activeGroup;
  _pathIndex = v._pathIndex;
  _paths = v._paths;
  _realtime = v._realtime;
  _ldata = v._ldata;
  _ok = v._ok;
  _rays = &_vol.getRays();
  _sweeps = &_vol.getSweeps();
  if (!_rays->empty())
  {
    // just to have something around
    _ray = (*_rays)[0];
  }
  else
  {
    _ray = NULL;
  }
  return *this;
}

//------------------------------------------------------------------
RadxAppVolume::~RadxAppVolume(void)
{
}

//------------------------------------------------------------------
bool RadxAppVolume::triggerRadxVolume(std::string &path)
{
  LOG(DEBUG) << "------before trigger-----";

  _vol = RadxVol();
  
  if (_realtime)
  {
    _ldata.readBlocking(_parms->max_realtime_data_age_secs,  1000,
			PMU_auto_register);
    path = _ldata.getDataPath();
  }
  else
  {
    if (_pathIndex >= (int)_paths.size())
    {
      LOG(DEBUG) << "---No more files to process--";
      return false;
    }
    path = _paths[_pathIndex++];
  }

  return _processFile(path);
}

//------------------------------------------------------------------
int RadxAppVolume::numProcessingNodes(bool twoD) const
{
  if (twoD)
  {
    return (int)_sweeps->size();
  }
  else
  {
    return (int)_rays->size();
  }
}

//------------------------------------------------------------------
// virtual
bool RadxAppVolume::synchUserDefinedInputs(const std::string &userKey,
					   const std::vector<std::string> &names)
{
  if (!needToSynch(userKey))
  {
    return true;
  }
  for (size_t i=0; i<names.size(); ++i)
  {
    if (!hasData(userKey, names[i], false))
    {
      LOG(ERROR) << "Cannot synch";
      return false;
    }
  }
  return true;
}

//------------------------------------------------------------------
void RadxAppVolume::trim(void)
{
  if (_parms->_outputAllFields)
  {
    return;
  }
  for (size_t i=0; i<_rays->size(); ++i)
  {
    (*_rays)[i]->trimToWantedFields(_parms->_outputFieldList);
  }
}

//------------------------------------------------------------------
bool RadxAppVolume::write(void)
{
  return write(_parms->output_url);
}

//------------------------------------------------------------------
bool RadxAppVolume::write(const std::string &path)
{
  _vol.loadVolumeInfoFromRays();
  _vol.loadSweepInfoFromRays();
  _vol.setPackingFromRays();

  RadxFile *outFile;
  NcfRadxFile *ncfFile = new NcfRadxFile();
  outFile = ncfFile;
  switch (_parms->netcdf_style) {
    case RadxAppParams::NETCDF4_CLASSIC:
      ncfFile->setNcFormat(RadxFile::NETCDF4_CLASSIC);
      break;
    case RadxAppParams::NC64BIT:
      ncfFile->setNcFormat(RadxFile::NETCDF_OFFSET_64BIT);
      break;
    case RadxAppParams::NETCDF4:
      ncfFile->setNcFormat(RadxFile::NETCDF4);
      break;
    default:
      ncfFile->setNcFormat(RadxFile::NETCDF_CLASSIC);
  }

  _setupWrite(*outFile);

  if (outFile->writeToDir(_vol, path,
                          _parms->append_day_dir_to_output_dir,
                          _parms->append_year_dir_to_output_dir))
  {
    LOG(ERROR) << "Cannot write file to dir: " <<  path;
    LOG(ERROR) << outFile->getErrStr();
    delete outFile;
    return false;
  }
  string outputPath = outFile->getPathInUse();
  delete outFile;

  string name = _nameWithoutPath(outputPath);
  LOG(DEBUG) << "Wrote output file: " << name;

  // in realtime mode, write latest data info file

  if (_parms->write_latest_data_info) {
    LdataInfo ldata(path);
    if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
    {
      ldata.setDebug(true);
    }
    RadxPath rpath(outputPath);
    ldata.setRelDataPath(rpath.getFile());
    ldata.setWriter("RadxAppVolume");
    if (ldata.write(_vol.getEndTimeSecs()))
    {
      LOG(WARNING) << "Cannot write latest data info file to dir: "
		   << _parms->output_url;
    }
  }
  return true;

}

//---------------------------------------------------------------
void RadxAppVolume::rewind(void)
{
  if (_realtime)
  {
    LOG(ERROR) << "Cannot rewind in real time mode";
  }
  _pathIndex = 0;
}

//---------------------------------------------------------------
void RadxAppVolume::fastForward(void)
{
  if (_realtime)
  {
    LOG(ERROR) << "Cannot fast forward in real time mode";
  }
  _pathIndex = (int)_paths.size();
}

//---------------------------------------------------------------
bool RadxAppVolume::_processFile(const string &path)

{
  string name = _nameWithoutPath(path);
  LOG(DEBUG) << "Primary file: " <<  name;
  
  RadxFile primaryFile;
  _setupRead(primaryFile);
  if (primaryFile.readFromPath(path, _vol))
  {
    LOG(ERROR) << "Cannot read in primary file: " << name;
    LOG(ERROR) << primaryFile.getErrStr();
    return false;
  }

  _time = _vol.getEndTimeSecs();
  bool dateOnly;
  if (DataFileNames::getDataTime(path, _time, dateOnly))
  {
    LOG(ERROR) << "Cannot get time from file path: " << name;
    return false;
  }

  LOG(DEBUG) << "Time for primary file: " << RadxTime::strm(_time);

  // Search for secondary files
  for (size_t igroup = 0; igroup < _parms->_inputs._secondaryGroups.size();
       igroup++)
  {
    _activeGroup = _parms->_inputs._secondaryGroups[igroup];

    string secondaryPath;
    if (_activeGroup.isClimo)
    {
      secondaryPath = _activeGroup.dir + "/" + _activeGroup.climoFile;
    }
    else
    {
      time_t searchTime = _time + _activeGroup.fileTimeOffset;
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

    string secondaryName = _nameWithoutPath(secondaryPath);
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
    if (!_mergeVol(secondaryVol))
    {
      LOG(ERROR) << "Merge failed, primary:" << name
		 << "secondary:" << secondaryName;
      return false;
    }

  } // igroup

  
  // remove some bad stuff we never will want
  _vol.removeTransitionRays();
  _vol.trimSurveillanceSweepsTo360Deg();
  

  LOG(DEBUG) << "-------Triggered " << RadxTime::strm(_time) << " ----------";

  _rays = &_vol.getRays();
  _sweeps = &_vol.getSweeps();
  if (!_rays->empty())
  {
    // just to have something around
    _ray = (*_rays)[0];
  }
  else
  {
    _ray = NULL;
  }

  return true;
}

//------------------------------------------------------------------
void RadxAppVolume::_setupRead(RadxFile &file)
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

  if (_parms->read_set_fixed_angle_limits)
  {
    file.setReadFixedAngleLimits(_parms->read_lower_fixed_angle,
                                 _parms->read_upper_fixed_angle);
  }

  for (size_t ii = 0; ii < _parms->_inputs._primaryGroup.names.size(); ii++) {
    file.addReadField(_parms->_inputs._primaryGroup.names[ii]);
  }

  if (_parms->ignore_antenna_transitions) {
    file.setReadIgnoreTransitions(true);
  }

  file.setReadAggregateSweeps(false);

  if (_parms->ignore_idle_scan_mode_on_read) {
    file.setReadIgnoreIdleMode(true);
  } else {
    file.setReadIgnoreIdleMode(false);
  }

  if (_parms->set_max_range) {
    file.setReadMaxRangeKm(_parms->max_range_km);
  }

  
  LOG(DEBUG_VERBOSE) << "===== SETTING UP READ FOR PRIMARY FILES =====";
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.printReadRequest(cerr);
  }
  LOG(DEBUG_VERBOSE) << "=============================================";
}



//------------------------------------------------------------------
void RadxAppVolume::_setupSecondaryRead(RadxFile &file)
{
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.setDebug(true);
  }
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.setVerbose(true);
  }

  if (_parms->read_set_fixed_angle_limits)
  {
    file.setReadFixedAngleLimits(_parms->read_lower_fixed_angle,
                                 _parms->read_upper_fixed_angle);
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
bool RadxAppVolume::_mergeVol(const RadxVol &secondaryVol)
{
  // loop through all rays in primary vol

  const vector<RadxRay *> &pRays = _vol.getRays();
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
void RadxAppVolume::_mergeRay(RadxRay &primaryRay,
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

//------------------------------------------------------------------
void RadxAppVolume::_setupWrite(RadxFile &file)
{
  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.setDebug(true);
  }

  if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG_VERBOSE))
  {
    file.setVerbose(true);
  }
  
  if (_parms->output_filename_mode == RadxAppParams::START_TIME_ONLY) {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_TIME_ONLY);
  } else if (_parms->output_filename_mode == RadxAppParams::END_TIME_ONLY) {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_END_TIME_ONLY);
  } else {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_AND_END_TIMES);
  }

  if (_parms->output_compressed) {
    file.setWriteCompressed(true);
    file.setCompressionLevel(_parms->compression_level);
  } else {
    file.setWriteCompressed(false);
  }

  if (_parms->output_native_byte_order) {
    file.setWriteNativeByteOrder(true);
  } else {
    file.setWriteNativeByteOrder(false);
  }

  file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
}


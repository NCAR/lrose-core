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
/////////////////////////////////////////////////////////////
// NexradRadxFile.cc
//
// NexradRadxFile object
//
// NetCDF file data for radar radial data in FORAY format
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2010
//
///////////////////////////////////////////////////////////////

#include <Radx/NexradRadxFile.hh>
#include <Radx/NexradLoc.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/ByteOrder.hh>
#include <Radx/RadxRemap.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxStr.hh>
#include <Radx/RadxReadDir.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <cerrno>
#include <unistd.h>
#include <dirent.h>
#include <algorithm>
#include <bzlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
using namespace std;

const double NexradRadxFile::_prtTable[5][8] =
  {
    {3066.7,2213.3,1533.3,1153.3,973.3,900.0,833.3,766.7},
    {3086.7,2226.7,1546.7,1160.0,980.0,906.7,840.0,773.3},
    {3106.7,2240.0,1553.3,1166.7,986.7,913.3,846.7,780.0},
    {3120.0,2253.3,1560.0,1173.3,993.3,920.0,853.3,786.7},
    {3140.0,2266.7,1573.3,1180.0,1000.0,926.7,860.0,793.3}
  };

const double NexradRadxFile::_angleMult = 360.0 / 65536;
const double NexradRadxFile::_rateMult = 90.0 / 65536;

//////////////
// Constructor

NexradRadxFile::NexradRadxFile() : 
        RadxFile(),
        _vcp11(11),
        _vcp12(12),
        _vcp21(21),
        _vcp31(31),
        _vcp32(32),
        _vcp35(35),
        _vcp121(121),
        _vcp211(211),
        _vcp212(212),
        _vcp215(215),
        _vcp221(221)
  
{

  _writeVol = NULL;
  _readVol = NULL;
  _file = NULL;
  _isBzipped = false;
  clear();

}

/////////////
// destructor

NexradRadxFile::~NexradRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void NexradRadxFile::clear()
  
{
  
  clearErrStr();

  _close();

  _startTimeSecs = 0;
  _endTimeSecs = 0;
  _startNanoSecs = 0;
  _endNanoSecs = 0;
  _writeFileNameMode = FILENAME_WITH_START_TIME_ONLY;

  _volumeNumber = Radx::missingMetaInt;
  _vcpNum = Radx::missingMetaInt;
  _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
  _platformType = Radx::PLATFORM_TYPE_FIXED;

  _instrumentName = "unknown";
  
  _latitude = Radx::missingMetaDouble;
  _longitude = Radx::missingMetaDouble;
  _altitudeM = Radx::missingMetaDouble;
  _sensorHtAglM = Radx::missingMetaDouble;
  _systemZdr = Radx::missingMetaDouble;
  _systemPhidp = Radx::missingMetaDouble;
  _atmosAttenDbPerKm = Radx::missingMetaDouble;
  _xmitFreqGhz = Radx::missingMetaDouble;
  _nyquistVelocity = Radx::missingMetaDouble;
  _unambiguousRangeKm = Radx::missingMetaDouble;
  _prtIndex = 2;
  _prtNum = 0;
  _beamWidthH = 0.89;
  _beamWidthV = 0.89;
  _antGainHDb = 46.0;
  _antGainVDb = 46.0;
  _dbz0 = Radx::missingMetaDouble;
  _powerHDbm = Radx::missingMetaDouble;
  _powerVDbm = Radx::missingMetaDouble;

  _maxAbsVel = 0;
  _meanPulseWidthUsec = 0;

  _vcpPatternType = 0;
  _vcpPatternNumber = 0;
  _vcpNElev = 0;
  _vcpVelRes = 0.5;
  _vcpShortPulse = true;
  _vcpPpis.clear();

  _startRangeKmLong = 0.5;
  _gateSpacingKmLong = 1.0;
  _startRangeKmShort = 0.125;
  _gateSpacingKmShort = 0.25;

  _msgSeqNum = 0;

  memset(&_adap, 0, sizeof(_adap));
  memset(&_vcp, 0, sizeof(_vcp));
}

/////////////////////////////////////////////////////////
// Write data from volume to specified directory
//
// If addDaySubDir is true, a  subdir will be
// created with the name dir/yyyymmdd/
//
// If addYearSubDir is true, a subdir will be
// created with the name dir/yyyy/
//
// If both addDaySubDir and addYearSubDir are true,
// the subdir will be dir/yyyy/yyyymmdd/
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getDirInUse() for dir written
// Use getPathInUse() for path written

int NexradRadxFile::writeToDir(const RadxVol &vol,
                               const string &dir,
                               bool addDaySubDir,
                               bool addYearSubDir)
  
{

  clearErrStr();
  _writeVol = &vol;
  _dirInUse = dir;
  _writePaths.clear();
  _writeDataTimes.clear();

  if (_debug) {
    cerr << "DEBUG - NexradRadxFile::writeToDir" << endl;
    cerr << "  Writing to dir: " << dir << endl;
  }

  RadxTime ftime(_writeVol->getStartTimeSecs());

  string outDir(dir);
  if (addYearSubDir) {
    char yearStr[BUFSIZ];
    sprintf(yearStr, "%s%.4d", PATH_SEPARATOR, ftime.getYear());
    outDir += yearStr;
  }
  if (addDaySubDir) {
    char dayStr[BUFSIZ];
    sprintf(dayStr, "%s%.4d%.2d%.2d", PATH_SEPARATOR,
            ftime.getYear(), ftime.getMonth(), ftime.getDay());
    outDir += dayStr;
  }

  // make sure output subdir exists
  
  if (makeDirRecurse(outDir)) {
    _addErrStr("ERROR - NexradRadxFile::writeToDir");
    _addErrStr("  Cannot make output dir: ", outDir);
    return -1;
  }
  
  // compute path
  
  int nSweeps = _writeVol->getNSweeps();
  string scanType = "SUR";
  if (nSweeps > 0) {
    const RadxSweep &sweep = *_writeVol->getSweeps()[0];
    scanType = Radx::sweepModeToShortStr(sweep.getSweepMode());
  }
  int volNum = _writeVol->getVolumeNumber();
  string outName =
    _computeFileName(volNum,
                     _writeVol->getInstrumentName(), scanType,
                     ftime.getYear(), ftime.getMonth(), ftime.getDay(),
                     ftime.getHour(), ftime.getMin(), ftime.getSec());
  string outPath(outDir);
  outPath += PATH_SEPARATOR;
  outPath += outName;
  
  int iret = writeToPath(*_writeVol, outPath);

  if (iret) {
    _addErrStr("ERROR - UfRadxFile::_writeToDir");
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified path
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int NexradRadxFile::writeToPath(const RadxVol &vol,
                                const string &path)
  
{

  clearErrStr();
  _writeVol = &vol;
  _pathInUse = path;
  vol.setPathInUse(_pathInUse);
  _writePaths.clear();
  _writeDataTimes.clear();
  _msgSeqNum = 0;

  // open the output Nc file

  string tmpPath(tmpPathFromFilePath(path, ""));
  
  if (_debug) {
    cerr << "DEBUG - NexradRadxFile::writeToPath" << endl;
    cerr << "  Writing to path: " << path << endl;
    cerr << "  Tmp path: " << tmpPath << endl;
  }

  if (_openWrite(tmpPath)) {
    _addErrStr("ERROR - NexradRadxFile::writeToPath");
    _addErrStr("  Cannot open tmp uf file: ", tmpPath);
    return -1;
  }

  // determine the max absolute velocity, so we can scale
  // velocity correctly

  _computeMaxAbsVel(vol);

  // determine the mean pulse width

  _computeMeanPulseWidth(vol);

  // write metadata headers

  if (_writeMetaDataHdrs(vol)) {
    _addErrStr("ERROR - NexradRadxFile::writeToPath");
    _addErrStr("  Writing header blocks, path: ", path);
    _close();
    return -1;
  }

  for (size_t isweep = 0; isweep < vol.getNSweeps(); isweep++) {

    const RadxSweep &sweep = *vol.getSweeps()[isweep];

    int rayNum = 0;
    for (size_t iray = sweep.getStartRayIndex();
         iray <= sweep.getEndRayIndex(); iray++, rayNum++) {
      
      // write ray block
      
      const RadxRay &ray = *vol.getRays()[iray];
      
      if (_writeMsg31(vol, isweep, sweep, rayNum, ray)) {
        _addErrStr("ERROR - NexradRadxFile::writeToPath");
        _addErrStr("  Writing ray block, path: ", path);
        _close();
        return -1;
      }
      
    } // iray

  } // isweep
  
  // close output file
  
  _close();
  
  // rename the tmp to final output file path
  
  if (rename(tmpPath.c_str(), _pathInUse.c_str())) {
    int errNum = errno;
    _addErrStr("ERROR - NexradRadxFile::writeToPath");
    _addErrStr("  Cannot rename tmp file: ", tmpPath);
    _addErrStr("  to: ", _pathInUse);
    _addErrStr(strerror(errNum));
    return -1;
  }

  if (_debug) {
    cerr << "DEBUG - NexradRadxFile::writeToPath" << endl;
    cerr << "  Renamed tmp path: " << tmpPath << endl;
    cerr << "     to final path: " << path << endl;
  }

  _writePaths.push_back(path);
  _writeDataTimes.push_back(vol.getStartTimeSecs());
 
  return 0;

}

/////////////////////////////////////////////////////////
// Check if specified file is a NEXRAD file
// Returns true if supported, false otherwise

bool NexradRadxFile::isSupported(const string &path)

{
  
  if (isNexrad(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a NEXRAD level2 file
// Returns true on success, false on failure
// Side effect:
//   sets _isBzipped if file is internally compressed

bool NexradRadxFile::isNexrad(const string &path)
  
{

  clear();
  
  // open file

  _close();
  _file = fopen(path.c_str(), "r");
  if (!_file) {
    int errNum = errno;
    _addErrStr("ERROR - NexradRadxFile::isNexrad");
    _addErrStr("  Cannot open file for reading, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return false;
  }
  
  // read buffer at start of file
  
  char buf[64];
  if (fread(buf, 1, 64, _file) != 64) {
    _close();
    return false;
  }
  _close();
  
  // check for ARCHIVE2 string at start of file
  
  if (strncmp(buf, "ARCHIVE2", 8) && strncmp(buf, "AR2V", 4)) {
    return false;
  }
  
  // check for bzipped archive2 file
  
  _isBzipped = false;
  Radx::si32 itest;
  memcpy(&itest, buf + 24, 4);
  if (itest != 0) {
    // expect 0s in this area, so file is probably compressed
    _isBzipped = true;
  }

  return true;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int NexradRadxFile::readFromPath(const string &path,
                                 RadxVol &vol)
  
{

  _initForRead(path, vol);

  // open file

  if (_openRead(_pathInUse)) {
    _addErrStr("ERROR - NexradRadxFile::readFromPath");
    return -1;
  }

  // volume title

  NexradData::vol_title_t title;
  if (fread(&title, sizeof(title), 1, _file) != 1) {
    _addErrStr("ERROR - NexradRadxFile::readFromPath");
    _addErrStr("  Cannot read title block");
    _addErrStr("  Path: ", _pathInUse);
    _close();
    return -1;
  }
  if (strncmp(title.filetype, "ARCHIVE2", 8) &&
      strncmp(title.filetype, "AR2V", 4)) {
    _addErrStr("ERROR - NexradRadxFile::readFromPath");
    _addErrStr("  Not an ARCHIVE2 file");
    _addErrStr("  Path: ", _pathInUse);
    _close();
    return -1;
  }
  NexradData::swap(title);
  if (_verbose) {
    NexradData::print(title, cerr);
  }
  
  if (_readPreserveSweeps &&
      !_readFixedAngleLimitsSet &&
      !_readSweepNumLimitsSet &&
      !_readRemoveRaysAllMissing &&
      !_readSetMaxRange &&
      _readFieldNames.size() == 0) {
    // no changes to file contents
    string origFormat = title.filetype;
    vol.setOrigFormat(origFormat.substr(0, 8));
  } else {
    vol.setOrigFormat("NEXRAD");
  }

  // set volume number if possible

  string volNumStr = Radx::makeString(title.vol_num, 3);
  int volNum;
  if (sscanf(volNumStr.c_str(), "%d", &volNum) == 1) {
    _volumeNumber = volNum;
  }

  // if possible initialize position etc from file path
  
  NexradLoc loc;
  if (loc.loadLocationFromFilePath(_pathInUse) == 0) {
    _latitude = loc.getLatDecimalDeg();
    _longitude = loc.getLonDecimalDeg();
    _altitudeM = loc.getHtMeters();
    _instrumentName = loc.getName();
  }
  
  // loop looking for message headers

  RadxBuf buf;
  NexradData::msg_hdr_t msgHdr;
  
  while (!feof(_file)) {
    
    if (_readMessage(msgHdr, buf, false, cerr)) {
      _addErrStr("ERROR - NexradRadxFile::readFromPath");
      _addErrStr("  Cannot read message");
      _addErrStr("  Path: ", _pathInUse);
      _close();
      return -1;
    }

    if (buf.getLen() == 0) {
      // done
      break;
    }
    
    if (_verbose) {
      NexradData::print(msgHdr, cerr);
    }

    if (!NexradData::msgTypeIsValid(msgHdr.message_type)) {
      cerr << "WARNING - NexradRadxFile::readFromPath" << endl;
      cerr << "  Bad message type: " << (int) msgHdr.message_type << endl;
      cerr << "  Path: " << _pathInUse << endl;
    }
    
    if (_verbose) {
      if (msgHdr.message_type == NexradData::DIGITAL_RADAR_DATA_31) {
        _printMessageType31(buf, cerr, true, false);
      } else if (msgHdr.message_type == NexradData::DIGITAL_RADAR_DATA_1) {
        _printMessageType1(buf, cerr, false, false);
      } else if (msgHdr.message_type == NexradData::VOLUME_COVERAGE_PATTERN) {
        _printVcp(buf, cerr);
      } else if (msgHdr.message_type == NexradData::RDA_ADAPTATION_DATA) {
        _printAdaptationData(buf, cerr);
      } else if (msgHdr.message_type == NexradData::CLUTTER_FILTER_BYPASS_MAP) {
        _printClutterFilterBypassMap(buf, cerr);
      } else if (msgHdr.message_type == NexradData::CLUTTER_FILTER_MAP) {
        _printClutterFilterMap(buf, cerr);
      } else {
        cerr << "====>> INFO - message type not yet handled <<====" << endl;
        cerr << "  Id: " << (int) msgHdr.message_type << ", "
             << NexradData::msgType2Str(msgHdr.message_type) << endl;
      }
    }
    
    if (msgHdr.message_type == NexradData::DIGITAL_RADAR_DATA_31) {

      // create ray from message

      RadxRay *ray = _handleMessageType31(buf);

      if (ray != NULL) {

        _checkIsLongRange(ray);
        
        // add ray to vol
        
        _readVol->addRay(ray);
        
        if (_verbose) {
          cerr << "Adding msg 31 ray, el, az: "
               << ray->getElevationDeg() << ", " << ray->getAzimuthDeg() << endl;
        }

      }
    } else if (msgHdr.message_type == NexradData::DIGITAL_RADAR_DATA_1) {

      // create ray from message

      RadxRay *ray = _handleMessageType1(buf);

      if (ray != NULL) {

        _checkIsLongRange(ray);

        // add ray to vol
        
        _readVol->addRay(ray);
        
        if (_verbose) {
          cerr << "Adding msg 1 ray, el, az: "
               << ray->getElevationDeg() << ", " << ray->getAzimuthDeg() << endl;
        }
        
      }

    } else if (msgHdr.message_type == NexradData::VOLUME_COVERAGE_PATTERN) {
      
      _handleVcpHdr(buf);
      
    } else if (msgHdr.message_type == NexradData::RDA_ADAPTATION_DATA) {
      
      if (_handleAdaptationData(buf)) {
        cerr << "WARNING - NexradRadxFile::readFromPath" << endl;
        cerr << "  Adaptation data probably not set, ignoring" << endl;
        cerr << "  File: " << _pathInUse << endl;
        // TODO - fix adaptation handling
      }
      
    }

  } // while

  // close file

  _close();
  
  if (_debug) {
    cerr << "VCP num: " << _vcpNum << endl;
  }

  // set sweep info

  _readVol->loadSweepInfoFromRays();

  // compute the fixed angle by averaging the elevation angles per sweep
  // if no VCP segment was found

  if (_readComputeSweepAnglesFromVcpTables){
    _computeFixedAngles();
  }

  // copy DBZ from long range to short range sweeps for split cuts

  // _copyDbzFromLongRangeSweeps();

  if (_readPreserveSweeps) {

    // keep sweeps just as they were read in

  } else {

    // interp long range geom to short range
    
    _readVol->remapRangeGeom(_startRangeKmShort, _gateSpacingKmShort, true);
    
    // combine fields as appropriate for sweeps with the same
    // fixed angle
    
    _readVol->combineSweepsAtSameFixedAngleAndGeom(!_readRemoveLongRange);
    
    // remove long or short range data as appropriate
    
    if (_readRemoveLongRange) {
      _removeLongRangeRays();
    } else if (_readRemoveShortRange) {
      _removeShortRangeRays();
    }
    
  }

  // put fields in standard order

  vector<string> ordered;
  if (_readFieldNames.size() > 0) {
    for (size_t ii = 0; ii < _readFieldNames.size(); ii++) {
      ordered.push_back(_readFieldNames[ii]);
    }
  } else {
    ordered.push_back("REF");
    ordered.push_back("VEL");
    ordered.push_back("SW");
    ordered.push_back("ZDR");
    ordered.push_back("PHI");
    ordered.push_back("RHO");
    ordered.push_back("REF-s0");
    ordered.push_back("REF-s1");
    ordered.push_back("REF-s2");
    ordered.push_back("REF-s3");
    ordered.push_back("REF-s4");
    ordered.push_back("REF-s5");
    ordered.push_back("REF-s6");
    ordered.push_back("REF-s7");
  }
  _readVol->reorderFieldsByName(ordered);

  // check we got some data
  
  if (_readVol->getRays().size() < 1) {
    _addErrStr("ERROR - NexradRadxFile::readFromPath");
    _addErrStr("  No valid rays found");
    return -1;
  }

  // finalize the read volume
  
  if (_finalizeReadVolume()) {
    return -1;
  }

  // add to paths used on read

  _readPaths.push_back(path);

  // set the packing from the rays

  _readVol->setPackingFromRays();
  
  // set format as read

  _fileFormat = FILE_FORMAT_NEXRAD_AR2;
  
  if (_verbose) {
    _readVol->print(cerr);
  }
  
  return 0;

}

////////////////////////////////////////////////////////////
// Read in a message, loading up header and buffer
// Returns 0 on success, -1 on failure

int NexradRadxFile::_readMessage(NexradData::msg_hdr_t &msgHdr,
                                 RadxBuf &buf,
                                 bool printHeaders,
                                 ostream &out)
  
{

  int messageType = -1;
  buf.clear();

  while (!feof(_file)) {

    // ctm info

    NexradData::ctm_info_t ctmInfo;
    if (fread(&ctmInfo, sizeof(ctmInfo), 1, _file) != 1) {
      if (feof(_file)) {
        return 0;
      }
      _addErrStr("ERROR - NexradRadxFile::readFromPath");
      _addErrStr("  Cannot read ctmInfo block");
      _addErrStr("  Path: ", _pathInUse);
      return -1;
    }
    NexradData::swap(ctmInfo);
    if (_verbose) {
      NexradData::print(ctmInfo, cerr);
    }

    // read in message header

    if (fread(&msgHdr, sizeof(msgHdr), 1, _file) != 1) {
      if (feof(_file)) {
        return 0;
      }
      _addErrStr("ERROR - NexradRadxFile::_readMessage");
      _addErrStr("  Cannot read msgHdr block");
      return -1;
    }
    NexradData::swap(msgHdr);
    if (printHeaders) {
      NexradData::print(msgHdr, out);
    }

    if (messageType >= 0 && messageType != msgHdr.message_type) {
      if (_debug) {
        // message type has changed - print warning
        cerr << "WARNING - NexradRadxFile::_readMessage" << endl;
        cerr << "  Message type has changed" << endl;
        cerr << "    from id: " << messageType << endl;
        cerr << "           = " << NexradData::msgType2Str(messageType) << endl;
        cerr << "      to id: " << (int) msgHdr.message_type << endl;
        cerr << "           = "
             << NexradData::msgType2Str(msgHdr.message_type) << endl;
        // zero out buffer
      }
      buf.clear();
    }
    
    // how many bytes to read?

    int bytesToRead;
    if (msgHdr.message_type == 31) {
      bytesToRead = msgHdr.message_len * 2 - sizeof(msgHdr);
    } else {
      bytesToRead = NexradData::PACKET_SIZE - sizeof(ctmInfo) - sizeof(msgHdr);
    }
    
    RadxBuf tmpBuf;
    char *tmp = (char *) tmpBuf.reserve(bytesToRead);
    int nread = fread(tmp, 1, bytesToRead, _file);
    if (nread != bytesToRead) {
      if (feof(_file)) {
        return 0;
      }
      _addErrStr("ERROR - NexradRadxFile::_readMessage");
      _addErrStr("  Cannot read message body");
      return -1;
    }

    // add to buffer

    buf.add(tmpBuf.getPtr(), tmpBuf.getLen());

    if (msgHdr.message_seg_num == msgHdr.num_message_segs) {
      // done reading
      return 0;
    }

    messageType = msgHdr.message_type;

  } // while

  return -1;

}
    
//////////////////////////////////////////////////////////////////////////////
// handle message type 1 on read
// creates ray
// returns pointer to created ray

RadxRay *NexradRadxFile::_handleMessageType1(const RadxBuf &msgBuf)

{

  // check buffer
  
  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();
  
  if (len < sizeof(NexradData::message_1_t)) {
    cerr << "WARNING - NexradRadxFile::_handleMessageType1" << endl;
    cerr << "  Buffer too small, size: " << msgBuf.getLen() << endl;
    cerr << "  Should be at least: " << sizeof(NexradData::message_1_t) << endl;
    return NULL;
  }

  // create a new ray

  RadxRay *ray = new RadxRay;

  // load up header structs
  
  NexradData::message_1_t hdr;
  memcpy(&hdr, buf, sizeof(hdr));
  NexradData::swap(hdr);

  ray->setVolumeNumber(_volumeNumber);
  ray->setSweepNumber(hdr.elev_num - 1);
  ray->setCalibIndex(0);
  ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);

  _dbz0 = Radx::missingFl32;
  _powerHDbm = Radx::missingFl32;
  _powerVDbm = Radx::missingFl32;

  ray->setMeasXmitPowerDbmH(_powerHDbm);
  ray->setMeasXmitPowerDbmV(_powerVDbm);
  
  _systemZdr = 0.0;
  _systemPhidp = 0.0;

  _vcpNum = hdr.vol_coverage_pattern;
  _atmosAttenDbPerKm = hdr.atmos_atten_factor / 1000.0;

  _unambiguousRangeKm = hdr.unamb_range_x10 / 10.0;
  _nyquistVelocity = hdr.nyquist_vel / 100.0;
  ray->setNyquistMps(_nyquistVelocity);
  ray->setUnambigRangeKm(_unambiguousRangeKm);
  ray->setPrtSec(Radx::LIGHT_SPEED / (_unambiguousRangeKm * 2000.0));
  ray->setPolarizationMode(Radx::POL_MODE_HORIZONTAL);
  ray->setPrtMode(Radx::PRT_MODE_FIXED);

  if (_vcpShortPulse) {
    ray->setPulseWidthUsec(1.5);
  } else {
    ray->setPulseWidthUsec(4.7);
  }
  
  int secs = hdr.millisecs_past_midnight / 1000;
  int msecs = hdr.millisecs_past_midnight - secs * 1000;
  double nanosecs = msecs * 1.0e6;
  time_t utimeSecs = (hdr.julian_date-1) * 86400 + secs;
  ray->setTime(utimeSecs, nanosecs);

  if (_startTimeSecs == 0) {
    _startTimeSecs = utimeSecs;
    _startNanoSecs = nanosecs;
  }
  _endTimeSecs = utimeSecs;
  _endNanoSecs = nanosecs;

  double el = (hdr.elevation / 8.0) * (180.0 / 4096.0);
  double az = (hdr.azimuth / 8.0) * (180.0 / 4096.0);

  ray->setAzimuthDeg(az);
  ray->setElevationDeg(el);

  _setRayProps(hdr.elev_num - 1, el, ray);

  // add the fields

  if (hdr.ref_ptr > 0) {
    _handleFieldType1(ray, "REF", "dBZ", hdr, msgBuf, hdr.ref_ptr);
  }
  if (hdr.vel_ptr > 0) {
    _handleFieldType1(ray, "VEL", "m/s", hdr, msgBuf, hdr.vel_ptr);
  }
  if (hdr.sw_ptr > 0) {
    _handleFieldType1(ray, "SW", "m/s", hdr, msgBuf, hdr.sw_ptr);
  }

  // set all fields to have the same number of gates

  ray->setNGatesConstant();

  return ray;

}

//////////////////////////////////////////////////////////////////////////////
// handle message type 31 on read
// creates ray
// returns pointer to created ray

RadxRay *NexradRadxFile::_handleMessageType31(const RadxBuf &msgBuf)

{

  // check buffer

  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();
  
  if (len < sizeof(NexradData::message_31_hdr_t)) {
    cerr << "WARNING - NexradRadxFile::_handleMessageType31" << endl;
    cerr << "  Buffer too small, size: " << msgBuf.getLen() << endl;
    cerr << "  Should be at least: " << sizeof(NexradData::message_31_hdr_t) << endl;
    return NULL;
  }

  // create a new ray
  
  RadxRay *ray = new RadxRay;
  
  // load up header structs
  
  NexradData::message_31_hdr_t hdr;
  memcpy(&hdr, buf, sizeof(hdr));
  NexradData::swap(hdr);
  
  _instrumentName = Radx::makeString(hdr.radar_icao, 4);
  if (_instrumentName.size() < 1) {
    _instrumentName = "unknown";
  }

  ray->setVolumeNumber(_volumeNumber);
  ray->setSweepNumber(hdr.elev_num - 1);
  ray->setCalibIndex(-1);
  ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
  
  if (_vcpShortPulse) {
    ray->setPulseWidthUsec(1.5);
  } else {
    ray->setPulseWidthUsec(4.7);
  }
  
  int secs = hdr.millisecs_past_midnight / 1000;
  int msecs = hdr.millisecs_past_midnight - secs * 1000;
  double nanosecs = msecs * 1.0e6;
  time_t utimeSecs = (hdr.julian_date-1) * 86400 + secs;
  ray->setTime(utimeSecs, nanosecs);

  if (_startTimeSecs == 0) {
    _startTimeSecs = utimeSecs;
    _startNanoSecs = nanosecs;
  }
  _endTimeSecs = utimeSecs;
  _endNanoSecs = nanosecs;

  ray->setAzimuthDeg(hdr.azimuth);
  ray->setElevationDeg(hdr.elevation);

  _setRayProps(hdr.elev_num - 1, hdr.elevation, ray);

  // handle data blocks

  memset(&_vol, 0, sizeof(_vol));
  memset(&_elev, 0, sizeof(_elev));
  memset(&_radial, 0, sizeof(_radial));
  _isDualPol = false;
  _isMsg1 = false;

  for (int ii = 0; ii < NexradData::MAX_DATA_BLOCKS; ii++) {
    if (hdr.data_block_offset[ii] > 0) {
      _handleDataBlockType31(ray, msgBuf, ii, hdr.data_block_offset[ii]);
    }
  }

  // set polarization mode

  if (_isDualPol) {
    ray->setPolarizationMode(Radx::POL_MODE_HV_SIM);
  }
  
  // set all fields to have the same number of gates

  ray->setNGatesConstant();

  return ray;

}

//////////////////////////////////////////////////////////////////////////////
// set ray properties

void NexradRadxFile::_setRayProps(int sweepNum, double elevation, RadxRay *ray)
  
{

  double fixedAngle = elevation;
  double scanRate = Radx::missingMetaDouble;
  double prt = 999.9;
  double prt2 = 999.9;
  int nSamples = 0;
  bool staggeredPrt = false;
  if (sweepNum < (int) _vcpPpis.size()) {
    int ppiIndex = sweepNum;
    const NexradData::ppi_hdr_t &ppi = _vcpPpis[ppiIndex];
    fixedAngle = ppi.elevation_angle * _angleMult;
    scanRate = ppi.azimuth_rate * _rateMult;
    int prfNum = ppi.doppler_prf_number1 - 1;
    int prfNum2 = ppi.doppler_prf_number2 - 1;
    nSamples = ppi.doppler_prf_pulse_count1;
    if (ppi.waveform_type == 1) {
      // surveillance
      prfNum = ppi.surveillance_prf_num;
      nSamples = ppi.surveillance_prf_pulse_count;
    } else if (ppi.waveform_type == 5) {
      // staggered prt
      staggeredPrt = true;
    }
    if (prfNum >= 0 && prfNum < 8 &&
        _prtIndex >= 0 && _prtIndex < 5) {
      prt = _prtTable[_prtIndex][prfNum];
    }
    if (prfNum2 >= 0 && prfNum2 < 8 &&
        _prtIndex >= 0 && _prtIndex < 5) {
      prt2 = _prtTable[_prtIndex][prfNum];
    }
  }
  ray->setFixedAngleDeg(fixedAngle);
  ray->setTargetScanRateDegPerSec(scanRate);
  ray->setNSamples(nSamples);
  ray->setPrtSec(prt / 1.0e6);
  if (staggeredPrt) {
    ray->setPrtMode(Radx::PRT_MODE_STAGGERED);
    ray->setPrtRatio(prt / prt2);
  } else {
    ray->setPrtMode(Radx::PRT_MODE_FIXED);
    ray->setPrtRatio(1.0);
  }

}
  
//////////////////////////////////////////////////////////////////////////////
// handle VCP header

void NexradRadxFile::_handleVcpHdr(const RadxBuf &msgBuf)

{

  // check buffer

  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();
  
  if (len < sizeof(NexradData::VCP_hdr_t)) {
    cerr << "WARNING - NexradRadxFile::_handleVcpHdr" << endl;
    cerr << "  Buffer too small, size: " << msgBuf.getLen() << endl;
    cerr << "  Should be at least: " << sizeof(NexradData::VCP_hdr_t) << endl;
    return;
  }
  
  memcpy(&_vcp, buf, sizeof(_vcp));
  NexradData::swap(_vcp);
  if (_verbose) {
    NexradData::print(_vcp, cerr);
  }

  _vcpPatternType = _vcp.pattern_type;
  _vcpPatternNumber = _vcp.pattern_number;
  _vcpNElev = _vcp.num_elevation_cuts;

  if (_vcp.dop_vel_resolution == 2) {
    _vcpVelRes = 0.5;
  } else {
    _vcpVelRes = 1.0;
  }

  if (_vcp.pulse_width == 2) {
    _vcpShortPulse = true;
  } else {
    _vcpShortPulse = false;
  }

  // elevation angles - ppi struct

  int nbytesPpi = _vcpNElev * sizeof(NexradData::ppi_hdr_t);
  if (len < sizeof(NexradData::VCP_hdr_t) + nbytesPpi) {
    cerr << "WARNING - NexradRadxFile::_handleVcpHdr" << endl;
    cerr << "  Buffer too small, size: " << msgBuf.getLen() << endl;
    cerr << "  Should be at least: " << sizeof(NexradData::VCP_hdr_t) << endl;
    cerr << "  Need space for n elev: " << _vcpNElev << endl;
    return;
  }
  
  RadxBuf ppibuf;
  NexradData::ppi_hdr_t *ppis = (NexradData::ppi_hdr_t *) ppibuf.reserve(nbytesPpi);
  memcpy(ppis, buf + sizeof(NexradData::VCP_hdr_t), nbytesPpi);

  _vcpPpis.clear();
  for (int ii = 0; ii < _vcpNElev; ii++) {
    NexradData::swap(ppis[ii]);
    if (_verbose) {
      NexradData::print(ppis[ii], cerr);
    }
    _vcpPpis.push_back(ppis[ii]);
  }

}

//////////////////////////////////////////////////////////////////////////////
// handle adaptation data on read

int NexradRadxFile::_handleAdaptationData(const RadxBuf &msgBuf)

{

  // check buffer

  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();
  
  if (len < sizeof(NexradData::adaptation_data_t)) {
    cerr << "WARNING - NexradRadxFile::_handleAdaptationData" << endl;
    cerr << "  Buffer too small, size: " << msgBuf.getLen() << endl;
    cerr << "  Should be at least: " <<
      sizeof(NexradData::adaptation_data_t) << endl;
    cerr << "  Ignoring adaptation data" << endl;
    return -1;
  }

  memcpy(&_adap, buf, sizeof(_adap));
  NexradData::swap(_adap);

  if (_adap.beamwidth < 0.25 || _adap.beamwidth > 5) {
    // TODO - fix adaptation data from ICD - dixon
    _antGainHDb = 46.0;
    _antGainVDb = 46.0;
    _beamWidthH = 0.89;
    _beamWidthV = 0.89;
    return -1;
  }

  _antGainHDb = _adap.antenna_gain;
  _antGainVDb = _adap.antenna_gain;

  _beamWidthH = _adap.beamwidth;
  _beamWidthV = _adap.beamwidth;

  _xmitFreqGhz = _adap.tfreq_mhz / 1000.0;
  _prtIndex = _adap.deltaprf - 1;

  _siteName = Radx::makeString(_adap.site_name, 4);

  _latitude = _adap.slatdeg + _adap.slatmin / 60.0 + _adap.slatsec / 3600.0;
  if (_adap.slatdir[0] == 'S') {
    _latitude *= -1.0;
  }

  _longitude = _adap.slondeg + _adap.slonmin / 60.0 + _adap.slonsec / 3600.0;
  if (_adap.slondir[0] == 'W') {
    _longitude *= -1.0;
  }

  int iret = 0;
  
  if (_adap.antenna_gain < 30 || _adap.antenna_gain > 50) {
    _addErrStr("ERROR - NexradRadxFile::_handleAdaptationData");
    _addErrStr("  bad value for antenna gain");
    _addErrDbl("  antenna_gain: ", _adap.antenna_gain, "%lg", false);
    _addErrStr("  bad adaptation data");
    iret = -1;
  }
  
  if (_adap.beamwidth < 0.25 || _adap.beamwidth > 5) {
    _addErrStr("ERROR - NexradRadxFile::_handleAdaptationData");
    _addErrStr("  bad value for beam width");
    _addErrDbl("  beam_width: ", _adap.beamwidth, "%lg", false);
    _addErrStr("  bad adaptation data");
    iret = -1;
  }

  return iret;

}

//////////////////////////////////////////////////////////////////////////////
// handle field from message 3

void NexradRadxFile::_handleFieldType1(RadxRay *ray,
                                       const string &fieldName,
                                       const string &units,
                                       const NexradData::message_1_t &hdr,
                                       const RadxBuf &msgBuf,
                                       size_t fieldOffset)
  
{

  if (fieldOffset == 0) {
    // field not included
    return;
  }

  _isMsg1 = true;

  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  
  double startRangeKm;
  double gateSpacingKm;
  if (hdr.vel_ptr > 0) {
    // Doppler cut
    gateSpacingKm = hdr.vel_gate_width / 1000.0;
    startRangeKm = hdr.vel_gate1 / 1000.0;
  } else {
    // non-Doppler cut
    gateSpacingKm = (hdr.ref_gate_width / 1000.0) / 4.0;
    startRangeKm = (hdr.ref_gate1 / 1000.0) - (gateSpacingKm * 1.5);
  }

  int nGatesIn = hdr.vel_num_gates;
  bool doGateInterp = false;
  if (fieldName == "REF") {
    nGatesIn = hdr.ref_num_gates;
    doGateInterp = true;
  }
  
  string standard_name;
  string long_name;
  double scale = 0.5, bias = -32.0;
  if (fieldName == "REF") {
    scale = 0.5;
    bias = -33.0;
    standard_name = "equivalent_reflectivity_factor";
    long_name = "radar_reflectivity";
  } else if (fieldName == "VEL") {
    // vel data
    if (hdr.velocity_resolution == 4) {
      scale = 1.0;
      bias = -129.0;
    } else {
      scale = 0.5;
      bias = -64.5;
    }
    standard_name = "radial_velocity_of_scatterers_away_from_instrument";
    long_name = "radial_velocity";
  } else if (fieldName == "SW") {
    // width data
    scale = 0.5;
    bias = -64.5;
    standard_name = "doppler_spectrum_width";
    long_name = "spectrum_width";
  }

  if (nGatesIn < 0 || nGatesIn > 10000) {
    cerr << "WARNING - NexradRadxFile::_handleFieldType1" << endl;
    cerr << "  Bad number of input gates: " << nGatesIn << endl;
    cerr << "  Ignoring field: " << fieldName << endl;
    return;
  }

  // load unsigned byte vector

  const Radx::ui08 *bdata = buf + fieldOffset;
  vector<Radx::ui08> udata;
  udata.resize(nGatesIn);
  memcpy(udata.data(), bdata, nGatesIn);

  // load up signed byte vector, interpolating as required

  vector<Radx::si08> sdata;
  _loadSignedData(udata, sdata, doGateInterp);
  int nGatesOut = sdata.size();

  ray->setRangeGeom(startRangeKm, gateSpacingKm);

  RadxField *field = new RadxField(fieldName, units);
  field->setRangeGeom(startRangeKm, gateSpacingKm);
  field->setLongName(long_name);
  field->setStandardName(standard_name);
  field->setTypeSi08(Radx::missingSi08, scale, bias + 128.0 * scale);
  field->addDataSi08(nGatesOut, sdata.data());
  
  ray->addField(field);

  if (_verbose) {
    double maxRange = startRangeKm + nGatesOut * gateSpacingKm;
    cerr << "Adding msg1 field, sweep, el, az, nGatesOut, maxRange: "
         << fieldName << ", "
         << ray->getSweepNumber() << ", "
         << ray->getElevationDeg() << ", "
         << ray->getAzimuthDeg() << ", "
         << nGatesOut << ", "
         << maxRange << endl;
  }

}

//////////////////////////////////////////////////////////////////////////////
// handle data block from message 31 

void NexradRadxFile::_handleDataBlockType31(RadxRay *ray,
                                            const RadxBuf &msgBuf,
                                            int blockNum,
                                            size_t byteOffset)
  
{

  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();
  if (len < 4) {
    cerr << "WARNING - _handleDataBlockType31" << endl;
    cerr << "  Length too short: " << len << endl;
    return;
  }

  // get the identifier string

  char blockType[8];
  memset(blockType, 0, 8);
  memcpy(blockType, buf + byteOffset, 4);

  if (strncmp(blockType, "RVOL", 4) == 0) {
    // message_31_vol_t
    _handleVolBlockType31(ray, msgBuf, blockNum, byteOffset);
    return;
  }
  
  if (strncmp(blockType, "RELV", 4) == 0) {
    // message_31_elev_t 
    _handleElevBlockType31(ray, msgBuf, blockNum, byteOffset);
    return;
  }
    
  if (strncmp(blockType, "RRAD", 4) == 0) {
    // message_31_radial_t 
    _handleRadialBlockType31(ray, msgBuf, blockNum, byteOffset);
    return;
  }

  // otherwise this is a data field block

  _handleFieldType31(ray, msgBuf, blockNum, byteOffset);

}

//////////////////////////////////////////////////////////////////////////////
// handle vol block from message 31 

void NexradRadxFile::_handleVolBlockType31(RadxRay *ray,
                                           const RadxBuf &msgBuf,
                                           int blockNum,
                                           size_t byteOffset)
  
{

  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();
  if (len < sizeof(NexradData::message_31_vol_t)) {
    cerr << "WARNING - _handlVolBlockType31" << endl;
    cerr << "  Length too short: " << len << endl;
    cerr << "  Should be at least sizeof(NexradData::message_31_vol_t): "
         << sizeof(NexradData::message_31_vol_t) << endl;
    return;
  }

  memcpy(&_vol, buf + byteOffset, sizeof(_vol));
  NexradData::swap(_vol);
  
  _vcpNum = _vol.vol_coverage_pattern;
  
  _latitude = _vol.lat;
  _longitude = _vol.lon;
  _altitudeM = _vol.height + _vol.feedhorn_height;
  _sensorHtAglM = _vol.feedhorn_height;

  _dbz0 = _vol.dbz0;
  if (_vol.horiz_power > 0) {
    _powerHDbm = 10.0 * log10(_vol.horiz_power / 1.0e6);
  }
  if (_vol.vert_power > 0) {
    _powerVDbm = 10.0 * log10(_vol.vert_power / 1.0e6);
  }
  
  ray->setMeasXmitPowerDbmH(_powerHDbm);
  ray->setMeasXmitPowerDbmV(_powerVDbm);
  
  _systemZdr = _vol.system_zdr;
  _systemPhidp = _vol.system_phi;
  
  if (_vol.horiz_power > 0 && _vol.vert_power > 0) {
    ray->setPolarizationMode(Radx::POL_MODE_HV_SIM);
  } else if (_vol.vert_power > 0) {
    ray->setPolarizationMode(Radx::POL_MODE_VERTICAL);
  } else {
    ray->setPolarizationMode(Radx::POL_MODE_HORIZONTAL);
  } 

  return;
  
}
  
//////////////////////////////////////////////////////////////////////////////
// handle elev block from message 31 

void NexradRadxFile::_handleElevBlockType31(RadxRay *ray,
                                            const RadxBuf &msgBuf,
                                            int blockNum,
                                            size_t byteOffset)
  
{

  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();
  if (len < sizeof(NexradData::message_31_elev_t)) {
    cerr << "WARNING - _handlElevBlockType31" << endl;
    cerr << "  Length too short: " << len << endl;
    cerr << "  Should be at least sizeof(NexradData::message_31_elev_t): "
         << sizeof(NexradData::message_31_elev_t) << endl;
    return;
  }
  
  memcpy(&_elev, buf + byteOffset, sizeof(_elev));
  NexradData::swap(_elev);
  _atmosAttenDbPerKm = _elev.atmos / 1000.0;
  
  return;

}

//////////////////////////////////////////////////////////////////////////////
// handle radial block from message 31 

void NexradRadxFile::_handleRadialBlockType31(RadxRay *ray,
                                              const RadxBuf &msgBuf,
                                              int blockNum,
                                              size_t byteOffset)
  
{
  
  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();
  if (len < sizeof(NexradData::message_31_radial_t)) {
    cerr << "WARNING - _handlRadialBlockType31" << endl;
    cerr << "  Length too short: " << len << endl;
    cerr << "  Should be at least sizeof(NexradData::message_31_radial_t): "
         << sizeof(NexradData::message_31_radial_t) << endl;
    return;
  }
  
  memcpy(&_radial, buf + byteOffset, sizeof(_radial));
  NexradData::swap(_radial);
  _unambiguousRangeKm = _radial.unamb_range_x10 / 10.0;
  _nyquistVelocity = _radial.nyquist_vel / 100.0;
  ray->setNyquistMps(_nyquistVelocity);
  ray->setUnambigRangeKm(_unambiguousRangeKm);
  
  return;
  
}

//////////////////////////////////////////////////////////////////////////////
// handle field from message 31 

void NexradRadxFile::_handleFieldType31(RadxRay *ray,
                                        const RadxBuf &msgBuf,
                                        int blockNum,
                                        size_t byteOffset)
  
{

  _isMsg1 = false;

  Radx::ui08 *msgStart = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();
  
  NexradData::message_31_field_t fieldHdr;
  size_t minLen = byteOffset + sizeof(fieldHdr);
  
  if (len < minLen) {
    cerr << "WARNING - NexradRadxFile::_handleFieldType31" << endl;
    cerr << "  Buffer too small, size: " << len << endl;
    cerr << "  Should be at least: " << minLen << endl;
    return;
  }
  
  memcpy(&fieldHdr, msgStart + byteOffset, sizeof(fieldHdr));
  NexradData::swap(fieldHdr);

  string fieldName(Radx::makeString(fieldHdr.block_name, 3));
  string units;
  string standard_name;
  string long_name;
  if (fieldName == "REF") {
    units = "dBZ";
    standard_name = "equivalent_reflectivity_factor";
    long_name = "radar_reflectivity";
  } else if (fieldName == "VEL") {
    units = "m/s";
    standard_name = "radial_velocity_of_scatterers_away_from_instrument";
    long_name = "radial_velocity";
  } else if (fieldName == "SW") {
    units = "m/s";
    standard_name = "doppler_spectrum_width";
    long_name = "spectrum_width";
  } else if (fieldName == "ZDR") {
    units = "dB";
    standard_name = "log_differential_reflectivity_hv";
    long_name = "differential_reflectivity";
    _isDualPol = true;
  } else if (fieldName == "PHI") {
    units = "deg";
    standard_name = "differential_phase_hv";
    long_name = "differential_phase";
    _isDualPol = true;
  } else if (fieldName == "RHO") {
    units = "";
    standard_name = "cross_correlation_ratio_hv";
    long_name = "cross_correlation";
    _isDualPol = true;
  }

  // check byte width

  if (fieldHdr.data_size != 8 && fieldHdr.data_size != 16) {
    cerr << "WARNING - NexradRadxFile::_handleFieldType31" << endl;
    cerr << "  Field: " << fieldName << endl;
    cerr << "  Invalid data nbits: " << fieldHdr.data_size << endl;
    cerr << "  Should be 8 or 16" << endl;
    return;
  }

  double startRangeKm = fieldHdr.gate1 / 1000.0;
  double gateSpacingKm = fieldHdr.gate_width / 1000.0;
  int nGatesIn = fieldHdr.num_gates;

  if (nGatesIn < 0 || nGatesIn > 10000) {
    cerr << "WARNING - NexradRadxFile::_handleFieldType31" << endl;
    cerr << "  Bad number of input gates: " << nGatesIn << endl;
    cerr << "  Ignoring field: " << fieldName << endl;
    return;
  }

  // load up signed byte vector, interpolating as required

  int byteWidth = fieldHdr.data_size / 8;
  int dataOffset = byteOffset + sizeof(fieldHdr);

  bool doGateInterp = false;
  if (fieldHdr.gate_width != 250) {
    doGateInterp = true;
    gateSpacingKm /= 4.0;
    startRangeKm -= ((4.0 - 1.0) / 2.0) * gateSpacingKm;
  }

  int nGatesOut = nGatesIn;
  vector<Radx::si08> sdata08;
  vector<Radx::si16> sdata16;
  if (byteWidth == 2) {
    vector<Radx::ui16> udata16;
    Radx::ui16 *bdata = (Radx::ui16 *) (msgStart + dataOffset);
    for (int ii = 0; ii < nGatesIn; ii++) {
      udata16.push_back(bdata[ii]);
    }
    ByteOrder::swap16(udata16.data(), udata16.size() * byteWidth);
    _loadSignedData(udata16, sdata16, doGateInterp);
    nGatesOut = sdata16.size();
  } else {
    vector<Radx::ui08> udata08;
    Radx::ui08 *bdata = (Radx::ui08 *) (msgStart + dataOffset);
    for (int ii = 0; ii < nGatesIn; ii++) {
      udata08.push_back(bdata[ii]);
    }
    _loadSignedData(udata08, sdata08, doGateInterp);
    nGatesOut = sdata08.size();
  }

  double scaleNexrad = fieldHdr.scale;
  double offsetNexrad = fieldHdr.offset;

  RadxField *field = new RadxField(fieldName, units);
  field->setRangeGeom(startRangeKm, gateSpacingKm);
  field->setLongName(long_name);
  field->setStandardName(standard_name);

  if (byteWidth == 1) {

    double scaleRadx = 1.0 / scaleNexrad;
    double offsetRadx = (128.0 - offsetNexrad) / scaleNexrad;

    field->setTypeSi08(Radx::missingSi08, scaleRadx, offsetRadx);
    field->addDataSi08(nGatesOut, sdata08.data());

  } else if (byteWidth == 2) {

    double scaleRadx = 1.0 / scaleNexrad;
    double offsetRadx = (32768.0 - offsetNexrad) / scaleNexrad;

    field->setTypeSi16(Radx::missingSi16, scaleRadx, offsetRadx);
    field->addDataSi16(nGatesOut, sdata16.data());

  } else {

    cerr << "WARNING - NexradRadxFile::_handleFieldType31" << endl;
    cerr << "  Field: " << fieldName << endl;
    cerr << "  Invalid data nbits: " << fieldHdr.data_size << endl;
    cerr << "  Should be 8 or 16" << endl;
    return;

  }

  ray->setRangeGeom(startRangeKm, gateSpacingKm);
  ray->addField(field);

  if (_verbose) {
    double maxRange = startRangeKm + nGatesOut * gateSpacingKm;
    cerr << "Adding msg31 field, sweep, el, az, nGatesOut, maxRange: "
         << fieldName << ", "
         << ray->getSweepNumber() << ", "
         << ray->getElevationDeg() << ", "
         << ray->getAzimuthDeg() << ", "
         << nGatesOut << ", "
         << maxRange << endl;
  }

}

//////////////////////////////////////
// open file for reading
// Returns 0 on success, -1 on failure

int NexradRadxFile::_openRead(const string &path)
  
{

  _close();

  if (!isNexrad(path)) {
    _addErrStr("ERROR - NexradRadxFile::readFromPath");
    _addErrStr("  Not a NEXRAD level2 archive file");
    _addErrStr("  Path: ", _pathInUse);
    return -1;
  }

  _tmpPath.clear();
  if (_isBzipped) {
    // need to unzip the file into a temporary file
    // also sets _pathInUse to tmp path
    if (_unzipFile(path)) {
      cerr << "WARNING - - NexradRadxFile::readFromPath" << endl;
      cerr << "  Cannot uncompress zipped file" << endl;
      cerr << "  Path: " << path << endl;
      cerr << "  Continuing and assuming not bzipped" << endl;
      clearErrStr();
    }
  }
  
  string pathToOpen = path;
  if (_tmpPath.size() > 0) {
    pathToOpen = _tmpPath;
  }
  
  _file = fopen(pathToOpen.c_str(), "r");
  if (!_file) {
    int errNum = errno;
    _addErrStr("ERROR - NexradRadxFile::_openRead");
    _addErrStr("  Cannot open file for reading, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////
// open file for writing
// Returns 0 on success, -1 on failure

int NexradRadxFile::_openWrite(const string &path) 
{
  
  _close();
  _file = fopen(path.c_str(), "w");
  
  // Check that constructor succeeded
  
  if (_file == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - NexradRadxFile::_openWrite");
    _addErrStr("  Cannot open file for writing, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////
// close file if open

void NexradRadxFile::_close()
  
{
  
  // close file if open
  
  if (_file) {
    fclose(_file);
    _file = NULL;
  }

  // delete tmp file if applicable
  
  if (_tmpPath.size() > 0) {
    unlink(_tmpPath.c_str());
    _tmpPath.clear();
  }

}

/////////////////////////////////////////////////////////////
// remove long range rays

void NexradRadxFile::_removeLongRangeRays()
  
{

  // load up vectors of good and bad rays

  vector<RadxRay *> goodRays, badRays;
  vector<RadxRay *> fileRays = _readVol->getRays();
  for (size_t ii = 0; ii < fileRays.size(); ii++) {
    RadxRay *ray = fileRays[ii];
    if (ray->getIsLongRange()) {
      // non-Doppler cut
      badRays.push_back(ray);
    } else {
      // doppler cut
      goodRays.push_back(ray);
    }
  }

  _readVol->removeBadRays(goodRays, badRays);

}
  
/////////////////////////////////////////////////////////////
// remove short range rays

void NexradRadxFile::_removeShortRangeRays()
  
{

  // load up vectors of good and bad rays

  vector<RadxRay *> goodRays, badRays;
  vector<RadxRay *> fileRays = _readVol->getRays();
  for (size_t ii = 0; ii < fileRays.size(); ii++) {
    RadxRay *ray = fileRays[ii];
    if (ray->getIsLongRange()) {
      // non-Doppler cut
      goodRays.push_back(ray);
    } else {
      // doppler cut
      badRays.push_back(ray);
    }
  }

  _readVol->removeBadRays(goodRays, badRays);

}

//////////////////////////////////////
// check for non-Doppler long-range cut

void NexradRadxFile::_checkIsLongRange(RadxRay *ray)
  
{

  int waveformType = 1;

  if (ray->getSweepNumber() < (int) _vcpPpis.size()) {
    waveformType = (int) _vcpPpis[ray->getSweepNumber()].waveform_type;
  }

  if (waveformType == 1 && ray->getNFields() == 1) {
    ray->setIsLongRange(true);
    _startRangeKmLong = ray->getStartRangeKm();
    _gateSpacingKmLong = ray->getGateSpacingKm();
  } else {
    ray->setIsLongRange(false);
    _startRangeKmShort = ray->getStartRangeKm();
    _gateSpacingKmShort = ray->getGateSpacingKm();
  }

  if (_verbose) {
    double maxRange = (ray->getStartRangeKm() +
                       ray->getNGates() * ray->getGateSpacingKm());
    cerr << "-->> ray vcp, nfields, el, az, sweepNum, maxRangeKm, waveformType: "
         << _vcpNum << ", "
         << ray->getNFields() << ", "
         << ray->getElevationDeg() << ", "
         << ray->getAzimuthDeg() << ", "
         << ray->getSweepNumber() << " "
         << maxRange << ", "
         << waveformType << ", ";
    if (ray->getIsLongRange()) {
      cerr << "*";
    }
    cerr << endl;
  }

}

/////////////////////////////////////////////////////////////
// finalize the read volume

int NexradRadxFile::_finalizeReadVolume()
  
{

  _readVol->setScanId(_vcpNum);
  char vcpStr[128];
  sprintf(vcpStr, "vcp-%d", _vcpNum);
  _readVol->setScanName(vcpStr);
  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setPlatformType(_platformType);

  if (_xmitFreqGhz > 0) {
    _readVol->addFrequencyHz(_xmitFreqGhz * 1.0e9);
  }
  
  _readVol->setRadarAntennaGainDbH(_antGainHDb);
  _readVol->setRadarAntennaGainDbV(_antGainVDb);
  _readVol->setRadarBeamWidthDegH(_beamWidthH);
  _readVol->setRadarBeamWidthDegV(_beamWidthV);
  
  _readVol->setStartTime(_startTimeSecs, _startNanoSecs);
  _readVol->setEndTime(_endTimeSecs, _endNanoSecs);

  _readVol->setTitle("");
  _readVol->setSource("ARCHIVE 2 data");
  _readVol->setScanName("Surveillance");
  _readVol->setInstrumentName(_instrumentName);
  _readVol->setSiteName(_siteName);

  _readVol->setLatitudeDeg(_latitude);
  _readVol->setLongitudeDeg(_longitude);
  _readVol->setAltitudeKm(_altitudeM / 1000.0);
  _readVol->setSensorHtAglM(_sensorHtAglM);

  // set max range

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }
  
  // remove rays with all missing data, if requested

  if (_readRemoveRaysAllMissing) {
    _readVol->removeRaysWithDataAllMissing();
  }

  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();
  
  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - NexradRadxFile::_finalizeReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - NexradRadxFile::_finalizeReadVolume");
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
      return -1;
    }
  }

  // check for indexed rays, set info on rays
  
  _readVol->checkForIndexedRays();

  // load up volume info from rays

  _readVol->loadVolumeInfoFromRays();
  
  // add calibration

  RadxRcalib *calib = new RadxRcalib;
  calib->setBaseDbz1kmHc(_dbz0);
  calib->setZdrCorrectionDb(_systemZdr);
  calib->setSystemPhidpDeg(_systemPhidp);
  _readVol->addCalib(calib);

  return 0;

}

/////////////////////////////////////////////////////////////
// Compute the fixed angles by averaging the elevation angles
// on the sweeps

void NexradRadxFile::_computeFixedAngles()
  
{

  for (size_t isweep = 0; isweep < _readVol->getNSweeps(); isweep++) {

    RadxSweep &sweep = *_readVol->getSweeps()[isweep];

    double sumElev = 0.0;
    double count = 0.0;

    for (size_t iray = sweep.getStartRayIndex();
         iray <= sweep.getEndRayIndex(); iray++) {
      const RadxRay &ray = *_readVol->getRays()[iray];
      sumElev += ray.getElevationDeg();
      count++;
    }

    double meanElev = sumElev / count;

    double fixedAngle;
    int sweepNum;
    
    if (_vcpNum == 12) {
      fixedAngle = _vcp12.getClosestFixedAngle(meanElev, sweepNum);
    } else if (_vcpNum == 21) {
      fixedAngle = _vcp21.getClosestFixedAngle(meanElev, sweepNum);
    } else if (_vcpNum == 31) {
      fixedAngle = _vcp31.getClosestFixedAngle(meanElev, sweepNum);
    } else if (_vcpNum == 32) {
      fixedAngle = _vcp32.getClosestFixedAngle(meanElev, sweepNum);
    } else if (_vcpNum == 35) {
      fixedAngle = _vcp35.getClosestFixedAngle(meanElev, sweepNum);
    } else if (_vcpNum == 121) {
      fixedAngle = _vcp121.getClosestFixedAngle(meanElev, sweepNum);
    } else if (_vcpNum == 211) {
      fixedAngle = _vcp211.getClosestFixedAngle(meanElev, sweepNum);
    } else if (_vcpNum == 212) {
      fixedAngle = _vcp212.getClosestFixedAngle(meanElev, sweepNum);
    } else if (_vcpNum == 215) {
      fixedAngle = _vcp215.getClosestFixedAngle(meanElev, sweepNum);
    } else {
      // default to 11
      fixedAngle = _vcp11.getClosestFixedAngle(meanElev, sweepNum);
    }
  
    _readVol->setFixedAngleDeg(isweep, fixedAngle);

    if (_verbose) {
      cerr << "==>> vcp, meanElev, fixedAngle, isweep, sweepNum: "
           << _vcpNum << ", "
           << meanElev << ", "
           << fixedAngle << ", "
           << isweep << ", "
           << sweepNum << endl;
    }
      
  } // isweep

}

/////////////////////////////////////////////////////////
// print summary after read

void NexradRadxFile::print(ostream &out) const
  
{
  
  out << "=============== NexradRadxFile ===============" << endl;
  RadxFile::print(out);
  out << "  startTimeSecs: " << RadxTime::strm(_startTimeSecs) << endl;
  out << "  endTimeSecs: " << RadxTime::strm(_endTimeSecs) << endl;
  out << "  startNanoSecs: " << _startNanoSecs << endl;
  out << "  endNanoSecs: " << _endNanoSecs << endl;
  out << "  volumeNumber: " << _volumeNumber << endl;
  out << "  instrumentType: " << Radx::instrumentTypeToStr(_instrumentType) << endl;
  out << "  platformType: " << Radx::platformTypeToStr(_platformType) << endl;
  out << "  instrumentName: " << _instrumentName << endl;
  out << "  latitude: " << _latitude << endl;
  out << "  longitude: " << _longitude << endl;
  out << "  altitudeM: " << _altitudeM << endl;
  out << "  sensorHtAglM: " << _sensorHtAglM << endl;
  out << "  systemZdr: " << _systemZdr << endl;
  out << "  systemPhidp: " << _systemPhidp << endl;
  out << "  atmosAttenDbPerKm: " << _atmosAttenDbPerKm << endl;
  out << "  xmitFregGhz: " << _xmitFreqGhz << endl;
  out << "  nyquistVelocity: " << _nyquistVelocity << endl;
  out << "  unambiguousRangeKm: " << _unambiguousRangeKm << endl;
  out << "  beamWidthH: " << _beamWidthH << endl;
  out << "  beamWidthV: " << _beamWidthV << endl;
  out << "  antGainHDb: " << _antGainHDb << endl;
  out << "  antGainVDb: " << _antGainVDb << endl;
  out << "  dbz0: " << _dbz0 << endl;
  out << "  powerHDbm: " << _powerHDbm << endl;
  out << "  powerVDbm: " << _powerVDbm << endl;
  out << "===========================================" << endl;

}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Print native data in nexrad file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int NexradRadxFile::printNative(const string &path, ostream &out,
                                bool printRays, bool printData)
  
{

  _pathInUse = path;

  // open file

  if (_openRead(_pathInUse)) {
    _addErrStr("ERROR - NexradRadxFile::printNative");
    return -1;
  }

  // volume title

  NexradData::vol_title_t title;
  if (fread(&title, sizeof(title), 1, _file) != 1) {
    _addErrStr("ERROR - NexradRadxFile::printNative");
    _addErrStr("  Cannot read title block");
    _addErrStr("  Path: ", _pathInUse);
    _close();
    return -1;
  }
  if (strncmp(title.filetype, "ARCHIVE2", 8) &&
      strncmp(title.filetype, "AR2V", 4)) {
    _addErrStr("ERROR - NexradRadxFile::printNative");
    _addErrStr("  Not an ARCHIVE2 file");
    _addErrStr("  Path: ", _pathInUse);
    _close();
    return -1;
  }
  NexradData::swap(title);
  NexradData::print(title, out);

  // loop looking for message headers

  RadxBuf buf;
  NexradData::msg_hdr_t msgHdr;
  
  while (!feof(_file)) {
    
    if (_readMessage(msgHdr, buf, true, out)) {
      _addErrStr("ERROR - NexradRadxFile::readFromPath");
      _addErrStr("  Cannot read message");
      _addErrStr("  Path: ", _pathInUse);
      _close();
      return -1;
    }

    if (buf.getLen() == 0) {
      // done
      _close();
      return 0;
    }
    
    if (msgHdr.message_type == NexradData::DIGITAL_RADAR_DATA_31) {
      _printMessageType31(buf, out, printRays, printData);
    } else if (msgHdr.message_type == NexradData::DIGITAL_RADAR_DATA_1) {
      _printMessageType1(buf, out, printRays, printData);
    } else if (msgHdr.message_type == NexradData::VOLUME_COVERAGE_PATTERN) {
      _printVcp(buf, out);
    } else if (msgHdr.message_type == NexradData::RDA_ADAPTATION_DATA) {
      _printAdaptationData(buf, out);
    } else if (msgHdr.message_type == NexradData::CLUTTER_FILTER_BYPASS_MAP) {
      _printClutterFilterBypassMap(buf, out);
    } else if (msgHdr.message_type == NexradData::CLUTTER_FILTER_MAP) {
      _printClutterFilterMap(buf, out);
    } else {
      out << "====>> INFO - message type not yet handled <<====" << endl;
      out << "  Id: " << (int) msgHdr.message_type << ", "
          << NexradData::msgType2Str(msgHdr.message_type) << endl;
    }

  } // while
  
  _close();
  return 0;

}

//////////////////////////////////////////////////////////////////////////////
// print volume coverage pattern (VCP)

void NexradRadxFile::_printVcp(const RadxBuf &msgBuf, ostream &out)

{

  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();

  if (len < sizeof(NexradData::VCP_hdr_t)) {
    cerr << "WARNING - NexradRadxFile::_printVcpHdr" << endl;
    cerr << "  Buffer too small, size: " << len << endl;
    cerr << "  Should be at least: " << sizeof(NexradData::VCP_hdr_t) << endl;
    return;
  }

  NexradData::VCP_hdr_t vcp;
  memcpy(&vcp, buf, sizeof(vcp));
  NexradData::swap(vcp);
  NexradData::print(vcp, out);

}

//////////////////////////////////////////////////////////////////////////////
// print adaptation data

void NexradRadxFile::_printAdaptationData(const RadxBuf &msgBuf, ostream &out)

{
  
  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();

  if (len < sizeof(NexradData::adaptation_data_t)) {
    cerr << "WARNING - NexradRadxFile::_printAdaptationData" << endl;
    cerr << "  Buffer too small, size: " << len << endl;
    cerr << "  Should be at least: " << sizeof(NexradData::adaptation_data_t) << endl;
    return;
  }

  NexradData::adaptation_data_t adap;
  memcpy(&adap, buf, sizeof(adap));
  NexradData::swap(adap);
  NexradData::print(adap, out);

}

//////////////////////////////////////////////////////////////////////////////
// print clutter filter bypass map

void NexradRadxFile::_printClutterFilterBypassMap(const RadxBuf &msgBuf, ostream &out)
  
{
  
  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();

  if (len < sizeof(NexradData::clutter_hdr_t)) {
    cerr << "WARNING - NexradRadxFile::_printClutterFilterBypassMap" << endl;
    cerr << "  Buffer too small, size: " << len << endl;
    cerr << "  Should be at least: " << sizeof(NexradData::clutter_hdr_t) << endl;
    return;
  }

  out << "=====>> Clutter filter bypass map <<====" << endl;
  NexradData::clutter_hdr_t clutHdr;
  memcpy(&clutHdr, buf, sizeof(clutHdr));
  NexradData::swap(clutHdr);
  NexradData::print(clutHdr, out);
  out << "========================================" << endl;

}

//////////////////////////////////////////////////////////////////////////////
// print clutter filter map

void NexradRadxFile::_printClutterFilterMap(const RadxBuf &msgBuf, ostream &out)
  
{
  
  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();

  if (len < sizeof(NexradData::clutter_hdr_t)) {
    cerr << "WARNING - NexradRadxFile::_printClutterFilterMap" << endl;
    cerr << "  Buffer too small, size: " << len << endl;
    cerr << "  Should be at least: " << sizeof(NexradData::clutter_hdr_t) << endl;
    return;
  }

  out << "=====>> Clutter filter map <<====" << endl;
  NexradData::clutter_hdr_t clutHdr;
  memcpy(&clutHdr, buf, sizeof(clutHdr));
  NexradData::swap(clutHdr);
  NexradData::print(clutHdr, out);
  out << "=================================" << endl;

}

//////////////////////////////////////////////////////////////////////////////
// print message type 1
  
void NexradRadxFile::_printMessageType1(const RadxBuf &msgBuf, ostream &out,
                                        bool printRays, bool printData)

{

  size_t minLen = (NexradData::PACKET_SIZE -
                   sizeof(NexradData::ctm_info_t) -
                   sizeof(NexradData::msg_hdr_t));
  if (msgBuf.getLen() < minLen) {
    cerr << "WARNING - NexradRadxFile::_printMessageType1" << endl;
    cerr << "  Buffer too small, size: " << msgBuf.getLen() << endl;
    cerr << "  Should be at least: " << minLen << endl;
    return;
  }

  NexradData::message_1_t msg;
  memcpy(&msg, (char *) msgBuf.getPtr(), sizeof(msg));
  NexradData::swap(msg);

  if (printRays) {
    NexradData::print(msg, out);
  }

  if (printData) {
    
    if (msg.ref_num_gates > 0) {

      // DBZ

      Radx::ui08 *msgDbz =
        (Radx::ui08 *) msgBuf.getPtr() + sizeof(NexradData::message_1_t);
      double dbz[460];
      for (int ii = 0; ii < msg.ref_num_gates; ii++) {
        if (msgDbz[ii] == 0) {
          dbz[ii] = Radx::missingMetaDouble;
        } else {
          dbz[ii] = msgDbz[ii] * 0.5 - 32;
        }
      }
      _printFieldData(out, "DBZ", msg.ref_num_gates, dbz);

    } // if (msg.ref_num_gates > 0)

    if (msg.vel_num_gates > 0) {

      // VEL

      double velScale = 1.0;
      if (msg.velocity_resolution == NexradData::POINT_FIVE_METERS_PER_SEC) {
        velScale = 0.5;
      }

      Radx::ui08 *msgVel =
        (Radx::ui08 *) msgBuf.getPtr() + sizeof(NexradData::message_1_t)
        + msg.ref_num_gates;
      double vel[920];
      for (int ii = 0; ii < msg.vel_num_gates; ii++) {
        if (msgVel[ii] == 0) {
          vel[ii] = Radx::missingMetaDouble;
        } else {
          vel[ii] = ((int) msgVel[ii] -128.0) * velScale;
        }
      }
      _printFieldData(out, "VEL", msg.ref_num_gates, vel);
      
      // WIDTH

      Radx::ui08 *msgWidth =
        (Radx::ui08 *) msgBuf.getPtr() + sizeof(NexradData::message_1_t)
        + msg.ref_num_gates + msg.vel_num_gates;
      double width[920];
      for (int ii = 0; ii < msg.vel_num_gates; ii++) {
        if (msgWidth[ii] == 0) {
          width[ii] = Radx::missingMetaDouble;
        } else {
          width[ii] = ((int) msgWidth[ii] -128.0) * velScale;
        }
      }
      _printFieldData(out, "SW", msg.ref_num_gates, width);

    } // if (msg.vel_num_gates > 0) 

  } // if (printData) {
  
}

//////////////////////////////////////////////////////////////////////////////
// print message type 31

void NexradRadxFile::_printMessageType31(const RadxBuf &msgBuf, ostream &out,
                                         bool printRays, bool printData)

{

  // check buffer

  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();
  
  if (len < sizeof(NexradData::message_31_hdr_t)) {
    cerr << "WARNING - NexradRadxFile::_printMessageType31" << endl;
    cerr << "  Buffer too small, size: " << msgBuf.getLen() << endl;
    cerr << "  Should be at least: " << sizeof(NexradData::message_31_hdr_t) << endl;
    return;
  }
  
  // load up header struct
  
  NexradData::message_31_hdr_t hdr;
  memcpy(&hdr, buf, sizeof(hdr));
  NexradData::swap(hdr);
  if (printRays) {
    NexradData::print(hdr, out);
  }

  // handle data blocks

  memset(&_vol, 0, sizeof(_vol));
  memset(&_elev, 0, sizeof(_elev));
  memset(&_radial, 0, sizeof(_radial));
  _isDualPol = false;

  for (int ii = 0; ii < NexradData::MAX_DATA_BLOCKS; ii++) {
    if (hdr.data_block_offset[ii] > 0) {
      _printDataBlockType31(msgBuf, out, ii, hdr.data_block_offset[ii], 
                            printRays, printData);
    }
  }

}

//////////////////////////////////////////////////////////////////////////////
// print data block from message type 31

void NexradRadxFile::_printDataBlockType31(const RadxBuf &msgBuf,
                                           ostream &out,
                                           int blockNum,
                                           size_t byteOffset,
                                           bool printRays,
                                           bool printData)
  
{

  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();
  if (len < 4) {
    cerr << "WARNING - _printDataBlockType31" << endl;
    cerr << "  Length too short: " << len << endl;
    return;
  }

  // get the identifier string

  char blockType[8];
  memset(blockType, 0, 8);
  memcpy(blockType, buf + byteOffset, 4);
  
  if (strncmp(blockType, "RVOL", 4) == 0) {
    // message_31_vol_t
    if (printRays) {
      _printVolBlockType31(msgBuf, out, blockNum, byteOffset);
    }
    return;
  }
  
  if (strncmp(blockType, "RELV", 4) == 0) {
    // message_31_elev_t 
    if (printRays) {
      _printElevBlockType31(msgBuf, out, blockNum, byteOffset);
    }
    return;
  }
  
  if (strncmp(blockType, "RRAD", 4) == 0) {
    // message_31_radial_t 
    if (printRays) {
      _printRadialBlockType31(msgBuf, out, blockNum, byteOffset);
    }
    return;
  }
  
  // otherwise this is a data field block

  if (printRays) {
    _printFieldType31(msgBuf, out, blockNum, byteOffset, printData);
  }

}


//////////////////////////////////////////////////////////////////////////////
// print vol block from message 31 

void NexradRadxFile::_printVolBlockType31(const RadxBuf &msgBuf,
                                          ostream &out,
                                          int blockNum,
                                          size_t byteOffset)
  
{

  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();
  if (len < byteOffset + sizeof(NexradData::message_31_vol_t)) {
    cerr << "WARNING - _printVolBlockType31" << endl;
    cerr << "  Length too short: " << len << endl;
    cerr << "  Should be at least sizeof(NexradData::message_31_vol_t): "
         << sizeof(NexradData::message_31_vol_t) << endl;
    return;
  }
  
  memcpy(&_vol, buf + byteOffset, sizeof(_vol));
  NexradData::swap(_vol);
  out << "====>> Data block index: " << blockNum << " <<====" << endl;
  NexradData::print(_vol, out);
  
}
  
//////////////////////////////////////////////////////////////////////////////
// print elev block from message 31 

void NexradRadxFile::_printElevBlockType31(const RadxBuf &msgBuf,
                                           ostream &out,
                                           int blockNum,
                                           size_t byteOffset)
  
{
  
  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();
  if (len < byteOffset + sizeof(NexradData::message_31_elev_t)) {
    cerr << "WARNING - _printElevBlockType31" << endl;
    cerr << "  Length too short: " << len << endl;
    cerr << "  Should be at least sizeof(NexradData::message_31_elev_t): "
         << sizeof(NexradData::message_31_elev_t) << endl;
    return;
  }

  memcpy(&_elev, buf + byteOffset, sizeof(_elev));
  NexradData::swap(_elev);
  out << "====>> Data block index: " << blockNum << " <<====" << endl;
  NexradData::print(_elev, out);
  
}
  
//////////////////////////////////////////////////////////////////////////////
// print radial block from message 31 

void NexradRadxFile::_printRadialBlockType31(const RadxBuf &msgBuf,
                                             ostream &out,
                                             int blockNum,
                                             size_t byteOffset)
  
{
  
  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();
  if (len < byteOffset + sizeof(NexradData::message_31_radial_t)) {
    cerr << "WARNING - _printRadialBlockType31" << endl;
    cerr << "  Length too short: " << len << endl;
    cerr << "  Should be at least sizeof(NexradData::message_31_radial_t): "
         << sizeof(NexradData::message_31_radial_t) << endl;
    return;
  }

  memcpy(&_radial, buf + byteOffset, sizeof(_radial));
  NexradData::swap(_radial);
  out << "====>> Data block index: " << blockNum << " <<====" << endl;
  NexradData::print(_radial, out);
  
}
  
//////////////////////////////////////////////////////////////////////////////
// print field from message 31 

void NexradRadxFile::_printFieldType31(const RadxBuf &msgBuf,
                                       ostream &out,
                                       int blockNum,
                                       size_t byteOffset,
                                       bool printData)
  
{

  Radx::ui08 *buf = (Radx::ui08 *) msgBuf.getPtr();
  size_t len = msgBuf.getLen();

  NexradData::message_31_field_t fieldHdr;
  size_t minLen = byteOffset + sizeof(fieldHdr);

  if (len < minLen) {
    cerr << "WARNING - NexradRadxFile::_printFieldType31" << endl;
    cerr << "  Buffer too small, size: " << len << endl;
    cerr << "  Should be at least: " << minLen << endl;
    return;
  }
  
  memcpy(&fieldHdr, buf + byteOffset, sizeof(fieldHdr));
  NexradData::swap(fieldHdr);
  out << "====>> Data block index: " << blockNum << " <<====" << endl;
  NexradData::print(fieldHdr, out);
  string fieldName(Radx::makeString(fieldHdr.block_name, 3));
  
  if (printData) {

    int nGates = fieldHdr.num_gates;
    double scale = fieldHdr.scale;
    double offset = fieldHdr.offset;
    
    RadxArray<double> ddata_;
    double *ddata = ddata_.alloc(nGates);
    int dataOffset = byteOffset + sizeof(fieldHdr);
    if (fieldHdr.data_size == 8) {
      Radx::ui08 *udata = buf + dataOffset;
      for (int ii = 0; ii < nGates; ii++) {
        if (udata[ii] == 0) {
          ddata[ii] = Radx::missingMetaDouble;
        } else {
          ddata[ii] = ((double) udata[ii] - offset) / scale;
        }
      }
    } else if (fieldHdr.data_size == 16) {
      Radx::ui16 *udata = (Radx::ui16 *) (buf + dataOffset);
      ByteOrder::swap16(udata, nGates * 2);
      for (int ii = 0; ii < nGates; ii++) {
        if (udata[ii] == 0) {
          ddata[ii] = Radx::missingMetaDouble;
        } else {
          ddata[ii] = ((double) udata[ii] - offset) / scale;
        }
      }
    } else {
      cerr << "WARNING - NexradRadxFile::_printFieldType31" << endl;
      cerr << "  Field: " << fieldName << endl;
      cerr << "  Invalid data nbits: " << fieldHdr.data_size << endl;
      cerr << "  Should be 8 or 16" << endl;
      return;
    }

    _printFieldData(out, fieldName, nGates, ddata);
    
  } // if (printData)

}

//////////////////////////////////////////////////////////////////////////////
// print field data

void NexradRadxFile::_printFieldData(ostream &out, const string &fieldName,
                                     int nGates, const double *data) const
  
{
  
  out << "=========== Data for field " << fieldName << " =============" << endl;
  if (nGates == 0) {
    out << "========= currently no data =========" << endl;
  } else {
    int printed = 0;
    int count = 1;
    double prevVal = data[0];
    for (int ii = 1; ii < nGates; ii++) {
      double dval = data[ii];
      if (dval != prevVal) {
        _printPacked(out, count, prevVal);
        printed++;
        if (printed > 6) {
          out << endl;
          printed = 0;
        }
        prevVal = dval;
        count = 1;
      } else {
        count++;
      }
    } // ii
    _printPacked(out, count, prevVal);
    out << endl;
  }
  out << "===========================================" << endl;

}

//////////////////////////////////////////////////////////////////////////////
// print in packed format, using count for identical data values

void NexradRadxFile::_printPacked(ostream &out, int count, double val) const

{
  
  char outstr[1024];
  if (count > 1) {
    out << count << "*";
  }
  if (val == Radx::missingMetaDouble) {
    out << "MISS ";
  } else {
    if (fabs(val) > 0.01) {
      sprintf(outstr, "%.3f ", val);
      out << outstr;
    } else if (val == 0.0) {
      out << "0.0 ";
    } else {
      sprintf(outstr, "%.3e ", val);
      out << outstr;
    }
  }
}

////////////////////////////////////////////////////////////
// compute and return nexrad file name

string NexradRadxFile::_computeFileName(int volNum,
                                        string instrumentName,
                                        string scanType,
                                        int year, int month, int day,
                                        int hour, int min, int sec)
  
{

  // make sure instrument name is reasonable length

  if (instrumentName.size() > 8) {
    instrumentName.resize(8);
  }

  // replaces spaces in strings with underscores

  for (size_t ii = 0; ii < instrumentName.size(); ii++) {
    if (isspace(instrumentName[ii])) {
      instrumentName[ii] = '_';
    }
  }
  for (size_t ii = 0; ii < scanType.size(); ii++) {
    if (isspace(scanType[ii] == ' ')) {
      scanType[ii] = '_';
    }
  }

  char outName[BUFSIZ];
  sprintf(outName, "nexrad.%04d%02d%02d_%02d%02d%02d_%s_v%03d_%s.msg31",
          year, month, day, hour, min, sec,
          instrumentName.c_str(), volNum, scanType.c_str());

  return outName;

}
  
////////////////////////////////////////////////////////////
// write the metadata header blocks

int NexradRadxFile::_writeMetaDataHdrs(const RadxVol &vol)

{

  // VOLUME TITLE

  NexradData::vol_title_t title;
  memset(&title, 0, sizeof(title));
  char text[128];
  sprintf(text, "AR2V0002.%.3d", vol.getVolumeNumber()); 
  memcpy(title.filetype, text, 12);

  int julianDate = vol.getEndTimeSecs() / 86400;
  int secsInDay = vol.getEndTimeSecs() - julianDate * 86400;
  double msecsInDay = (secsInDay + vol.getEndNanoSecs() / 1.0e9) * 1.0e3;
  title.julian_date = julianDate + 1;
  title.millisecs_past_midnight = (int) (msecsInDay + 0.5);

  if (_verbose) {
    NexradData::print(title, cerr);
  }
  NexradData::swap(title);

  if (fwrite(&title, sizeof(title), 1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - NexradRadxFile::_writeMetaDataHdrs");
    _addErrStr("  Cannot write title block");
    _addErrStr(strerror(errNum));
    return -1;
  }

  // ADAPTATION DATA HEADER
  if(_adap.beamwidth == 0)
  {
    _adap.antenna_gain = vol.getRadarAntennaGainDbH();
    _adap.beamwidth = vol.getRadarBeamWidthDegH();
    const vector<double> &freqHz = vol.getFrequencyHz();
    if (freqHz.size() > 0) {
      _adap.tfreq_mhz = (int) (freqHz[0] / 1.0e6 + 0.5);
    }
    _adap.deltaprf = _prtIndex + 1;
    
    double latitude = vol.getLatitudeDeg();
    double absLat = fabs(latitude);
    int latDeg = (int) absLat;
    int latMin = (int) ((absLat - latDeg) * 60.0);
    int latSec = (int) ((absLat - latDeg - latMin / 60.0) * 3600.0 + 0.5);
    _adap.slatdeg = latDeg;
    _adap.slatmin = latMin;
    _adap.slatsec = latSec;
    if (latitude > 0) {
      _adap.slatdir[0] = 'N';
    } else {
      _adap.slatdir[0] = 'S';
    }
    
    double longitude = vol.getLongitudeDeg();
    double absLon = fabs(longitude);
    int lonDeg = (int) absLon;
    int lonMin = (int) ((absLon - lonDeg) * 60.0);
    int lonSec = (int) ((absLon - lonDeg - lonMin / 60.0) * 3600.0 + 0.5);
    _adap.slondeg = lonDeg;
    _adap.slonmin = lonMin;
    _adap.slonsec = lonSec;
    if (longitude > 0) {
      _adap.slondir[0] = 'E';
    } else {
      _adap.slondir[0] = 'W';
    }
  }

  if (_verbose) {
    NexradData::print(_adap, cerr);
  }
  NexradData::swap(_adap);

  if (_writeMessage(&_adap, sizeof(_adap),
                    NexradData::RDA_ADAPTATION_DATA,
                    vol.getEndTimeSecs(),
                    (int) vol.getEndNanoSecs())) {
    _addErrStr("ERROR - NexradRadxFile::_writeMetaDataHdrs");
    _addErrStr("  Cannot write RDA adaptation data");
    return -1;
  }

  // VCP HEADER AND PPI SWEEP HEADERS

  RadxBuf vcpBuf;
  
  int nbytesVcp =
    sizeof(_vcp) + vol.getNSweeps() * sizeof(NexradData::ppi_hdr_t);

  if(_vcp.message_len == 0)
  {
    _vcp.message_len = nbytesVcp / 2;
    _vcp.pattern_type = 2;
    _vcp.pattern_number = vol.getScanId();
    _vcp.num_elevation_cuts = vol.getNSweeps();
    _vcp.clutter_map_group = 1;
    if (_maxAbsVel < 64.0) {
      _vcp.dop_vel_resolution = 2;
    } else {
      _vcp.dop_vel_resolution = 4;
    }
    if (_meanPulseWidthUsec < 2) {
      _vcp.pulse_width = 2; // short
    } else {
      _vcp.pulse_width = 4; // long
    }
  }

  if (_verbose) {
    NexradData::print(_vcp, cerr);
  }
  NexradData::swap(_vcp);
  vcpBuf.add(&_vcp, sizeof(_vcp));

  for (size_t isweep = 0; isweep < vol.getNSweeps(); isweep++) {

    const RadxSweep &sweep = *vol.getSweeps()[isweep];

    NexradData::ppi_hdr_t ppi;
    memset(&ppi, 0, sizeof(ppi));

    double fixedElev = sweep.getFixedAngleDeg();
    if (fixedElev < 0) {
      fixedElev += 360.0;
    }
    ppi.elevation_angle = (int) (fixedElev / _angleMult + 0.5);

    // check first ray to see if this is staggered PRT

    const RadxRay &firstRay = *vol.getRays()[sweep.getStartRayIndex()];
    bool isStaggered = false;
    if (firstRay.getPrtMode() == Radx::PRT_MODE_STAGGERED) {
      isStaggered = true;
    }

    if (isStaggered) {
      ppi.waveform_type = 5;
      _setPrtIndexes(firstRay.getPrtSec());
      ppi.doppler_prf_number1 = _prtNum + 1;
      ppi.doppler_prf_pulse_count1 = firstRay.getNSamples() / 2;
      _setPrtIndexes(firstRay.getPrtSec() * firstRay.getPrtRatio());
      ppi.doppler_prf_number2 = _prtNum + 1;
      ppi.doppler_prf_pulse_count2 = firstRay.getNSamples() / 2;
      
    } else {
      ppi.waveform_type = 3;
      _setPrtIndexes(firstRay.getPrtSec());
      ppi.doppler_prf_number1 = _prtNum + 1;
      ppi.doppler_prf_pulse_count1 = firstRay.getNSamples();
    }

    ppi.azimuth_rate =
      (int) (sweep.getTargetScanRateDegPerSec() / _rateMult + 0.5);
    
    NexradData::swap(ppi);
    vcpBuf.add(&ppi, sizeof(ppi));

  } // isweep
  
  if (_writeMessage(vcpBuf.getPtr(), vcpBuf.getLen(),
                    NexradData::VOLUME_COVERAGE_PATTERN,
                    vol.getEndTimeSecs(),
                    (int) vol.getEndNanoSecs())) {
    _addErrStr("ERROR - NexradRadxFile::_writeMetaDataHdrs");
    _addErrStr("  Cannot write vcp headers");
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// write message

int NexradRadxFile::_writeMessage(void *buf,
                                  int nBytesBuf,
                                  int messageType,
                                  time_t timeSecs,
                                  int nanoSecs)
  
{
  
  int maxLen = (NexradData::PACKET_SIZE -
                sizeof(NexradData::ctm_info_t) -
                sizeof(NexradData::msg_hdr_t));
  int nMessages = (nBytesBuf / maxLen) + 1;
  char *ptr = (char *) buf;
  int nBytesLeft = nBytesBuf;
  
  for (int ii = 0; ii < nMessages; ii++) {

    int nBytesToSend = nBytesLeft;
    if (nBytesToSend > maxLen) {
      nBytesToSend = maxLen;
    }
    
    // CTM INFO
    
    NexradData::ctm_info_t ctmInfo;
    memset(&ctmInfo, 0, sizeof(ctmInfo));
    NexradData::swap(ctmInfo);
    if (fwrite(&ctmInfo, sizeof(ctmInfo), 1, _file) != 1) {
      int errNum = errno;
      _addErrStr("ERROR - NexradRadxFile::_writeMessage");
      _addErrStr("  Cannot write ctm info block");
      _addErrStr(strerror(errNum));
      return -1;
    }

    // write message header
    
    NexradData::msg_hdr_t msgHdr;
    memset(&msgHdr, 0, sizeof(msgHdr));
    msgHdr.message_type = messageType;
    msgHdr.message_len = nBytesToSend / 2;
    msgHdr.seq_num = _msgSeqNum;
    _msgSeqNum++;
    int julianDate = (timeSecs / 86400);
    int secsInDay = timeSecs - julianDate * 86400;
    double msecsInDay = (secsInDay + nanoSecs / 1.0e9) * 1.0e3;
    msgHdr.julian_date = julianDate + 1;
    msgHdr.millisecs_past_midnight = (int) (msecsInDay + 0.5);
    msgHdr.num_message_segs = nMessages;
    msgHdr.message_seg_num = ii + 1;

    NexradData::swap(msgHdr);
    
    if (fwrite(&msgHdr, sizeof(msgHdr), 1, _file) != 1) {
      int errNum = errno;
      _addErrStr("ERROR - NexradRadxFile::_writeMessage");
      _addErrStr("  Cannot write message header");
      _addErrStr(strerror(errNum));
      return -1;
    }

    // write data

    if (fwrite(ptr, nBytesToSend, 1, _file) != 1) {
      int errNum = errno;
      _addErrStr("ERROR - NexradRadxFile::_writeMessage");
      _addErrStr("  Cannot write data");
      _addErrStr(strerror(errNum));
      return -1;
    }

    // pad out to even record size as needed

    if (nBytesToSend < maxLen) {
      int nPadding = maxLen - nBytesToSend;
      RadxBuf padBuf;
      char *pad = (char *) padBuf.reserve(nPadding);
      memset(pad, 0, nPadding);
      if (fwrite(pad, nPadding, 1, _file) != 1) {
        int errNum = errno;
        _addErrStr("ERROR - NexradRadxFile::_writeMessage");
        _addErrStr("  Cannot write pad at end of record");
        _addErrStr(strerror(errNum));
        return -1;
      }
    }

    nBytesLeft -= nBytesToSend;
    ptr += nBytesToSend;

  } // ii

  return 0;

}

////////////////////////////////////////////////////////////
// write the ray block

int NexradRadxFile::_writeMsg31(const RadxVol &writeVol,
                                int sweepIndex,
                                const RadxSweep &sweep,
                                int rayIndex,
                                const RadxRay &ray)

{

  // make a copy of the ray so we can modify it as needed

  RadxRay copy(ray);

  int julianDate = (copy.getTimeSecs() / 86400);
  int secsInDay = copy.getTimeSecs() - julianDate * 86400;
  double msecsInDay = (secsInDay + copy.getNanoSecs() / 1.0e9) * 1.0e3;

  // ensure the start range is positive
  // because the message 31 gate1 is an unsigned int

  if (copy.getStartRangeKm() < 0) {
    double startRangeKm = copy.getStartRangeKm();
    double gateSpacingKm = copy.getGateSpacingKm();
    int nGatesMiss = (int) ((-1.0 * startRangeKm) / gateSpacingKm + 1);
    startRangeKm += nGatesMiss * gateSpacingKm;
    copy.remapRangeGeom(startRangeKm, gateSpacingKm);
  }
  
  int offset = 0;

  // msg 31 header

  NexradData::message_31_hdr_t hdr;
  memset(&hdr, 0, sizeof(hdr));

  int nameLen = 4;
  if (writeVol.getInstrumentName().size() < 4) {
    nameLen = writeVol.getInstrumentName().size();
  }
  memcpy(hdr.radar_icao, writeVol.getInstrumentName().c_str(), nameLen);
  hdr.elev_num = copy.getSweepNumber() + 1;
  
  hdr.millisecs_past_midnight = (int) (msecsInDay + 0.5);
  hdr.julian_date = julianDate + 1;

  hdr.radial_num = rayIndex + 1;
  hdr.azimuth = copy.getAzimuthDeg();
  hdr.compression = 0;
  hdr.radial_length = 0;

  if (fabs(copy.getAngleResDeg() - 0.5) < 0.1) {
    hdr.azimuth_spacing = 1;
  } else {
    hdr.azimuth_spacing = 2;
  }

  if (sweepIndex == 0 && rayIndex == 0) {
    hdr.radial_status = 3; // start of vol
  } else if ((sweepIndex == (int) writeVol.getNSweeps() - 1) &&
             (rayIndex == (int) sweep.getNRays() - 1)) {
    hdr.radial_status = 4; // end of vol
  } else if (rayIndex == 0) {
    hdr.radial_status = 0; // start of sweep
  } else if (rayIndex == (int) sweep.getNRays() - 1) {
    hdr.radial_status = 2; // end of sweep
  } else {
    hdr.radial_status = 1; // intermediate radial
  }
    
  hdr.elev_num = sweepIndex + 1;
  hdr.sector_num = 1;
  hdr.elevation = copy.getElevationDeg();
  hdr.spot_blank = 0;

  if (sweep.getRaysAreIndexed()) {
    hdr.azimuth_index = (int) (sweep.getAngleResDeg() * 100.0 + 0.5);
  } else {
    hdr.azimuth_index = 0;
  }

  hdr.n_data_blocks = 0;
  for (int ii = 0; ii < NexradData::MAX_DATA_BLOCKS; ii++) {
    hdr.data_block_offset[ii] = 0;
  }

  offset += sizeof(hdr);

  // msg 31 vol header

  NexradData::message_31_vol_t vol;
  memset(&vol, 0, sizeof(vol));
  char blockText[32];
  strncpy(blockText, "RVOL", 4);
  memcpy(&vol, blockText, 4);
  vol.block_size = sizeof(vol);
  vol.ver_major = 1;
  vol.ver_minor = 0;

  vol.lat = writeVol.getLatitudeDeg();
  vol.lon = writeVol.getLongitudeDeg();

  double sensorHt = 0;
  if (writeVol.getSensorHtAglM() > 0) {
    sensorHt = writeVol.getSensorHtAglM();
  }
  vol.height = (int) ((writeVol.getAltitudeKm() * 1000.0 - sensorHt) + 0.5);
  vol.feedhorn_height = (int) (sensorHt + 0.5);

  double dbz0 = 0;
  double noiseH = 0;
  double noiseV = 0;
  if (writeVol.getNRcalibs() > 0) {
    const RadxRcalib &calib = *writeVol.getRcalibs()[0];
    dbz0 = calib.getBaseDbz1kmHc();
    vol.dbz0 = dbz0;
    vol.system_zdr = calib.getZdrCorrectionDb();
    vol.system_phi = calib.getSystemPhidpDeg();
    noiseH = calib.getNoiseDbmHc();
    noiseV = calib.getNoiseDbmVc();
  }
  if (copy.getMeasXmitPowerDbmH() > -9990) {
    vol.horiz_power = pow(10.0, copy.getMeasXmitPowerDbmH() / 10.0) * 1.0e6;
  }
  if (copy.getMeasXmitPowerDbmV() > -9990) {
    vol.vert_power = pow(10.0, copy.getMeasXmitPowerDbmV() / 10.0) * 1.0e6;
  }
  vol.vol_coverage_pattern = writeVol.getScanId();

  hdr.data_block_offset[hdr.n_data_blocks] = offset;
  hdr.n_data_blocks++;
  offset += sizeof(vol);

  // elev header

  NexradData::message_31_elev_t elev;
  memset(&elev, 0, sizeof(elev));
  strncpy(blockText, "RELV", 4);
  memcpy(&elev, blockText, 4);
  elev.block_size = sizeof(elev);
  elev.dbz0 = dbz0;

  hdr.data_block_offset[hdr.n_data_blocks] = offset;
  hdr.n_data_blocks++;
  offset += sizeof(elev);

  // radial header

  NexradData::message_31_radial_t radial;
  memset(&radial, 0, sizeof(radial));
  strncpy(blockText, "RRAD", 4);
  memcpy(&radial, blockText, 4);
  radial.block_size = sizeof(radial);

  radial.unamb_range_x10 = (int) (copy.getUnambigRangeKm() * 10.0 + 0.5);
  radial.horiz_noise = noiseH;
  radial.vert_noise = noiseV;
  radial.nyquist_vel = (int) (copy.getNyquistMps() * 100.0 + 0.5);
  
  hdr.data_block_offset[hdr.n_data_blocks] = offset;
  hdr.n_data_blocks++;
  offset += sizeof(radial);

  // data fields

  bool haveRef = true;
  RadxBuf refBuf;
  NexradData::message_31_field_t refFhdr;
  if (_loadField(copy, "REF", "REF,DBZ,DBZHC,ZH,Z_HH,DZ",
                 1, 2.0, 66.0, refFhdr, refBuf)) {
    haveRef = false;
  }
  
  bool haveVel = true;
  RadxBuf velBuf;
  NexradData::message_31_field_t velFhdr;
  if (_maxAbsVel < 64.0) {
    if (_loadField(copy, "VEL", "VEL,VR,VELHC,V",
                   1, 2.0, 129.0, velFhdr, velBuf)) {
      haveVel = false;
    }
  } else {
    if (_loadField(copy, "VEL", "VEL,VR,VELHC,V",
                   1, 1.0, 129.0, velFhdr, velBuf)) {
      haveVel = false;
    }
  }
  
  bool haveSw = true;
  RadxBuf swBuf;
  NexradData::message_31_field_t swFhdr;
  if (_loadField(copy, "SW", "WIDTH,SW,SPW",
                 1, 2.0, 129.0, swFhdr, swBuf)) {
    haveSw = false;
  }
  
  bool haveZdr = true;
  RadxBuf zdrBuf;
  NexradData::message_31_field_t zdrFhdr;
  if (_loadField(copy, "ZDR", "ZDR,ZD",
                 1, 16.0, 128.0, zdrFhdr, zdrBuf)) {
    haveZdr = false;
  }
  
  bool havePhi = true;
  RadxBuf phiBuf;
  NexradData::message_31_field_t phiFhdr;
  if (_loadField(copy, "PHI", "PHIDP,PHI,PH",
                 2, 2.8361, 2.0, phiFhdr, phiBuf)) {
    havePhi = false;
  }
  
  bool haveRho = true;
  RadxBuf rhoBuf;
  NexradData::message_31_field_t rhoFhdr;
  if (_loadField(copy, "RHO", "RHOHV,RHO,RH",
                 1, 300.0, -60.0, rhoFhdr, rhoBuf)) {
    haveRho = false;
  }

  // compute lengths and offsets

  int refLen = 0;
  if (haveRef) {
    refLen = sizeof(NexradData::message_31_field_t) + refBuf.getLen();
    hdr.data_block_offset[hdr.n_data_blocks] = offset;
    hdr.n_data_blocks++;
    offset += refLen;
  }

  int velLen = 0;
  if (haveVel) {
    velLen = sizeof(NexradData::message_31_field_t) + velBuf.getLen();
    hdr.data_block_offset[hdr.n_data_blocks] = offset;
    hdr.n_data_blocks++;
    offset += velLen;
  }

  int swLen = 0;
  if (haveSw) {
    swLen = sizeof(NexradData::message_31_field_t) + swBuf.getLen();
    hdr.data_block_offset[hdr.n_data_blocks] = offset;
    hdr.n_data_blocks++;
    offset += swLen;
  }

  int zdrLen = 0;
  if (haveZdr) {
    zdrLen = sizeof(NexradData::message_31_field_t) + zdrBuf.getLen();
    hdr.data_block_offset[hdr.n_data_blocks] = offset;
    hdr.n_data_blocks++;
    offset += zdrLen;
  }

  int phiLen = 0;
  if (havePhi) {
    phiLen = sizeof(NexradData::message_31_field_t) + phiBuf.getLen();
    hdr.data_block_offset[hdr.n_data_blocks] = offset;
    hdr.n_data_blocks++;
    offset += phiLen;
  }

  int rhoLen = 0;
  if (haveRho) {
    rhoLen = sizeof(NexradData::message_31_field_t) + rhoBuf.getLen();
    hdr.data_block_offset[hdr.n_data_blocks] = offset;
    hdr.n_data_blocks++;
    offset += rhoLen;
  }

  hdr.radial_length = offset;

  // load up main message buffer

  RadxBuf msgBuf;

  NexradData::swap(hdr);
  msgBuf.add(&hdr, sizeof(hdr)); // will swap and overwrite later
  
  NexradData::swap(vol);
  msgBuf.add(&vol, sizeof(vol));
  
  NexradData::swap(elev);
  msgBuf.add(&elev, sizeof(elev));
  
  NexradData::swap(radial);
  msgBuf.add(&radial, sizeof(radial));

  if (haveRef) {
    NexradData::swap(refFhdr);
    msgBuf.add(&refFhdr, sizeof(refFhdr));
    msgBuf.add(refBuf.getPtr(), refBuf.getLen());
  }

  if (haveVel) {
    NexradData::swap(velFhdr);
    msgBuf.add(&velFhdr, sizeof(velFhdr));
    msgBuf.add(velBuf.getPtr(), velBuf.getLen());
  }

  if (haveSw) {
    NexradData::swap(swFhdr);
    msgBuf.add(&swFhdr, sizeof(swFhdr));
    msgBuf.add(swBuf.getPtr(), swBuf.getLen());
  }

  if (haveZdr) {
    NexradData::swap(zdrFhdr);
    msgBuf.add(&zdrFhdr, sizeof(zdrFhdr));
    msgBuf.add(zdrBuf.getPtr(), zdrBuf.getLen());
  }

  if (havePhi) {
    NexradData::swap(phiFhdr);
    msgBuf.add(&phiFhdr, sizeof(phiFhdr));
    msgBuf.add(phiBuf.getPtr(), phiBuf.getLen());
  }

  if (haveRho) {
    NexradData::swap(rhoFhdr);
    msgBuf.add(&rhoFhdr, sizeof(rhoFhdr));
    msgBuf.add(rhoBuf.getPtr(), rhoBuf.getLen());
  }
  
  // pad out to minimum size if too small
  
  size_t minLen = (NexradData::PACKET_SIZE -
                   sizeof(NexradData::ctm_info_t) -
                   sizeof(NexradData::msg_hdr_t));

  if (msgBuf.getLen() < minLen) {
    int nPadding = minLen - msgBuf.getLen();
    RadxBuf padBuf;
    char *pad = (char *) padBuf.reserve(nPadding);
    memset(pad, 0, nPadding);
    msgBuf.add(padBuf.getPtr(), nPadding);
  }

    // CTM INFO
    
    NexradData::ctm_info_t ctmInfo;
    memset(&ctmInfo, 0, sizeof(ctmInfo));
    NexradData::swap(ctmInfo);
    if (fwrite(&ctmInfo, sizeof(ctmInfo), 1, _file) != 1) {
      int errNum = errno;
      _addErrStr("ERROR - NexradRadxFile::_writeMsg31");
      _addErrStr("  Cannot write ctm info block");
      _addErrStr(strerror(errNum));
      return -1;
    }

  // write the main message header

  NexradData::msg_hdr_t msgHdr;
  memset(&msgHdr, 0, sizeof(msgHdr));
  msgHdr.message_len = (msgBuf.getLen() + sizeof(NexradData::msg_hdr_t)) / 2;
  msgHdr.message_type = 31;
  msgHdr.seq_num = _msgSeqNum;
  _msgSeqNum++;
  msgHdr.julian_date = julianDate + 1;
  msgHdr.millisecs_past_midnight = (int) (msecsInDay + 0.5);
  msgHdr.num_message_segs = 1;
  msgHdr.message_seg_num = 1;
  NexradData::swap(msgHdr);

  if (fwrite(&msgHdr, sizeof(msgHdr), 1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - NexradRadxFile::_writeMsg31");
    _addErrStr("  Cannot write message header");
    _addErrStr(strerror(errNum));
    return -1;
  }
  
  // write the message
  
  if (fwrite(msgBuf.getPtr(), msgBuf.getLen(), 1, _file) != 1) {
    int errNum = errno;
    _addErrStr("ERROR - NexradRadxFile::_writeMsg31");
    _addErrStr("  Cannot write message buffer");
    _addErrStr(strerror(errNum));
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////////////////
// Load a data field into the buffer provided.
// If data field not found, the buffer is loaded with missing values.
// Returns 0 on success, -1 if data field not found.

int NexradRadxFile::_loadField(const RadxRay &ray,
                               const string &name,
                               const string &names,
                               int byteWidth,
                               double scale,
                               double offset,
                               NexradData::message_31_field_t &fhdr,
                               RadxBuf &buf)
                               
{

  buf.clear();
  int nGates = ray.getNGates();

  // fill out field header

  memset(&fhdr, 0, sizeof(fhdr));
  fhdr.block_type = 'D';
  memcpy(fhdr.block_name, name.c_str(), name.size());
  fhdr.num_gates = nGates;
  fhdr.gate1 = (int) (ray.getStartRangeKm() * 1000.0 + 0.5);
  fhdr.gate_width = (int) (ray.getGateSpacingKm() * 1000.0 + 0.5);
  fhdr.data_size = byteWidth * 8;
  fhdr.scale = scale;
  fhdr.offset = offset;

  // tokenize the field list

  vector<string> toks;
  RadxStr::tokenize(names, ",", toks);

  // search for the field in the ray which matches the earliest token
  // in the list exactly

  const RadxField *field = NULL;
  for (size_t jj = 0; jj < toks.size(); jj++) {
    for (size_t ii = 0; ii < ray.getNFields(); ii++) {
      const RadxField *fld = ray.getFields()[ii];
      if (fld->getName() == toks[jj]) {
        field = fld;
        break;
      }
    } // ii
  } // jj

  // if no exact match found, search for partial match

  for (size_t jj = 0; jj < toks.size(); jj++) {
    if (field != NULL) {
      break;
    }
    for (size_t ii = 0; ii < ray.getNFields(); ii++) {
      const RadxField *fld = ray.getFields()[ii];
      string partName(fld->getName().substr(0, toks[jj].size()));
      if (partName == toks[jj]) {
        field = fld;
        break;
      }
    } // ii
  } // jj
    
  // check for data field found

  if (field == NULL) {

    // no suitable found, fill with missing

    if (byteWidth == 2) {
      Radx::ui16 *data = (Radx::ui16 *) buf.reserve(nGates * sizeof(Radx::ui16));
      for (int ii = 0; ii < nGates; ii++) {
        data[ii] = 0;
      }
    } else {
      // byte width 1
      Radx::ui08 *data = (Radx::ui08 *) buf.reserve(nGates * sizeof(Radx::ui08));
      for (int ii = 0; ii < nGates; ii++) {
        data[ii] = 0;
      }
    }

    return -1;

  }

  // copy the incoming field, convert to floats

  RadxField copy(*field);
  copy.convertToFl32();
  Radx::fl32 *fdata = (Radx::fl32 *) copy.getData();
  Radx::fl32 miss = copy.getMissingFl32();

  // load up data field

  if (byteWidth == 2) {

    // byte width 2

    Radx::ui16 *data = (Radx::ui16 *) buf.reserve(nGates * sizeof(Radx::ui16));
    for (int ii = 0; ii < nGates; ii++) {
      if (fdata[ii] == miss) {
        data[ii] = 0;
      } else {
        int ival = (int) ((fdata[ii] * scale) + offset + 0.5);
        if (ival < 1) {
          ival = 1;
        } else if (ival > 65535) {
          ival = 65535;
        }
        data[ii] = ival;
      }
    }
    ByteOrder::swap16(data, nGates * sizeof(Radx::ui16));

  } else {

    // byte width 1

    Radx::ui08 *data = (Radx::ui08 *) buf.reserve(nGates * sizeof(Radx::ui08));
    for (int ii = 0; ii < nGates; ii++) {
      if (fdata[ii] == miss) {
        data[ii] = 0;
      } else {
        int ival = (int) ((fdata[ii] * scale) + offset + 0.5);
        if (ival < 1) {
          ival = 1;
        } else if (ival > 255) {
          ival = 255;
        }
        data[ii] = ival;
      }
    }

  }

  return 0;

}

///////////////////////////////////////////////////////////
// determine the max absolute velocity, so we can scale
// velocity correctly

void NexradRadxFile::_computeMaxAbsVel(const RadxVol &vol)

{

  _maxAbsVel = 0;

  for (size_t iray = 0; iray < vol.getNRays(); iray++) {

    const RadxRay &ray = *vol.getRays()[iray];

    for (size_t ifield = 0; ifield < ray.getNFields(); ifield++) {

      const RadxField &field = *ray.getFields()[ifield];

      if (field.getName()[0] != 'V') {
        // not a velocity field
        continue;
      }

      // compute min and max

      field.computeMinAndMax();
      double absMin = fabs(field.getMinValue());
      if (absMin > _maxAbsVel) {
        _maxAbsVel = absMin;
      }
      double absMax = fabs(field.getMaxValue());
      if (absMax > _maxAbsVel) {
        _maxAbsVel = absMax;
      }

    } // ifield

  } // iray
  
}

///////////////////////////////////////////////////////////
// determine the mean pulse width

void NexradRadxFile::_computeMeanPulseWidth(const RadxVol &vol)

{

  double sum = 0.0;
  double count = 0.0;

  for (size_t iray = 0; iray < vol.getNRays(); iray++) {
    const RadxRay &ray = *vol.getRays()[iray];
    sum += ray.getPulseWidthUsec();
    count++;
  } // iray

  if (count > 0) {
    _meanPulseWidthUsec = sum / count;
  } else {
    _meanPulseWidthUsec = 0;
  }
  
}

////////////////////////////////////////////////
// set find the prt indexes for a given prt

void NexradRadxFile::_setPrtIndexes(double prtSec)

{

  double minDiff = 1.0e99;
  for (int ii = 0; ii < 5; ii++) {
    for (int jj = 0; jj < 8; jj++) {
      double prt = _prtTable[ii][jj] / 1.0e6;
      double diff = fabs(prtSec - prt);
      if (diff < minDiff) {
        _prtIndex = ii;
        _prtNum = jj;
        minDiff = diff;
      }
    } // jj
  } // ii

}

////////////////////////////////////////////////
// unzip an LDM-based zipped file
// returns 0 on success, -1 on failure

int NexradRadxFile::_unzipFile(const string &path)

{

  if (_debug) {
    cerr << "Unzipping file: " << path << endl;
  }

  // clean up any existing tmp files

  _removeTmpFiles();

  // open input file

  FILE *in = fopen(path.c_str(), "r");
  if (in == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - NexradRadxFile::readFromPath");
    _addErrStr("  Cannot open zipped file");
    _addErrStr("  Path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  // read in header
  
  char header[24];
  if (fread(header, 1, 24, in) != 24) {
    _addErrStr("ERROR - NexradRadxFile::readFromPath");
    _addErrStr("  Cannot read 24-byte header");
    _addErrStr("  Path: ", path);
    fclose(in);
    return -1;
  }

  if (strncmp(header, "ARCH", 4) &&
      strncmp(header, "AR2V", 4)) {
    _addErrStr("ERROR - NexradRadxFile::readFromPath");
    _addErrStr("  Not a NEXRAD file");
    _addErrStr("  Path: ", path);
    fclose(in);
    return -1;
  }

  // store output in buffer for later writing to tmp file

  RadxBuf uncomp;
  uncomp.add(header, 24);

  bool lastBlock = false;
  while (!feof(in)) {

    // read in sub-buffer length
    
    int32_t length;
    if (fread(&length, 4, 1, in) != 1) {
      if (feof(in)) {
        break;
      }
      _addErrStr("ERROR - NexradRadxFile::readFromPath");
      _addErrStr("  Cannot unzip file");
      _addErrStr("  Path: ", path);
    }
    length = ntohl(length);

    if(length < 0) {
      // a negative length indicates this is the last block
      length = -length;
      lastBlock = true;
    }

    // read in sub-buffer
    
    RadxArray<char> inBuf_;
    char *inBuf = inBuf_.alloc(length);
    if ((int) fread(inBuf, 1, length, in) != length) {
      _addErrStr("ERROR - NexradRadxFile::readFromPath");
      _addErrStr("  Cannot read zipped file");
      _addErrStr("  Path: ", path);
      fclose(in);
      return -1;
    }
    
    if (length <= 10) {
      continue;
    }

    // unzip

    unsigned int outSize = length * 40;
    RadxArray<char> outBuf_;
    char *outBuf = outBuf_.alloc(outSize);
    unsigned int outLen;
    for (int itry = 0; itry < 10; itry++) {
      outLen = outSize;
      int iret = BZ2_bzBuffToBuffDecompress(outBuf, &outLen,
                                            inBuf, length,
                                            0, 0);
      if (iret == BZ_OK) {
        break;
      } else if (iret == BZ_OUTBUFF_FULL) {
        outSize *= 2;
        outBuf = outBuf_.alloc(outSize);
        continue;
      } else {
        _addErrStr("ERROR - NexradRadxFile::readFromPath");
        _addErrStr("  Path: ", path);
        _addErrInt("  BZIP unzip error: ", iret);
        fclose(in);
        return -1;
      }
    } // itry
    
    // add to uncomp buffer
    
    uncomp.add(outBuf, outLen);

    if (lastBlock) {
      break;
    }

  } // while

  // success

  fclose(in);
    
  // compute temporary path
  
  RadxPath rpath(path);
  string tmpPath("/tmp/");
  char tmpName[128];
  long long now = time(NULL);
  long long pid = getpid();
  sprintf(tmpName, "NexradRadxFile.%s.%lld.%lld", rpath.getFile().c_str(), now, pid);
  tmpPath += tmpName;

  // open output file
  
  FILE *out = fopen(tmpPath.c_str(), "w");
  if (out == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - NexradRadxFile::readFromPath");
    _addErrStr("  Cannot open tmp file for unzipping");
    _addErrStr("  Path: ", tmpPath);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }

  // write the uncomp buffer to the tmp file
  
  if (fwrite(uncomp.getPtr(), uncomp.getLen(), 1, out) != 1) {
    _addErrStr("ERROR - NexradRadxFile::readFromPath");
    _addErrStr("  Cannot write uncompressed data to tmp file");
    _addErrStr("  Path: ", tmpPath);
    fclose(in);
    return -1;
  }
  
  fclose(out);
  _tmpPath = tmpPath;
  return 0;

}

///////////////////////////////////////////////////////////
// clean up any existing tmp files created by
// unzipping - see _unzipFile
//
// this is only required if the app exits before the tmpe
// file is removed
//
// only remove files older than 2 mins

void NexradRadxFile::_removeTmpFiles()

{

  RadxReadDir rdir;
  if (rdir.open("/tmp") == 0) {
    
    // Loop thru directory looking for the tmp files
    
    struct dirent *dp;
    for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
      
      // only include files that start with NexradRadxFile

      string name(dp->d_name);
      if (name.find("NexradRadxFile") != 0) {
        continue;
      }

      // Get the file status so we can check the modify time
      
      struct stat fstat;
      string path("/tmp/");
      path += name;
      if (!RadxPath::doStat(path.c_str(), fstat) == 0) {

        // compute file age

        time_t now = time(NULL);
        double age = (double) now - fstat.st_mtime;

        // remove if older than 1 minute

        if (age > 120) {
          unlink(path.c_str());
        }
            
      }

    } // dp
    rdir.close();

  } // if (rdir ...

}

///////////////////////////////////////////////////////////
// load up signed data, interpolating from 1000m gates to
// 250m gates as required

void NexradRadxFile::_loadSignedData(const vector<Radx::ui08> &udata,
                                     vector<Radx::si08> &sdata,
                                     bool interp)

{

  for (size_t ii = 0; ii < udata.size(); ii++) {

    Radx::ui08 uval = udata[ii];
    Radx::si08 sval;
    if (uval < 2) {
      sval = Radx::missingSi08;
    } else {
      int ival = uval - 128;
      sval = ival;
    }

    if (!interp) {
      sdata.push_back(sval);
    } else {
      // repeat 1000m gates by 4 to load up 250m gates
      for (int jj = 0; jj < 4; jj++) {
        sdata.push_back(sval);
      }
    }

  } // ii

  if (interp) {
    _interp1kmGates(sdata.size(), sdata.data());
  }

}

void NexradRadxFile::_loadSignedData(const vector<Radx::ui16> &udata,
                                     vector<Radx::si16> &sdata,
                                     bool interp)

{
  
  for (size_t ii = 0; ii < udata.size(); ii++) {

    Radx::ui16 uval = udata[ii];
    Radx::si16 sval;
    if (uval < 2) {
      sval = Radx::missingSi16;
    } else {
      int ival = uval - 32768;
      sval = ival;
    }

    if (!interp) {
      sdata.push_back(sval);
    } else {
      // repeat 1000m gates by 4 to load up 250m gates
      for (int jj = 0; jj < 4; jj++) {
        sdata.push_back(sval);
      }
    }

  } // ii

  if (interp) {
    _interp1kmGates(sdata.size(), sdata.data());
  }

}

////////////////////////////////////////////////
// interpolate 1km to 250m gates

void NexradRadxFile::_interp1kmGates(int nGates,
                                     Radx::si08 *idata)

{
  
  for (int ii = 2; ii < (nGates - 4); ii += 4) {
    
    int startIval = idata[ii];
    int endIval = idata[ii+4];

    if (startIval == Radx::missingSi08 || 
        endIval == Radx::missingSi08) {
      // one of the end points is missing, so don't interpolate
      continue;
    }

    double startVal = startIval;
    double endVal = endIval;
    double deltaVal = (endVal - startVal) / 4.0;
    double val = startVal + deltaVal / 2.0;
    
    for (int jj = 0; jj < 4; jj++, val += deltaVal) {
      int ival = (int) floor(val + 0.5);
      if (ival < -128) {
        ival = -128;
      } else if (ival > 127) {
        ival = 127;
      }
      int kk = ii + jj;
      idata[kk] = ival;
    } // jj

  } // ii

}

void NexradRadxFile::_interp1kmGates(int nGates,
                                     Radx::si16 *idata)

{

  for (int ii = 2; ii < (nGates - 4); ii += 4) {
    
    int startIval = idata[ii];
    int endIval = idata[ii+4];

    if (startIval == Radx::missingSi16 || 
        endIval == Radx::missingSi16) {
      // one of the end points is missing, so don't interpolate
      continue;
    }

    double startVal = startIval;
    double endVal = endIval;
    double deltaVal = (endVal - startVal) / 4.0;
    double val = startVal + deltaVal / 2.0;

    for (int jj = 0; jj < 4; jj++, val += deltaVal) {
      int ival = (int) floor(val + 0.5);
      if (ival < -32768) {
        ival = -32768;
      } else if (ival > 32767) {
        ival = 32767;
      }
      int kk = ii + jj;
      idata[kk] = ival;
    } // jj

  } // ii

}


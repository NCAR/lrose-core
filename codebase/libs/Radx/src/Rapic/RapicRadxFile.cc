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
// RapicRadxFile.cc
//
// RapicRadxFile object
//
// Support for radial data in RAPIC format
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2013
//
///////////////////////////////////////////////////////////////

#include <Radx/RapicRadxFile.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxRcalib.hh>
#include <cmath>
#include "PPIField.hh"
#include "RapicRay.hh"
#include "Linebuff.hh"
#include "ScanParams.hh"
#include "sRadl.hh"
using namespace std;

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MEM_zero(a) memset(&(a), 0, sizeof((a)))

int RapicRadxFile::_volumeNumber = 0;
 
//////////////
// Constructor

RapicRadxFile::RapicRadxFile() : RadxFile()
                                   
{

  _readVol = NULL;
  _scanParams = new ScanParams;
  _file = NULL;
  clear();

}

/////////////
// destructor

RapicRadxFile::~RapicRadxFile()
{
  if (_scanParams) {
    delete _scanParams;
  }
  clear();
}

/////////////////////////////////////////////////////////
// Check if specified file is FORAY NC format
// Returns true if supported, false otherwise

bool RapicRadxFile::isSupported(const string &path)

{
  
  if (isRapic(path)) {
    return true;
  }

  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a RAPIC RAW file
// Returns true on success, false on failure

bool RapicRadxFile::isRapic(const string &path)
  
{

  clear();
  
  // open file
  
  FILE *fp;
  if ((fp = fopen(path.c_str(), "r")) == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - RapicRadxFile::isRapic()");
    _addErrStr("  Cannot open file: ", path);
    _addErrStr("  ", strerror(errNum));
    return false;
  }

  // read in first few lines, check for RAPIC-type file

  char line[256];
  if (fgets(line, 256, fp) == NULL) {
    fclose(fp);
    return false;
  } 

  // look for '/IMAGE:'
  
  if (strncmp(line, "/IMAGE:", 7)) {
    // is not RAPIC file
    fclose(fp);
    return false;
  }
  
  // look for '/IMAGEHEADER END:'

  while (!feof(fp)) {
    if (fgets(line, 256, fp) == NULL) {
      fclose(fp);
      return false;
    }
    if (strncmp(line, "/IMAGEHEADER END:", 17) == 0) {
      // is RAPIC file
      fclose(fp);
      return true;
    }
  }
  
  fclose(fp);

  return false;

}

/////////////////////////////////////////////////////////
// clear the data in the object

void RapicRadxFile::clear()
  
{

  clearErrStr();

  _startTimeSecs = 0;
  _endTimeSecs = 0;
  _startNanoSecs = 0;
  _endNanoSecs = 0;

  _dualPol = false;

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

int RapicRadxFile::writeToDir(const RadxVol &vol,
                              const string &dir,
                              bool addDaySubDir,
                              bool addYearSubDir)
  
{

  // Writing RAPIC files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - RapicRadxFile::writeToDir" << endl;
  cerr << "  Writing RAPIC format files not supported" << endl;
  cerr << "  Will write CfRadial file instead" << endl;

  // set up NcfRadxFile object

  NcfRadxFile ncfFile;
  ncfFile.copyWriteDirectives(*this);

  // perform write

  int iret = ncfFile.writeToDir(vol, dir, addDaySubDir, addYearSubDir);

  // set return values

  _errStr = ncfFile.getErrStr();
  _dirInUse = ncfFile.getDirInUse();
  _pathInUse = ncfFile.getPathInUse();
  vol.setPathInUse(_pathInUse);

  return iret;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified path
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int RapicRadxFile::writeToPath(const RadxVol &vol,
                                const string &path)
  
{

  // Writing RAPIC files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - RapicRadxFile::writeToPath" << endl;
  cerr << "  Writing RAPIC format files not supported" << endl;
  cerr << "  Will write CfRadial file instead" << endl;

  // set up NcfRadxFile object

  NcfRadxFile ncfFile;
  ncfFile.copyWriteDirectives(*this);

  // perform write

  int iret = ncfFile.writeToPath(vol, path);

  // set return values

  _errStr = ncfFile.getErrStr();
  _pathInUse = ncfFile.getPathInUse();
  vol.setPathInUse(_pathInUse);

  return iret;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int RapicRadxFile::readFromPath(const string &path,
                                RadxVol &vol)
  
{

  _initForRead(path, vol);

  if (_debug) {
    cerr << "Processing file: " << path << endl;
    _scanParams->setDebug(_debug);
  }

  // set up the input line buffer

  Linebuff lineBuf;
  
  if (lineBuf.openFile(path.c_str())) {
    _addErrStr("ERROR - Rapic2Dsr::readFromPath");
    _addErrStr("  Cannot open file: ", path);
    return -1;
  }

  // loop through all images in the file

  int imageNum = 0;
  bool endFound = false;
  while (!lineBuf.endOfFile()) {
    
    // find start of image
    
    if (_findImageStart(lineBuf)) {
      lineBuf.closeFile();
      return -1;
    }
    
    if (_debug) {
      cerr << ">>>>>> start of image found <<<<<<" << endl;
    }
    
    // process the image
    
    if (_processImage(path.c_str(), lineBuf, imageNum)) {
      continue;
    }
    imageNum++;

    // find end of image
    
    if (_findImageEnd(lineBuf)) {
      lineBuf.closeFile();
      return -1;
    } else {
      if (_debug) {
        cerr << ">>>>>> end of image found <<<<<<" << endl;
      }
      endFound = true;
      break;
    }

  } // while
  
  // close input file
  
  lineBuf.closeFile();

  if (!endFound) {
  }

  // set volume number - increments per file

  _volumeNumber++;

  if (_readVol->getNRays() == 0) {
    _addErrStr("ERROR - RapicRadxFile::readFromPath");
    _addErrStr("  No rays found, file: ", _pathInUse);
    return -1;
  }
  
  // load up data in read volume

  if (_finalizeReadVolume()) {
    return -1;
  }

  if (_debug) {
    _readVol->print(cerr);
  }
  
  // set the packing from the rays

  _readVol->setPackingFromRays();

  // add to paths used on read
  
  _readPaths.push_back(path);

  // set internal format

  _fileFormat = FILE_FORMAT_FORAY_NC;

  return 0;

}

/////////////////////////////////////////////////////////////
// finalize the read volume

int RapicRadxFile::_finalizeReadVolume()
  
{

  // set the meta data on the volume

  _setVolMetaData();

  // interp times so they are smoothly increasing

  _readVol->interpRayTimes();
  
  // remove rays with all missing data, if requested

  if (_readRemoveRaysAllMissing) {
    _readVol->removeRaysWithDataAllMissing();
  }

  // check for indexed rays, set info on rays
  
  _readVol->checkForIndexedRays();
  
  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();
  
  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - RapicRadxFile::_finalizeReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                      _readStrictAngleLimits)) {
      _addErrStr("ERROR - RapicRadxFile::_finalizeReadVolume");
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
      return -1;
    }
  }

  // load the volume information from the rays

  _readVol->loadVolumeInfoFromRays();

  return 0;
  
}

/////////////////////////////////////////////////////////
// print summary after read

void RapicRadxFile::print(ostream &out) const
  
{
  
  out << "=============== RapicRadxFile ===============" << endl;
  RadxFile::print(out);
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in rapic file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int RapicRadxFile::printNative(const string &path, ostream &out,
                               bool printRays, bool printData)
  
{
  
  clear();
  RadxVol vol;
  _readVol = &vol;
  _readVol->clear();
  _pathInUse = path;
  _readPaths.clear();

  int iret = 0;

#ifdef JUNK

  // open file

  if (_openRead(path)) {
    _addErrStr("ERROR - RapicRadxFile::printNative");
    return -1;
  }

  // read in product and ingest headers
  
  if (_printHeaders(out)) {
    _addErrStr("ERROR - RapicRadxFile::printNative");
    _addErrStr("  Reading header, file: ", _pathInUse);
    return -1;
  }

  if (!printRays) {
    return 0;
  }
  
  // read the data, a sweep at a time
  
  while (!feof(_file)) {

    if (_printSweepHeaders(out) == 0) {
      if (_nFieldsRead > 0 && printData) {
        if (_printSweepData(out)) {
          _addErrStr("ERROR - RapicRadxFile::printNative");
          _addErrStr("  Processing sweep, file: ", _pathInUse);
          iret = -1;
        }
      }
    } else {
      iret = -1;
    }

  } // while
  
  // finalize the read volume
  
  if (_debug) {
    _finalizeReadVolume();
    _readVol->print(cerr);
  }

#endif
  
  return iret;

}

///////////////////////////
// set volume meta data

void RapicRadxFile::_setVolMetaData()

{

  _readVol->setOrigFormat("RAPIC");

  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setStartTime(_startTimeSecs, _startNanoSecs);
  _readVol->setEndTime(_endTimeSecs, _endNanoSecs);
  
  _readVol->setInstrumentName(_scanParams->radar_name);

  char stationId[128];
  sprintf(stationId, "%d", _scanParams->station_id);
  _readVol->setSiteName(stationId);

  _readVol->setScanName("UNKNOWN");
  _readVol->setScanId(0);
  _readVol->setTargetScanRateDegPerSec(_scanParams->angle_rate);
  
  _readVol->setInstrumentType(Radx::INSTRUMENT_TYPE_RADAR);
  _readVol->setPlatformType(Radx::PLATFORM_TYPE_FIXED);
  _readVol->setPrimaryAxis(Radx::PRIMARY_AXIS_Z);
  
  _readVol->setTitle("RAPIC VOLUME FROM AUSTRALIA BUREAU OF METEOROLOGY");
  _readVol->setSource("BOM RAPIC system");
  _readVol->setReferences("Conversion software: Radx::RapicRadxFile");
  
  _readVol->setHistory("Rapic RAW file: ");

  if (_readChangeLatitudeSign) {
    _readVol->setLatitudeDeg(_scanParams->lat);
  } else {
    _readVol->setLatitudeDeg(_scanParams->lat * -1.0);
  }
  _readVol->setLongitudeDeg(_scanParams->lon);

  _readVol->setAltitudeKm(_scanParams->ht / 1000.0);
  // _readVol->setSensorHtAglM(0);

  _readVol->setFrequencyHz(_scanParams->freq_mhz * 1.0e6);

  _readVol->setRadarBeamWidthDegH(_scanParams->hbeamwidth);
  _readVol->setRadarBeamWidthDegV(_scanParams->vbeamwidth);
  
  RadxRcalib *cal = new RadxRcalib;
  cal->setXmitPowerDbmH(_scanParams->peakpowerh);
  cal->setXmitPowerDbmV(_scanParams->peakpowerv);
  cal->setNoiseDbmHc(_scanParams->rxnoise_h);
  cal->setNoiseDbmVc(_scanParams->rxnoise_v);
  cal->setReceiverGainDbHc(_scanParams->rxgain_h);
  cal->setReceiverGainDbVc(_scanParams->rxgain_v);
  _readVol->addCalib(cal);

}

//////////////////////////////////////////////////
// find start of image
//
// Return 0 on success, -1 on failure

int RapicRadxFile::_findImageStart(Linebuff &lineBuf)

{
  const char *searchStr = "/IMAGE: ";
  int searchLen = strlen(searchStr);
  while (lineBuf.readNext() == 0) {
    if (!strncmp(lineBuf.line_buff, searchStr, searchLen)) {
      return 0;
    }
    lineBuf.reset();
  }
  return -1;
}

//////////////////////////////////////////////////
// find end of image
//
// Return 0 on success, -1 on failure

int RapicRadxFile::_findImageEnd(Linebuff &lineBuf)

{
  const char *searchStr = "/IMAGEEND: ";
  int searchLen = strlen(searchStr);
  while (lineBuf.readNext() == 0) {
    if (!strncmp(lineBuf.line_buff, searchStr, searchLen)) {
      // rjp 4 Sep 2001 need to reset lineBuf or next scan is missed.
      lineBuf.reset();
      return 0;
    }
    lineBuf.reset();
  }
  return -1;
}

//////////////////////////////////////////////////
// process the next image

int RapicRadxFile::_processImage(const char *path, 
                                 Linebuff &lineBuf,
                                 int imageNum)

{

  // read the scan list in volume header 

  _clearScanList();
  lineBuf.reset();
  while (!_scanListComplete && lineBuf.readNext() == 0) {
    if (_addToScanList(lineBuf)) {
      _addErrStr("ERROR - RapicRadxFile::_processImage");
      _addErrStr("  Cannot load scan list");
      _addErrStr("    File path: ", path);
      _addErrInt("    Image num: ", imageNum);
      return -1;
    }
    lineBuf.reset();
  }

  if (_scanList.size() < 1) {
    _addErrStr("ERROR - RapicRadxFile::_processImage");
    _addErrStr("  File has no scans: ", path);
    return -1;
  }
  
  // rjp 3 Sep 2001 - CompPPI scan with only one PPI
  if (_scanList.size() == 1 && _nScansFull == 1) {
    if (_debug) {
      _addErrStr("WARNING - RapicRadxFile::_processImage");
      _addErrStr("  Scan is CompPPI: ", path);
      _addErrStr("  Ignoring this file");
    }
    return -1;
  } 

  // get the elev list and n fields

  double prevElev = -9999;
  int nFieldsElev = 0;
  int nFieldsMax = 0;

  for (size_t ii = 0; ii < _scanList.size(); ii++) {
    double elev = _scanList[ii].elev_angle;
    if (fabs(elev - prevElev) > 0.00001) {
      _elevList.push_back(elev);
      prevElev = elev;
      nFieldsMax = MAX(nFieldsMax, nFieldsElev);
      nFieldsElev = 1;
    } else {
      nFieldsElev++;
    }
  } // ii
  nFieldsMax = MAX(nFieldsMax, nFieldsElev);
  _nFields = nFieldsMax;
  
  if (_nFields < 1) {
    _addErrStr("ERROR - RapicRadxFile::_processImage");
    _addErrStr("  File has no fields: ", path);
    return -1;
  }
  
  // compute the number of PPI's

  int nPPIs = _scanList.size() / _nFields;
  if (_debug) {
    cerr << "nPPIs: " << nPPIs << endl;
  }

  // load scan params from scan header
  
  if (_loadScanParams(lineBuf)) {
    _addErrStr("ERROR - RapicRadxFile::_processImage");
    _addErrStr("  Cannot load scan params");
    return -1;
  }
  
  // loop through the PPIs

  for (int sweepNum = 0; sweepNum < nPPIs; sweepNum++) {

    // load up data into PPI fields
    
    if (_loadPpi(sweepNum, lineBuf)) {
      _addErrStr("ERROR - RapicRadxFile::_processImage");
      _addErrInt("  Failed on PPI number: ", sweepNum);
      return -1;
    }

    // add the rays for the ppi

    _addRaysPpi(sweepNum);

  }

  // for a single PPI, compute the rays times from the scan rate

  if (nPPIs == 1) {
    double scanRate = _scanParams->angle_rate;
    vector<RadxRay *> rays = _readVol->getRays();
    double prevAz = rays[0]->getAzimuthDeg();
    RadxTime startRayTime = rays[0]->getRadxTime();
    double sumSecs = 0.0;
    for (size_t iray = 1; iray < rays.size(); iray++) {
      double az = rays[iray]->getAzimuthDeg();
      double deltaAz = fabs(az - prevAz);
      if (deltaAz > 180.0) {
        deltaAz = fabs(deltaAz - 360.0);
      }
      double deltaSecs = deltaAz / scanRate;
      sumSecs += deltaSecs;
      RadxTime rayTime = startRayTime + sumSecs;
      rays[iray]->setTime(rayTime);
      prevAz = az;
    } // iray
  }

  return 0;

}


//////////////////////////////////////////////////
// process PPI

int RapicRadxFile::_loadPpi(int sweepNum, 
                            Linebuff &lineBuf)
  
{

  // Due to uncertainty about integrity of the radar volume there is need 
  // to add a test to check that station_id for each PPI scan matches what is 
  // defined in volume header. (rjp 21/5/2008)

  _clearPpiFields();

  // read in the PPI scan
  
  sRadl radial;
  _maxGatesPpi = 0;
  _maxRaysPpi = 0;

  if (_debug) {
    cerr << "==========================" << endl;
    cerr << "-->> Starting ppi num: " << sweepNum << endl;
  }

  // read in all fields

  for (int ifield = 0; ifield < _nFields; ifield++) {
    
    int scan_index = sweepNum * _nFields + ifield ;
    int scan_num = _scanList[scan_index].scan_num;
    double targetElev = _scanList[scan_index].elev_angle;
    
    PPIField *ppiField = new PPIField();
    
    while (lineBuf.readNext() == 0) {
      
      if (lineBuf.IsEndOfRadarImage()) {
        
	if (_checkScanParams()) {
	  delete ppiField;
	  return -1;
	}
        
	ppiField->setName(_scanParams->field_name, *_scanParams);
	ppiField->setTime(_scanParams->time);
	// To give index from 0 instead of 1. 
	ppiField->setScanNum(_scanParams->scan_num - 1);
	if (_scanParams->scan_num != scan_num) {
	  cerr << "WARNING - scanNum mismatch" << endl;
	  cerr << "  Header scanNum: " << _scanParams->scan_num << endl;
	  cerr << "  Calculated scanNum: " << scan_num << endl;
	}
	ppiField->setRangeRes(_scanParams->range_res);
	ppiField->setStartRange(_scanParams->start_range);

        if (ppiField->getName() == "ZDR" ||
            ppiField->getName() == "RHOHV" ||
            ppiField->getName() == "PHIDP") {
          _dualPol = true;
        }

	lineBuf.reset();
	break;
	
      } // if (lineBuf.IsEndOfRadarImage())
      
      if (lineBuf.IsBinRadl() || lineBuf.IsRadl()) {

	bool isBinary;
	if (_decodeRadial(lineBuf, radial, 
                          isBinary, _scanParams->video_res) == 0) {
	  ppiField->addRay(&radial, *_scanParams, isBinary, targetElev);
	}
	
      } else {
	
	// scan params info
	
	if (_verbose) {
	  cerr << "Params info >>> " << lineBuf.line_buff << endl;
	}
	if (_scanParams->set(lineBuf.line_buff, _nScansFull)) {
	  _addErrStr("ERROR - RapicRadxFile::_loadPPI");
	  _addErrStr("  Cannot load scan params");
	  return -1;
	}
	
      } // if (lineBuf.IsBinRadl() || lineBuf.IsRadl()) 
      
      lineBuf.reset();

    } // while (lineBuf ...
    
    _maxRaysPpi = MAX(_maxRaysPpi, ((int) ppiField->getRays().size()));
    _maxGatesPpi = MAX(_maxGatesPpi, ppiField->getMaxGates());
    
    //rjp 19/8/2006 test for scan with 0 beams
    if (_maxRaysPpi == 0) {
      if (_debug) {
	cerr << "WARNING - RapicRadxFile::_loadPPI" << endl;
	cerr << " Scan has 0 beams" << endl;
	cerr << " Scan will not be written to fmq" << endl;
      }
    }
 
    //rjp 5 Jul 2006 for scan with all null radials set _maxGatesPpi > 0 to 
    // ensure scan is sent to fmq
    if (_maxGatesPpi == 0) {
      _maxGatesPpi=4;
      if (_debug) {
	cerr << "WARNING - RapicRadxFile::_loadPPI" << endl;
	cerr << "  Scan has null radials only" << endl;
	cerr << "  set _maxGatesPpi > 0 to ensure scan sent to fmq" << endl; 
      }
    }

    if (_debug) {
      // cerr << "---->> scanNum: " << ppiField->scanNum << endl;
      // rjp set idx for scan num same as data file. 
      cerr << "---->> scanNum: " << _scanParams->scan_num << endl;
      cerr << "       time: " << RadxTime::str(ppiField->getTime()) << endl;
      cerr << "       fieldNum: " << ifield << endl;
      cerr << "       field name: " << ppiField->getName() << endl;
      cerr << "       elev angle: " << _scanParams->elev_angle << endl;  // rjp 11 Sep 2001
      cerr << "          ppi.nBeams: " << ppiField->getRays().size() << endl;
      cerr << "          ppi.maxGates: " << ppiField->getMaxGates() << endl;
      cerr << "          ppiField->getRangeRes(): " << ppiField->getRangeRes() << endl;
      cerr << "          ppiField->getStartRange(): " << ppiField->getStartRange() << endl;
    }
    
    if (_verbose) {
      cerr << "=====================================" << endl;
      ppiField->printFull(cerr);
    }

    // add field to vector
    
    _ppiFields.push_back(ppiField);
    
  } // ifield
  
  if (_debug) {
    cerr << "-------->> _nFields: " << _nFields << endl;
    cerr << "-------->> _maxRaysPpi: " << _maxRaysPpi << endl;
    cerr << "-------->> _maxGatesPpi: " << _maxGatesPpi << endl;
  }
  
  // check startRange / rangeRes is same for multiple fields for Beijing data
  if (_nFields > 1) {
    if (_ppiFields[0]->getName() == "Refl" && _ppiFields[1]->getName() == "Vel") {
      if (_ppiFields[0]->getRangeRes() != _ppiFields[1]->getRangeRes()) {
	if (_debug) {
	  cerr << "******************************************" << endl;
	  cerr << "WARNING - nFields > 1 "
               << "and range resolution different for fields" << endl; 
	  for ( int ifield = 0; ifield < _nFields; ifield++) {
	    cerr << "  field name: " << _ppiFields[ifield]->getName() 
		 << "  maxGatesPpi: " << _ppiFields[ifield]->getMaxGates()  
		 << "  start range: " << _ppiFields[ifield]->getStartRange() 
		 << "  range res:  " << _ppiFields[ifield]->getRangeRes() << endl; 
	  }
          cerr << " _maxGatesPpi:  " << _maxGatesPpi << endl;
	  cerr << "******************************************" << endl;
	}

	// remap refl data to same resolution as vel data 

	_ppiFields[0]->convertNexradResolution(_maxGatesPpi); 

	if (_debug) {
	  cerr << "Remap refl to same rangeRes as vel" << endl;
	  cerr << "  field name: " << _ppiFields[0]->getName()
               << "  maxGatesPpi: " << _ppiFields[0]->getMaxGates()
               << "  startRange: " << _ppiFields[0]->getStartRange() 
               << "  rangeRes: " << _ppiFields[0]->getRangeRes() << endl;
	  cerr << endl;
	  cerr << "******************************************" << endl;
	}
      }
    }
  }

  if (_debug) {
    cerr << "-->> Ending ppi num: " << sweepNum << endl;
  }

  return 0;

}

////////////////////////////////////////
// Add rays to volume

void RapicRadxFile::_addRaysPpi(int sweepNum)

{
  
  // check ppi fields have correct number of rays
  
  int nRays = (int) ((360.0 / _scanParams->angle_res) + 0.5);
  
  if (_debug) {
    for (int ifield = 0; ifield < _nFields; ifield++) {
      // Rapic format data does not send blank radls,
      // so size will often be < nRays
      if ((int) _ppiFields[ifield]->getRays().size() != nRays) {
	cerr << "WARNING - RapicRadxFile::_loadPPI" << endl;
	cerr << "  Field does not have correct number of rays" << endl;
	cerr << "  Field " << ifield << " has "
	     << _ppiFields[ifield]->getRays().size() << " rays" << endl;
	cerr << "  Expected number: " << nRays << endl;
      }
    } // ifield
  }

  // initialize the ray index array

  RadxArray<size_t> rayIndex_;
  size_t *rayIndex = rayIndex_.alloc(_nFields);
  for (int ifield = 0; ifield < _nFields; ifield++) {
    rayIndex[ifield] = 0;
  }

  // loop through the azimuths
  
  for (int iray = 0; iray < _maxRaysPpi; iray++) {

    // find the next azimuth and elevation in sequence

    time_t rayTime = 0;
    double az = 0;
    double el = 0;
    bool dataFound = false;
    
    for (int ifield = 0; ifield < (int) _ppiFields.size(); ifield++) {
      const PPIField *ppiField = _ppiFields[ifield];
      size_t index = rayIndex[ifield];
      if (index < ppiField->getRays().size()) {

	RapicRay *ray = ppiField->getRays()[index];
        
        rayTime = ray->timeSecs;

	if (!dataFound) {
	  az = ray->azimuth;
	  el = ray->elevation;
	  dataFound = true;
	} else {
	  double az1 = ray->azimuth;
	  if (_azLessThan(az1, az)) {
	    az = az1;
	    el = ray->elevation;
	  }
	}

      } // if (index ...
    } // ifield

    if (!dataFound) {
      // all fields done
      break;
    }

    // create new ray

    RadxRay *radxRay = new RadxRay;
    radxRay->setTime(rayTime);
    scan_description_t &scanDesc = _scanList[_ppiFields[0]->getScanNum()];
    // if (_scanParams->tilt_num >= 0 && 
    //     _scanParams->tilt_num <= (int) _scanList.size()) {
    //   radxRay->setSweepNumber(_scanParams->tilt_num);
    // } else {
    radxRay->setSweepNumber(scanDesc.sweep_num);
    // }
    radxRay->setVolumeNumber(_volumeNumber);
    radxRay->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
    if (_dualPol) {
      radxRay->setPolarizationMode(Radx::POL_MODE_HV_SIM);
    } else {
      radxRay->setPolarizationMode(Radx::POL_MODE_HORIZONTAL);
    }
    radxRay->setAzimuthDeg(az);
    radxRay->setElevationDeg(el);
    radxRay->setFixedAngleDeg
      (_scanList[_ppiFields[0]->getScanNum()].elev_angle);
    if (_scanParams->angle_res > 0) {
      radxRay->setIsIndexed(true);
      radxRay->setAngleResDeg(_scanParams->angle_res);
    }
    radxRay->setPulseWidthUsec(_scanParams->pulse_length);
    radxRay->setPrtSec(1.0 / _scanParams->prf);
    radxRay->setNyquistMps(_scanParams->nyquist);
    radxRay->setTargetScanRateDegPerSec(_scanParams->angle_rate);

    double startRangeKm = _scanParams->start_range / 1000.0;
    double gateSpacingKm = _scanParams->range_res / 1000.0;
    radxRay->setRangeGeom(startRangeKm, gateSpacingKm);

    // for each field which has a matching azimuth, load up the data

    for (int ifield = 0; ifield < (int) _ppiFields.size(); ifield++) {
      
      const PPIField *ppiField = _ppiFields[ifield];
      size_t index = rayIndex[ifield];

      if (index < ppiField->getRays().size()) {
	
	RapicRay *ray = ppiField->getRays()[index];
	double az1 = ray->azimuth;
        
	if (az == az1) {
	  
          if (isFieldRequiredOnRead(ppiField->getName())) {

            // load the data for this field
            
            RadxArray<Radx::fl32> fdata_;
            Radx::fl32 *fdata = fdata_.alloc(ray->nGates);
            Radx::fl32 missing = Radx::missingFl32;
            
            for (int igate = 0; igate < ray->nGates; igate++) {
              Radx::ui08 uval = ray->vals[igate];
              Radx::fl32 fval = missing;
              if (uval != 0) {
                fval = uval * ppiField->getScale() + ppiField->getBias();
              }
              fdata[igate] = fval;
            }
            
            RadxField *field = new RadxField(ppiField->getName(),
                                             ppiField->getUnits());
            field->setDataFl32(ray->nGates, fdata, true);
            field->setRangeGeom(startRangeKm, gateSpacingKm);
            field->convertToSi16();
            radxRay->addField(field);

          } // if (isFieldRequiredOnRead ...

	  rayIndex[ifield]++;

	} // if (az == az1)

      } // if (index < ppiField->getRays().size())
      
    } // ifield

    // add ray to volume

    _readVol->addRay(radxRay);
    
  } // iray

}

///////////////////////////
// add the rays for the ppi

void RapicRadxFile::_clearPpiFields()

{
  for (int ii = 0; ii < (int) _ppiFields.size(); ii++) {
    delete _ppiFields[ii];
  }
  _ppiFields.clear();
}

//////////////////
// decode a radial

int RapicRadxFile::_decodeRadial(Linebuff &lineBuf,
                                 sRadl &radial,
                                 bool &isBinary,
                                 int rLevels)
  
{

  isBinary = true;

  if (lineBuf.IsBinRadl()) {

    if (sRadl::DecodeBinaryRadl((unsigned char *) lineBuf.line_buff,
				&radial) < 0) {
      _addErrStr("ERROR - RapicRadxFile::_decodeRadial");
      _addErrStr("  calling DecodeBinaryRadl");
      return -1;
    }

    isBinary = true;
    
  } else if (lineBuf.IsRadl()) {
    if (rLevels == 6) {
      if (sRadl::RLE_6L_radl(lineBuf.line_buff,
			     &radial) < 0) {
	_addErrStr("ERROR - RapicRadxFile::_decodeRadial");
	_addErrStr("  calling RLE_6L_radl");
	return -1;
      }
 
    }
    else {
      int NumLevels = 256;
      if (sRadl::RLE_16L_radl(lineBuf.line_buff,
			      &radial, NumLevels-1) < 0) {
	_addErrStr("ERROR - RapicRadxFile::_decodeRadial");
	_addErrStr("  calling RLE_16L_radl");
	return -1;
      }
    }
    isBinary = false;
    
  }
  
  return 0;

}

//////////////////////
// clear the scan list

void RapicRadxFile::_clearScanList()

{
  _scanListComplete = false;
  _nScansFull = 0;
  _nFields = 0;
  _scanList.clear();
  _elevList.clear();
}

////////////////////////
// add to the scan list

int RapicRadxFile::_addToScanList(Linebuff &lineBuf)

{

  if (_debug) {
    cerr << "Adding to scan list, line: >>>" << lineBuf.line_buff << "<<<" << endl;
  }

  if (_nScansFull == 0) {
    int nscans;
    if (sscanf(lineBuf.line_buff, "/IMAGESCANS: %d", &nscans) == 1) {
      _nScansFull = nscans;
    }
    return 0;
  }

  scan_description_t scan;
  MEM_zero(scan);
  if (sscanf(lineBuf.line_buff,
	     "/SCAN%d:%d%s%d%lg%d%d%d%d",
	     &scan.scan_num,
	     &scan.station_id,
	     scan.time_str,
	     &scan.flag1,
	     &scan.elev_angle,
	     &scan.field_num,
	     &scan.n_fields,
	     &scan.flag2,
	     &scan.flag3) == 9) {
    if (_scanList.size() == 0) {
      scan.sweep_num = 1;
    } else {
      scan_description_t &prevScan = _scanList[_scanList.size() - 1];
      double prevElevAngle = prevScan.elev_angle;
      double thisElevAngle = scan.elev_angle;
      double diff = fabs(thisElevAngle - prevElevAngle);
      if (diff > 0.01) {
        scan.sweep_num = prevScan.sweep_num + 1;
      } else {
        scan.sweep_num = prevScan.sweep_num;
      }
    }
    _scanList.push_back(scan);
    return 0;
  }
  
  if (strstr(lineBuf.line_buff, "/IMAGEHEADER END") != NULL) {
    _scanListComplete = true;
    if (_debug) {
      _printScanList(cerr);
    }
    if ((int) _scanList.size() != _nScansFull) {
      _addErrStr("WARNING - RapicRadxFile::_loadScanList");
      _addErrStr("  Only partial volume found");
      _addErrInt("  n scans in full volume: ", _nScansFull);
      _addErrInt("  n scans found: ", _scanList.size());
    }
  }

  return 0;

}

//////////////////////
// print the scan list

void RapicRadxFile::_printScanList(ostream &out)

{

  out << "SCAN LIST" << endl;
  out << "=========" << endl;
  out << endl;

  out << "_scanList.size: " << _scanList.size() << endl;
  for (size_t ii = 0; ii < _scanList.size(); ii++) {
    out << "#" << _scanList[ii].scan_num
	<< " id: " << _scanList[ii].station_id
	<< " time: " << _scanList[ii].time_str
	<< " flag1: " << _scanList[ii].flag1
	<< " elev: " << _scanList[ii].elev_angle
	<< " field_num: " << _scanList[ii].field_num
	<< " n_fields: " << _scanList[ii].n_fields
	<< " flag2: " << _scanList[ii].flag2
	<< " flag3: " << _scanList[ii].flag3
	<< endl;
  } // ii

  if (!_scanListComplete) {
    _addErrStr("ERROR - scan list not yet complete");
    return;
  }

}

//////////////////////////////////////////////////
// load scan params
//
// Return 0 on success, -1 on failure

int RapicRadxFile::_loadScanParams(Linebuff &lineBuf)

{

  while (lineBuf.readNext() == 0) {
    
    if (lineBuf.IsBinRadl() || lineBuf.IsRadl()) {

      // into radial data, set the linebuf to repeat this line
      // and break out

      lineBuf.setRepeat();
      break;
      
    } else {
      
      // params info
	
      if (_debug) {
	cerr << "Params info >>> " << lineBuf.line_buff << endl;
      }
      if (_scanParams->set(lineBuf.line_buff, _nScansFull)) {
	_addErrStr("ERROR - RapicRadxFile::_loadScanParams");
	_addErrStr("  Cannot load scan params");
	return -1;
      }
      
    } // if (lineBuf.IsBinRadl() || lineBuf.IsRadl()) 
      
    lineBuf.reset();

  } // while

  if (!_scanParams->setDone || _scanParams->scan_num == 0) {
    return -1;
  }

  if (_debug) {
    _scanParams->print(cerr);
  }

  return 0;

}

////////////////////////////////////
// check that the scan params match

int RapicRadxFile::_checkScanParams()

{

  if (!_scanParams->setDone || _scanParams->scan_num == 0) {
    _addErrStr("ERROR - RapicRadxFile::_checkScanParams");
    _addErrStr("  Scan params not set");
    return -1;
  }

  if (_verbose) {
    _scanParams->print(cerr);
  }

  // check elevation

  for (size_t ii = 0; ii < _scanList.size(); ii++) {

    const scan_description_t &scan = _scanList[ii];

    if (_scanParams->scan_num == scan.scan_num) {
      
      //  rjp 7 Sep 2001. Make same as wxwdss version.
      //  if (_scanParams->elev_angle != scan.elev_angle) {
      if (fabs(_scanParams->elev_angle - scan.elev_angle) > 0.5)  {
        _addErrStr("ERROR - RapicRadxFile::_checkScanParams");
        _addErrInt("  Scan number: ", _scanParams->scan_num);
        _addErrStr("  Incorrect elevation angle");
        _addErrDbl("  Should be: ", scan.elev_angle, "%lg");
        _addErrDbl("  Found: ", _scanParams->elev_angle, "%lg");
        return -1;
      }

      break;
      
    }

  } // ii

  return 0;

}

//////////////////////////
// test for azimiuth order
//
// Returns true if az1 < az2

bool RapicRadxFile::_azLessThan(double az1, double az2)

{
  if (az1 < az2) {
    if (az2 - az1 < 180.0) {
      return true;
    }
  } else {
    if (az1 - az2 > 180.0) {
      return true;
    }
  }
  return false;
}


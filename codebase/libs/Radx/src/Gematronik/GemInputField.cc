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
// GemInputField.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
///////////////////////////////////////////////////////////////

#include "GemInputField.hh"
#include "GemSweep.hh"
#include <cerrno>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <Radx/RadxTime.hh>
#include <Radx/RadxStr.hh>
using namespace std;

// Constructor

GemInputField::GemInputField(const string &fileName,
                             const string &filePath,
                             time_t fileTime,
                             const string &fieldName,
                             const string &standardName,
                             const string &longName,
                             const string &units,
                             bool debug,
                             bool verbose) :
        _debug(debug),
        _verbose(verbose),
        _fileName(fileName),
        _filePath(filePath),
        _fieldName(fieldName),
        _standardName(standardName),
        _longName(longName),
        _units(units),
        _fileTime(fileTime)

{

  clear();

}

// destructor

GemInputField::~GemInputField()

{

  clear();

}

//////////////////
// read file
// returns 0 on success, -1 on failure

int GemInputField::read()
  
{

  // clear any previous data

  clear();

  // get file size

  struct stat fileStat;
  if (stat(_filePath.c_str(), &fileStat)) {
    int errNum = errno;
    RadxStr::addStr(_errStr, "ERROR - GemInputField::read");
    RadxStr::addStr(_errStr, "  Cannot stat file: ", _filePath);
    RadxStr::addStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  off_t fileLen = fileStat.st_size;
  
  // open file

  FILE *in;
  if ((in = fopen(_filePath.c_str(), "r")) == NULL) {
    int errNum = errno;
    RadxStr::addStr(_errStr, "ERROR - GemInputField::read");
    RadxStr::addStr(_errStr, "  Cannot open file: ", _filePath);
    RadxStr::addStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

  // read in buffer

  char *fileBuf = new char[fileLen + 1];
  if ((int) fread(fileBuf, 1, fileLen, in) != fileLen) {
    int errNum = errno;
    RadxStr::addStr(_errStr, "ERROR - GemInputField::read");
    RadxStr::addStr(_errStr, "  Cannot read file: ", _filePath);
    RadxStr::addStr(_errStr, "  ", strerror(errNum));
    fclose(in);
    delete[] fileBuf;
    return -1;
  }

  // ensure null termination
  fileBuf[fileLen] = '\0';

  // close file

  fclose(in);

  // find the end of the XML

  char *endPtr;
  endPtr = strstr(fileBuf, "<!-- END XML -->");
  if (endPtr == NULL) {
    RadxStr::addStr(_errStr, "ERROR - GemInputField::read");
    RadxStr::addStr(_errStr, "  Cannot find <!-- END XML --> string in file: ", _filePath);
    delete[] fileBuf;
    return -1;
  }
  int xmlLen = endPtr - fileBuf;

  // create a string for the XML portion
  
  string xmlBuf(fileBuf, xmlLen);
  _xmlStr = xmlBuf;

  // decode the XML part of the file
  
  if (_decodeXml(xmlBuf)) {
    delete[] fileBuf;
    return -1;
  }

  if (_sweeps.size() < 1) {
    RadxStr::addStr(_errStr, "ERROR - GemInputField::read");
    RadxStr::addStr(_errStr, "  No sweeps found");
  }

  // decode the BLOBs (Binary Large Objects)

  if (_decodeBlobs(fileBuf, fileLen)) {
    delete[] fileBuf;
    return -1;
  }

  // set the angles and data on each sweep

  for (int ii = 0; ii < (int) _sweeps.size(); ii++) {
    GemSweep &sweep = *_sweeps[ii];
    for (int jj = 0; jj < (int) _blobs.size(); jj++) {
      const GemBlob &blob = *_blobs[jj];
      if (sweep.getAnglesBlobId() == blob.getId()) {
        sweep.setAngles(blob);
      } else if (sweep.getDataBlobId() == blob.getId()) {
        sweep.setFieldData(blob);
      }
    }
  }

  // delete file buffer

  delete[] fileBuf;

  return 0;

}

//////////////////
// clear data

void GemInputField::clear()

{

  _errStr.clear();

  _volTime = 0;
  _radarAlt = 0;
  _radarLat = 0;
  _radarLon = 0;
  _radarName = "unknown";
  _radarWavelength = 0.05;
  _radarBeamwidth = 1.0;

  _scanName = "unknown";
  _scanTime = 0;
  _scanNumEle = 0;
  _scanFirstEle = 0;
  _scanLastEle = 0;

  _polarization = "unknown";
  _pulseWidthUs = Radx::missingMetaDouble;
  _antennaSpeed = 0;

  for (int ii = 0; ii < (int) _sweeps.size(); ii++) {
    delete _sweeps[ii];
  }
  _sweeps.clear();

  for (int ii = 0; ii < (int) _blobs.size(); ii++) {
    delete _blobs[ii];
  }
  _blobs.clear();

}

//////////////////
// print

void GemInputField::print(ostream &out) const
  
{

  out << "=====================================" << endl;
  out << "Input file object" << endl;
  out << "  FileName: " << _fileName << endl;
  out << "  FilePath: " << _filePath << endl;
  out << "  FileTime: " << _fileTime << endl;
  out << "  FieldName: " << _fieldName << endl;
  out << "  StandardName: " << _standardName << endl;
  out << "  LongName: " << _longName << endl;
  out << "  Units: " << _units << endl;

  out << "  VolTime: " << RadxTime::strm(_volTime) << endl;
  out << "  Radar info:" << endl;
  out << "    alt: " << _radarAlt << endl;
  out << "    lat: " << _radarLat << endl;
  out << "    lon: " << _radarLon << endl;
  out << "    name: " << _radarName << endl;
  out << "    wavelength: " << _radarWavelength << endl;
  out << "    beamwidth: " << _radarBeamwidth << endl;
  
  out << "  Scan info:" << endl;
  out << "    scanName: " << _scanName << endl;
  out << "    scanTime: " << RadxTime::strm(_scanTime) << endl;
  out << "    scanNumEle: " << _scanNumEle << endl;
  out << "    scanFirstEle: " << _scanFirstEle << endl;
  out << "    scanLastEle: " << _scanLastEle << endl;
  out << "    polarization: " << _polarization << endl;
  out << "    pulseWidthUs: " << _pulseWidthUs << endl;
  out << "    antennaSpeed: " << _antennaSpeed << endl;
  
  for (int ii = 0; ii < (int) _sweeps.size(); ii++) {
    out << "-------------------------------------" << endl;
    _sweeps[ii]->print(out);
  }
  
  out << "=====================================" << endl;

}

/////////////////////////////////////
// decode the XML
// returns 0 on success, -1 on failure

int GemInputField::_decodeXml(const string &xmlBuf)
  
{

  if (_verbose) {
    cerr << "===XML===XML===XML===XML===XML===XML" << endl;
    cerr << xmlBuf << endl;
    cerr << "===XML===XML===XML===XML===XML===XML" << endl;
  }

  // volume info

  vector<RadxXml::attribute> attributes;
  string volBuf;
  if (RadxXml::readString(xmlBuf, "volume", volBuf, attributes)) {
    RadxStr::addStr(_errStr, "ERROR - GemInputField::_decodeXml");
    RadxStr::addStr(_errStr, "  Cannot find <volume> tag");
    return -1;
  }
  
  // get time

  if (_decodeRadxTime(attributes, _volTime)) {
    RadxStr::addStr(_errStr, "ERROR - GemInputField::_decodeXml");
    RadxStr::addStr(_errStr, "  Cannot decode volume date and time");
    return -1;
  }

  // get type

  _volType = "vol";
  for (size_t ii = 0; ii < attributes.size(); ii++) {
    const RadxXml::attribute &att = attributes[ii];
    if (att.getName() == "type") {
      _volType = att.getVal();
    }
  }

  // get radar info or sensor info

  attributes.clear();
  string sensorBuf;
  if (RadxXml::readString(volBuf, "radarinfo", sensorBuf, attributes) == 0) {
    
    // use <radarinfo> tag, Rainbow 5.30 and before
    //
    // Example:
    // <radarinfo alt="148.000000" lon="8.527500" lat="50.038600" id="GEMA" >
    //    <name>Gematronik</name>
    //    <wavelen>0.0531</wavelen>
    //    <beamwidth>1</beamwidth>
    // </radarinfo>

    RadxXml::readStringAttr(attributes, "id", _radarId);
    RadxXml::readDoubleAttr(attributes, "alt", _radarAlt);
    RadxXml::readDoubleAttr(attributes, "lat", _radarLat);
    RadxXml::readDoubleAttr(attributes, "lon", _radarLon);
    RadxXml::readString(sensorBuf, "name", _radarName);
    RadxXml::readDouble(sensorBuf, "wavelen", _radarWavelength);
    RadxXml::readDouble(sensorBuf, "beamwidth", _radarBeamwidth);

  } else if (RadxXml::readString(volBuf, "sensorinfo", 
                                 sensorBuf, attributes) == 0) {
    
    // use <sensorinfo> tag, Rainbow 5.31 and later
    //
    // Example:
    // <sensorinfo type="rcl" id="GEMA" name="Gematronik">
    //    <lon>8.527500</lon>
    //    <lat>50.038600</lat>
    //    <alt>148.000000</alt>
    //    <wavelen>0.0531</wavelen>
    //    <beamwidth>1</beamwidth>
    //  </sensorinfo>
    
    RadxXml::readStringAttr(attributes, "id", _radarId);
    RadxXml::readStringAttr(attributes, "name", _radarName);
    RadxXml::readStringAttr(attributes, "type", _radarType);
    RadxXml::readDouble(sensorBuf, "lat", _radarLat);
    RadxXml::readDouble(sensorBuf, "lon", _radarLon);
    RadxXml::readDouble(sensorBuf, "alt", _radarAlt);
    RadxXml::readDouble(sensorBuf, "wavelen", _radarWavelength);
    RadxXml::readDouble(sensorBuf, "beamwidth", _radarBeamwidth);

  } else if (RadxXml::readString(volBuf, "radar", 
                                 sensorBuf, attributes) == 0) {
    
    // use <radar> tag, early releases
    //
    // Example:
    //  <radar alt="146" lon="15.8311" lat="53.7903" >
    //    <radarname>Swidwin</radarname>
    //    <wavelen>0.053</wavelen>
    //    <beamwidth>1</beamwidth>
    // </radar>
    
    RadxXml::readDoubleAttr(attributes, "alt", _radarAlt);
    RadxXml::readDoubleAttr(attributes, "lat", _radarLat);
    RadxXml::readDoubleAttr(attributes, "lon", _radarLon);
    RadxXml::readString(sensorBuf, "radarname", _radarName);
    RadxXml::readDouble(sensorBuf, "wavelen", _radarWavelength);
    RadxXml::readDouble(sensorBuf, "beamwidth", _radarBeamwidth);

  } else {
  
    RadxStr::addStr(_errStr, "ERROR - GemInputField::_decodeXml");
    RadxStr::addStr(_errStr, "  Cannot find <radarinfo> or <sensorinfo>");
    RadxStr::addStr(_errStr, "  At least one must be present");
    return -1;

  }

  // get scan info
  
  attributes.clear();
  string scanBuf;
  if (RadxXml::readString(volBuf, "scan", scanBuf, attributes)) {
    RadxStr::addStr(_errStr, "ERROR - GemInputField::_decodeXml");
    RadxStr::addStr(_errStr, "  Cannot find scan info");
    return -1;
  }

  RadxXml::readStringAttr(attributes, "name", _scanName);
  _decodeRadxTime(attributes, _scanTime);

  attributes.clear();
  string pargroupBuf;
  if (RadxXml::readString(scanBuf, "pargroup", pargroupBuf, attributes)) {
    RadxStr::addStr(_errStr, "ERROR - GemInputField::_decodeXml");
    RadxStr::addStr(_errStr, "  Cannot find pargroup");
    return -1;
  }
  
  if (RadxXml::readInt(pargroupBuf, "numele", _scanNumEle)) {
    RadxStr::addStr(_errStr, "ERROR - GemInputField::_decodeXml");
    RadxStr::addStr(_errStr, "  Cannot find num ele");
    return -1;
  }

  if (RadxXml::readDouble(pargroupBuf, "startAzi", _startAzi)) {
    RadxXml::readDouble(pargroupBuf, "startazi", _startAzi);
  }
  if (RadxXml::readDouble(pargroupBuf, "stopAzi", _stopAzi)) {
    RadxXml::readDouble(pargroupBuf, "stopazi", _stopAzi);
  }

  if (RadxXml::readDouble(pargroupBuf, "firstele", _scanFirstEle)) {
    RadxXml::readDouble(pargroupBuf, "startele", _scanFirstEle);
  }
  if (RadxXml::readDouble(pargroupBuf, "lastele", _scanLastEle)) {
    RadxXml::readDouble(pargroupBuf, "stopele", _scanLastEle);
  }

  RadxXml::readString(pargroupBuf, "pol", _polarization);
  RadxXml::readDouble(pargroupBuf, "pw_index", _pulseWidthUs);
  RadxXml::readDouble(pargroupBuf, "antspeed", _antennaSpeed);

  // deduce scan mode

  if (_volType == "ele") {
    _isRhi = true;
    _isSector = false;
  } else if (_volType == "azi") {
    _isRhi = false;
    double deltaStartStop = fabs(_stopAzi - _startAzi);
    if (deltaStartStop > 180) {
      deltaStartStop = fabs(deltaStartStop - 360.0);
    }
    if (deltaStartStop < 5) {
      // surveillance
      _isSector = false;
    } else {
      // sector
      _isSector = true;
    }
  } else {
    // surveillance
    _isRhi = false;
    _isSector = false;
  }

  // read in array of tag buffers for slices

  vector<string> tagBufArray;
  
  if (RadxXml::readTagBufArray(scanBuf, "slice", tagBufArray)) {
    RadxStr::addStr(_errStr, "ERROR - GemInputField::_decodeXml");
    RadxStr::addStr(_errStr, "  Cannot find slices");
    return -1;
  }

  if ((int) tagBufArray.size() != _scanNumEle) {
    cerr << "WARNING - GemInputField::_decodeXml" << endl;
    cerr << "  Number of slice tags does not match <numele>." << endl;
    cerr << "  numele: " << _scanNumEle << endl;
    cerr << "  num slices: " << tagBufArray.size() << endl;
    if (_scanNumEle > (int) tagBufArray.size()) {
      _scanNumEle = (int) tagBufArray.size();
      cerr << "Changing numele to: " << _scanNumEle << endl;
    } else {
      cerr << "Only using the first " << _scanNumEle << " slices" << endl;
    }
  }

  GemSweep *prevSweep = NULL;

  for (int ii = 0; ii < _scanNumEle; ii++) {
    
    // If a previous sweep exists, use this as a template in 
    // case some info is not available in the XML

    GemSweep *sweep;
    if (prevSweep == NULL) {
      sweep = new GemSweep(ii, _debug, _verbose);
    } else {
      sweep = new GemSweep(*prevSweep, ii, _debug, _verbose);
    }

    if (sweep->decodeInfoXml(tagBufArray[ii])) {
      RadxStr::addStr(_errStr, "ERROR - GemInputField::_decodeXml");
      RadxStr::addInt(_errStr, "  Cannot decode XML for sweep: ", ii);
      delete sweep;
      return -1;
    }

    _sweeps.push_back(sweep);
    prevSweep = sweep;

  } // ii
 
  return 0;

}

////////////////////////////////////////
// decode date and time from attributes

int GemInputField::_decodeRadxTime(const vector<RadxXml::attribute> &attributes,
                                   time_t &time)

  
{

  string dateTimeStr;
  if (RadxXml::readStringAttr(attributes, "datetimehighaccuracy", dateTimeStr) == 0) {
    if (RadxXml::readTime(dateTimeStr, time) == 0) {
      return 0;
    }
  }
  if (RadxXml::readStringAttr(attributes, "datetime", dateTimeStr) == 0) {
    if (RadxXml::readTime(dateTimeStr, time) == 0) {
      return 0;
    }
  }
  
  string dateStr, timeStr;
  if (RadxXml::readStringAttr(attributes, "date", dateStr)) {
    RadxStr::addStr(_errStr, "  Cannot find date attribute");
    return -1;
  }
  if (RadxXml::readStringAttr(attributes, "time", timeStr)) {
    RadxStr::addStr(_errStr, "  Cannot find time attribute");
    return -1;
  }
  int year, month, day, hour, min, sec;
  if (sscanf(dateStr.c_str(), "%4d-%2d-%2d", &year, &month, &day) != 3) {
    RadxStr::addStr(_errStr, "  Cannot decode date attribute");
    return -1;
  }
  if (sscanf(timeStr.c_str(), "%2d:%2d:%2d", &hour, &min, &sec) != 3) {
    RadxStr::addStr(_errStr, "  Cannot decode time attribute");
    return -1;
  }
  RadxTime volTime(year, month, day, hour, min, sec);
  time = volTime.utime();
  return 0;

}

//////////////////////////////////////////
// decode the BLOBs - binary large objects
// returns 0 on success, -1 on failure

int GemInputField::_decodeBlobs(const char *fileBuf, int fileLen)
  
{

  if (_verbose) {
    cerr << "Decoding BLOBs" << endl;
    cerr << "===>> fileLen: " << fileLen << endl;
  }

  const char *ptr = fileBuf;
  const char *endOfBuf = fileBuf + fileLen;
  const char *startTok = "<BLOB";
  const char *endTok = "</BLOB>";
  
  while (ptr < endOfBuf - 7) {

    // look for starting pattern
    
    if (strncmp(ptr, startTok, 5) == 0) {

      if (_verbose) {
        cerr << "==>> Found BLOB start, pos: " << (ptr - fileBuf) << endl;
      }

      // find the limits of the attributes
      
      const char *attrStart = ptr + 5;
      const char *attrEnd = strchr(attrStart, '>');
      if (attrEnd == NULL) {
        RadxStr::addInt(_errStr, "ERROR - BLOB pos: ", (int) (ptr - fileBuf));
        RadxStr::addStr(_errStr, "  Cannot find closing '>'.");
        return -1;
      }
      int attrLen = attrEnd - attrStart;
      string attrStr(attrStart, attrLen);

      if (_verbose) {
        cerr << "  attrStr: " << attrStr << endl;
      }

      vector<RadxXml::attribute> attrs;
      RadxXml::attrDecode(attrStr, attrs);

      int blobId;
      if (RadxXml::readIntAttr(attrs, "blobid", blobId)) {
        RadxStr::addInt(_errStr, "ERROR - BLOB pos: ", (int) (ptr - fileBuf));
        RadxStr::addStr(_errStr, "  Cannot find blobid attribute");
        return -1;
      }

      int blobSize;
      if (RadxXml::readIntAttr(attrs, "size", blobSize)) {
        RadxStr::addInt(_errStr, "ERROR - BLOB pos: ", (int) (ptr - fileBuf));
        RadxStr::addStr(_errStr, "  Cannot find size attribute");
        return -1;
      }

      string blobCompression;
      if (RadxXml::readStringAttr(attrs, "compression", blobCompression)) {
        RadxStr::addInt(_errStr, "ERROR - BLOB pos: ", (int) (ptr - fileBuf));
        RadxStr::addStr(_errStr, "  Cannot find compression attribute");
        return -1;
      }
      
      if (_verbose) {
        cerr << "  ===>> blobId: " << blobId << endl;
        cerr << "  ===>> blobSize: " << blobSize << endl;
        cerr << "  ===>> blobCompression: " << blobCompression << endl;
      }
      
      // blob starts after '>\n\0'
      // create new Blob object
      
      const char *blobStart = attrEnd + 2;
      GemBlob *blob = new GemBlob(blobId, _verbose);
      if (blob->loadData(blobSize, blobCompression, blobStart)) {
        delete blob;
        return -1;
      }
      _blobs.push_back(blob);
      
      const char *blobEnd = blobStart + blobSize;

      if (strncmp(blobEnd, endTok, 7) == 0) {
        if (_verbose) {
          cerr << "==>> Found BLOB end, pos: " << (blobEnd - fileBuf) << endl;
        }
      }

      ptr = blobEnd;
      
    } else {

      ptr++;

    }

  } // while

  return 0;

}



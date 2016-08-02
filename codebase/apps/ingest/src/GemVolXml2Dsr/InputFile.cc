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
// InputFile.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2006
//
///////////////////////////////////////////////////////////////

#include "InputFile.hh"
#include "Tilt.hh"
#include <cerrno>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
using namespace std;

// Constructor

InputFile::InputFile(const Params &params,
                     const char *input_field_name,
                     const char *output_field_name,
                     const char *output_units,
                     bool required) :
        _params(params),
        _inputFieldName(input_field_name),
        _outputFieldName(output_field_name),
        _outputUnits(output_units),
        _required(required)

{

  clear();

}

// destructor

InputFile::~InputFile()

{

  clear();

}

//////////////////
// read file
// returns 0 on success, -1 on failure

int InputFile::read(const string &path,
                    const string &name,
                    time_t time,
                    int nn)
  
{

  if (_params.debug) {
    cerr << "---->> Reading file, path: " << path << endl;
    cerr << "  name: " << name << endl;
    cerr << "  time: " << DateTime::strm(time) << endl;
    cerr << "  nn: " << nn << endl;
  }

  // clear any previous data

  clear();

  // get file size

  stat_struct_t fileStat;
  if (ta_stat(path.c_str(), &fileStat)) {
    int errNum = errno;
    cerr << "ERROR - InputFile::read" << endl;
    cerr << "  Cannot stat file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  off_t fileLen = fileStat.st_size;
  
  // open file

  FILE *in;
  if ((in = fopen(path.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - InputFile::read" << endl;
    cerr << "  Cannot open file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // read in buffer

  char *fileBuf = new char[fileLen + 1];
  if ((int) fread(fileBuf, 1, fileLen, in) != fileLen) {
    int errNum = errno;
    cerr << "ERROR - InputFile::read" << endl;
    cerr << "  Cannot read file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
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
    cerr << "ERROR - InputFile::read" << endl;
    cerr << "  Cannot find <!-- END XML --> string in file: " << path << endl;
    delete[] fileBuf;
    return -1;
  }
  int xmlLen = endPtr - fileBuf;

  // create a string for the XML portion
  
  string xmlBuf(fileBuf, xmlLen);

  // decode the XML part of the file
  
  if (_decodeXml(xmlBuf)) {
    delete[] fileBuf;
    return -1;
  }

  // decode the BLOBs (Binary Large Objects)

  if (_decodeBlobs(fileBuf, fileLen)) {
    delete[] fileBuf;
    return -1;
  }

  // set the angles and data on each tilt

  for (int ii = 0; ii < (int) _tilts.size(); ii++) {
    Tilt &tilt = *_tilts[ii];
    for (int jj = 0; jj < (int) _blobs.size(); jj++) {
      const Blob &blob = *_blobs[jj];
      if (tilt.getAnglesBlobId() == blob.getId()) {
        tilt.setAzAngles(blob);
      } else if (tilt.getDataBlobId() == blob.getId()) {
        tilt.setFieldData(blob);
      }
    }
  }

  // delete file buffer

  delete[] fileBuf;

  // set members

  _path = path;
  _name = name;
  _fileTime = time;
  _nn = nn;
  _found = true;

  return 0;

}

//////////////////
// clear data

void InputFile::clear()

{

  _path.clear();
  _name.clear();
  _fileTime = 0;
  _nn = -1;
  _found = false;

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
  _pulseWidth = "unknown";
  _antennaSpeed = 0;

  for (int ii = 0; ii < (int) _tilts.size(); ii++) {
    delete _tilts[ii];
  }
  _tilts.clear();

  for (int ii = 0; ii < (int) _blobs.size(); ii++) {
    delete _blobs[ii];
  }
  _blobs.clear();

}

//////////////////
// print

void InputFile::print(ostream &out) const
  
{

  out << "=====================================" << endl;
  out << "Input file object" << endl;
  out << "  Input field: " << _inputFieldName << endl;
  out << "  Output field: " << _outputFieldName << endl;
  out << "  Units: " << _outputUnits << endl;
  out << "  Required field: " << (_required? "true" : "false") << endl;

  if (!_found) {
    return;
  }

  out << "  Input file found" << endl;
  out << "  Path: " << _path << endl;
  out << "  Name: " << _name << endl;
  out << "  TileTime: " << _fileTime << endl;
  out << "  Nn: " << _nn << endl;
  
  out << "  VolTime: " << DateTime::strm(_volTime) << endl;
  out << "  Radar info:" << endl;
  out << "    alt: " << _radarAlt << endl;
  out << "    lat: " << _radarLat << endl;
  out << "    lon: " << _radarLon << endl;
  out << "    name: " << _radarName << endl;
  out << "    wavelength: " << _radarWavelength << endl;
  out << "    beamwidth: " << _radarBeamwidth << endl;
  
  out << "  Scan info:" << endl;
  out << "    scanName: " << _scanName << endl;
  out << "    scanTime: " << DateTime::strm(_scanTime) << endl;
  out << "    scanNumEle: " << _scanNumEle << endl;
  out << "    scanFirstEle: " << _scanFirstEle << endl;
  out << "    scanLastEle: " << _scanLastEle << endl;
  out << "    polarization: " << _polarization << endl;
  out << "    pulseWidth: " << _pulseWidth << endl;
  out << "    antennaSpeed: " << _antennaSpeed << endl;
  
  for (int ii = 0; ii < (int) _tilts.size(); ii++) {
    out << "-------------------------------------" << endl;
    _tilts[ii]->print(out);
  }
  
  out << "=====================================" << endl;

}

/////////////////////////////////////
// decode the XML
// returns 0 on success, -1 on failure

int InputFile::_decodeXml(const string &xmlBuf)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===XML===XML===XML===XML===XML===XML" << endl;
    cerr << xmlBuf << endl;
    cerr << "===XML===XML===XML===XML===XML===XML" << endl;
  }

  // volume info

  vector<TaXml::attribute> attributes;
  string volBuf;
  if (TaXml::readString(xmlBuf, "volume", volBuf, attributes)) {
    cerr << "ERROR - InputFile::_decodeXml" << endl;
    cerr << "  Cannot find <volume> tag" << endl;
    return -1;
  }

  if (_decodeDateTime(attributes, _volTime)) {
    cerr << "ERROR - InputFile::_decodeXml" << endl;
    cerr << "  Cannot decode volume date and time" << endl;
    return -1;
  }

  // get radar info or sensor info

  attributes.clear();
  string sensorBuf;
  if (TaXml::readString(volBuf, "radarinfo", sensorBuf, attributes) == 0) {
    
    // use <radarinfo> tag, Rainbow 5.30 and before
    //
    // Example:
    // <radarinfo alt="148.000000" lon="8.527500" lat="50.038600" id="GEMA" >
    //    <name>Gematronik</name>
    //    <wavelen>0.0531</wavelen>
    //    <beamwidth>1</beamwidth>
    // </radarinfo>

    TaXml::readStringAttr(attributes, "id", _radarId);
    TaXml::readDoubleAttr(attributes, "alt", _radarAlt);
    TaXml::readDoubleAttr(attributes, "lat", _radarLat);
    TaXml::readDoubleAttr(attributes, "lon", _radarLon);
    TaXml::readString(sensorBuf, "name", _radarName);
    TaXml::readDouble(sensorBuf, "wavelen", _radarWavelength);
    TaXml::readDouble(sensorBuf, "beamwidth", _radarBeamwidth);

  } else if (TaXml::readString(volBuf, "sensorinfo", 
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
    
    TaXml::readStringAttr(attributes, "id", _radarId);
    TaXml::readStringAttr(attributes, "name", _radarName);
    TaXml::readStringAttr(attributes, "type", _radarType);
    TaXml::readDouble(sensorBuf, "lat", _radarLat);
    TaXml::readDouble(sensorBuf, "lon", _radarLon);
    TaXml::readDouble(sensorBuf, "alt", _radarAlt);
    TaXml::readDouble(sensorBuf, "wavelen", _radarWavelength);
    TaXml::readDouble(sensorBuf, "beamwidth", _radarBeamwidth);

  } else if (TaXml::readString(volBuf, "radar", 
                               sensorBuf, attributes) == 0) {
    
    // use <radar> tag, early releases
    //
    // Example:
    //  <radar alt="146" lon="15.8311" lat="53.7903" >
    //    <radarname>Swidwin</radarname>
    //    <wavelen>0.053</wavelen>
    //    <beamwidth>1</beamwidth>
    // </radar>
    
    TaXml::readDoubleAttr(attributes, "alt", _radarAlt);
    TaXml::readDoubleAttr(attributes, "lat", _radarLat);
    TaXml::readDoubleAttr(attributes, "lon", _radarLon);
    TaXml::readString(sensorBuf, "radarname", _radarName);
    TaXml::readDouble(sensorBuf, "wavelen", _radarWavelength);
    TaXml::readDouble(sensorBuf, "beamwidth", _radarBeamwidth);

  } else {
  
    cerr << "ERROR - InputFile::_decodeXml" << endl;
    cerr << "  Cannot find <radarinfo> or <sensorinfo>" << endl;
    cerr << "  At least one must be present" << endl;
    return -1;

  }

  // get scan info
  
  attributes.clear();
  string scanBuf;
  if (TaXml::readString(volBuf, "scan", scanBuf, attributes)) {
    cerr << "ERROR - InputFile::_decodeXml" << endl;
    cerr << "  Cannot find scan info" << endl;
    return -1;
  }

  TaXml::readStringAttr(attributes, "name", _scanName);
  _decodeDateTime(attributes, _scanTime);

  attributes.clear();
  string pargroupBuf;
  if (TaXml::readString(scanBuf, "pargroup", pargroupBuf, attributes)) {
    cerr << "ERROR - InputFile::_decodeXml" << endl;
    cerr << "  Cannot find pargroup" << endl;
    return -1;
  }
  
  if (TaXml::readInt(pargroupBuf, "numele", _scanNumEle)) {
    cerr << "ERROR - InputFile::_decodeXml" << endl;
    cerr << "  Cannot find num ele" << endl;
    return -1;
  }
  TaXml::readDouble(pargroupBuf, "firstele", _scanFirstEle);
  TaXml::readDouble(pargroupBuf, "lastele", _scanLastEle);
  TaXml::readString(pargroupBuf, "pol", _polarization);
  TaXml::readString(pargroupBuf, "pulsewidth", _pulseWidth);
  TaXml::readDouble(pargroupBuf, "antspeed", _antennaSpeed);

  // read in array of tag buffers for slices

  vector<string> tagBufArray;

  if (TaXml::readTagBufArray(scanBuf, "slice", tagBufArray)) {
    cerr << "ERROR - InputFile::_decodeXml" << endl;
    cerr << "  Cannot find slices" << endl;
    return -1;
  }

  if ((int) tagBufArray.size() != _scanNumEle) {
    cerr << "WARNING - InputFile::_decodeXml" << endl;
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

  Tilt *prevTilt = NULL;

  for (int ii = 0; ii < _scanNumEle; ii++) {
    
    // If a previous tilt exists, use this as a template in 
    // case some info is not available in the XML

    Tilt *tilt;
    if (prevTilt == NULL) {
      tilt = new Tilt(_params, ii);
    } else {
      tilt = new Tilt(*prevTilt, ii);
    }

    if (tilt->decodeInfoXml(tagBufArray[ii])) {
      cerr << "ERROR - InputFile::_decodeXml" << endl;
      cerr << "  Cannot decode XML for tilt: " << ii << endl;
      delete tilt;
      return -1;
    }

    _tilts.push_back(tilt);
    prevTilt = tilt;

  } // ii
 
  return 0;

}

////////////////////////////////////////
// decode date and time from attributes

int InputFile::_decodeDateTime(const vector<TaXml::attribute> &attributes,
                               time_t &time)

  
{

  string dateTimeStr;
  if (TaXml::readStringAttr(attributes, "datetime", dateTimeStr) == 0) {
    if (TaXml::readTime(dateTimeStr, time) == 0) {
      return 0;
    }
  }
  
  string dateStr, timeStr;
  if (TaXml::readStringAttr(attributes, "date", dateStr)) {
    cerr << "  Cannot find date attribute" << endl;
    return -1;
  }
  if (TaXml::readStringAttr(attributes, "time", timeStr)) {
    cerr << "  Cannot find time attribute" << endl;
    return -1;
  }
  int year, month, day, hour, min, sec;
  if (sscanf(dateStr.c_str(), "%4d-%2d-%2d", &year, &month, &day) != 3) {
    cerr << "  Cannot decode date attribute" << endl;
    return -1;
  }
  if (sscanf(timeStr.c_str(), "%2d:%2d:%2d", &hour, &min, &sec) != 3) {
    cerr << "  Cannot decode time attribute" << endl;
    return -1;
  }
  DateTime volTime(year, month, day, hour, min, sec);
  time = volTime.utime()  + _params.input_file_time_offset_secs;
  return 0;

}

//////////////////////////////////////////
// decode the BLOBs - binary large objects
// returns 0 on success, -1 on failure

int InputFile::_decodeBlobs(const char *fileBuf, int fileLen)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
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

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> Found BLOB start, pos: " << (ptr - fileBuf) << endl;
      }

      // find the limits of the attributes
      
      const char *attrStart = ptr + 5;
      const char *attrEnd = strchr(attrStart, '>');
      if (attrEnd == NULL) {
        cerr << "ERROR - BLOB num: " << (ptr - fileBuf) << endl;
        cerr << "  Cannot find closing '>'." << endl;
        return -1;
      }
      int attrLen = attrEnd - attrStart;
      string attrStr(attrStart, attrLen);

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "  attrStr: " << attrStr << endl;
      }

      vector<TaXml::attribute> attrs;
      TaXml::attrDecode(attrStr, attrs);

      int blobId;
      if (TaXml::readIntAttr(attrs, "blobid", blobId)) {
        cerr << "ERROR - BLOB num: " << (ptr - fileBuf) << endl;
        cerr << "  Cannot find blobid attribute" << endl;
        return -1;
      }

      int blobSize;
      if (TaXml::readIntAttr(attrs, "size", blobSize)) {
        cerr << "ERROR - BLOB num: " << (ptr - fileBuf) << endl;
        cerr << "  Cannot find size attribute" << endl;
        return -1;
      }

      string blobCompression;
      if (TaXml::readStringAttr(attrs, "compression", blobCompression)) {
        cerr << "ERROR - BLOB num: " << (ptr - fileBuf) << endl;
        cerr << "  Cannot find compression attribute" << endl;
        return -1;
      }
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "  ===>> blobId: " << blobId << endl;
        cerr << "  ===>> blobSize: " << blobSize << endl;
        cerr << "  ===>> blobCompression: " << blobCompression << endl;
      }
      
      // blob starts after '>\n\0'
      // create new Blob object
      
      const char *blobStart = attrEnd + 2;
      Blob *blob = new Blob(blobId, _params.debug >= Params::DEBUG_VERBOSE);
      if (blob->loadData(blobSize, blobCompression, blobStart)) {
        delete blob;
        return -1;
      }
      _blobs.push_back(blob);
      
      const char *blobEnd = blobStart + blobSize;

      if (strncmp(blobEnd, endTok, 7) == 0) {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
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



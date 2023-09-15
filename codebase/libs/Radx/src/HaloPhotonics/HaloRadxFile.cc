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
// HaloRadxFile.cc
//
// Halo Photonics format for lidar (radial ?)  data
//
// Brenda Javornik, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Code is based on the LeoRadxFile.cc code developed
// by Mike Dixon.
// Code development also based on ideas from actris-cloudnet/halo-reader python code
//
// June 2023
//
///////////////////////////////////////////////////////////////

#include <Radx/NcfRadxFile.hh>
#include <Radx/HaloRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxBuf.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxXml.hh>
#include <Radx/RadxStr.hh>
#include <cstring>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <string>
using namespace std;

//////////////
// Constructor

HaloRadxFile::HaloRadxFile() : RadxFile()
  
{

  _configFileName = "Config_AP.ini";
  _readVol = NULL;
  _file = NULL;
  clear();

  _latitude = 0.0;
  _longitude = 0.0;
  _altitudeM = 0.0;

  _wavelengthM = 0.0;
  _nyquist = 0.0;
  
  _nSamples = 0;

}

/////////////
// destructor

HaloRadxFile::~HaloRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void HaloRadxFile::clear()
  
{

  clearErrStr();
  _close();
  _ranges.clear();
  _clearRays();
  _modelStr.clear();

}

void HaloRadxFile::_clearRays()
{
  for (int ii = 0; ii < (int) _rays.size(); ii++) {
    delete _rays[ii];
  }
  _rays.clear();
}

void HaloRadxFile::_clearHeaderData() {
  _headingAngle = 0.0;
  _directionOffset = 0.0;
}

/////////////////////////////////////////////////////////
// Check if specified file is HaloPhotonics format
// Returns true if supported, false otherwise

bool HaloRadxFile::isSupported(const string &path)

{
  
  if (isHaloPhotonics(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a HaloPhotonics file
// Returns true on success, false on failure

bool HaloRadxFile::isHaloPhotonics(const string &path)
  
{

  _close();
  
  // open file
  
  if (_openRead(path)) {
    _addErrStr("ERROR - HaloRadxFile::isHaloPhotonics");
    return false;
  }
  
  // identify by file name extension
  size_t idx = path.find_last_of(".");
  if (idx != string::npos) {
    string extension = path.substr(idx+1, 3);
    if (extension.compare("hpl") == 0) {
      return true;
    } else {
      _close();
      return false;
    }
  } else {
    _close();
    return false;    
  }

  /* read first line

  char line[128];
  if (fgets(line, 128, _file) == NULL) {
    _close();
    return false;
  }
  _close();
  
  if ((strncmp(line, "HeaderSize", 10) == 0) || (strncmp(line, "HeaderLength", 10) == 0)) {
    return true;
  }
  if (path.find("WLS100") != string::npos) {
    return true;
  }
  */

  return false;

}

void HaloRadxFile::_getRayQualifiers(unordered_map<string, string> &dictionary) {

  unordered_map<string, string>::iterator it = dictionary.find("Data line 1");
  if (it != dictionary.end()) {
    string columnLabel = it->second;
    _findRayQualifiers(columnLabel); 
  } else {
    std::cerr << "Data line1 ray meta data not found\n";
  } 
}

void HaloRadxFile::_getFields(unordered_map<string, string> &dictionary) {

  unordered_map<string, string>::iterator it = dictionary.find("Data line 2");
  if (it != dictionary.end()) {
    string columnLabel = it->second;
    _findFields(columnLabel); 
  } else {
    std::cerr << "Data line2 field labels not found\n";
  } 
}

int HaloRadxFile::_getNGates(unordered_map<string, string> &dictionary) {

  int nGates = 0;
  unordered_map<string, string>::iterator it = dictionary.find("Number of gates");
  if (it != dictionary.end()) {
    string nGatesValue = it->second;
    nGates = atoi(nGatesValue.c_str());
  } else {
    std::cerr << "Number of gates not found\n";
    return 0;
  } 
  return nGates;
}

int HaloRadxFile::_getNRays(unordered_map<string, string> &dictionary) {

  int nRays = 0;
  unordered_map<string, string>::iterator it = dictionary.find("No. of rays in file");
  if (it != dictionary.end()) {
    string value = it->second;
    nRays = atoi(value.c_str());
  } else {
    std::cerr << "Number of rays not found\n";
    return 0;
  } 
  return nRays;
}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int HaloRadxFile::readFromPath(const string &path,
                              RadxVol &vol)
  
{

  _initForRead(path, vol);

  // is this a HaloPhotonics file?
  
  if (!isHaloPhotonics(_pathInUse)) {
    _addErrStr("ERROR - HaloRadxFile::readFromPath");
    _addErrStr("  Not a HaloPhotonics file: ", _pathInUse);
    return -1;
  }

  // initialize XML

  string platform = "HaloPhotonics_LIDAR";
  _configXml.clear();
  _configXml += RadxXml::writeStartTag(platform, 0);
  
  // read in config file and store as xml ????
  
  //if (_loadConfigXml(_pathInUse)) {
  //  if (_debug) {
  //    cerr << "WARNING - no Config_AP.ini file" << endl;
  //  }
  //}
  
  // open data file
  
  if (_openRead(_pathInUse)) {
    _addErrStr("ERROR - HaloRadxFile::readFromPath");
    return -1;
  }

  // read in header metadata
  // add to status xml

  _configXml += RadxXml::writeStartTag("Header", 1);
  if (_readHeaderData(_configXml)) {
    _addErrStr("ERROR - HaloRadxFile::readFromPath");
    _close();
    return -1;
  }

  _modelID = identifyModel(_rawHeaderInfo);

  _configXml += RadxXml::writeEndTag("Header", 1);

  // end XML data

  _configXml += RadxXml::writeEndTag(platform, 0);

  if (_debug) {
    cerr << "====================== config XML =============================" << endl;
    cerr << _configXml;
    cerr << "===============================================================" << endl;
    cerr << "nRanges: " << _ranges.size() << endl;
  }
  
  // set status from config XML

  _setStatusFromXml(_configXml);

  // set angles from xml

  _setAnglesFromXml(_configXml);

  _getRayQualifiers(_rawHeaderInfo); // parse Data line 1

  
  // sanity check - need at least 2 gates
/*
  if ((_ranges.size() < 2) && (_modelID != 100)) {
    _addErrStr("ERROR - HaloRadxFile::readFromPath");
    _addErrStr("  No range array (Altitudes) found");
    _addErrStr("  File: ", _pathInUse);
    _close();
    return -1;
  }
  
  // set gate geom
  if (_modelID != 100) {
    _startRangeKm = _ranges[0] / 1000.0;
    _gateSpacingKm = (_ranges[1] - _ranges[0]) / 1000.0;
  }
  */
  // get row index for time, el and az
  /*
  _timeStampIndex = findTimeStamp();
  _elevationIndex = findElevationAngle();
  _azimuthIndex = findAzimuthAngle();

  if (_timeStampIndex < 0) {
    _addErrStr("ERROR - HaloRadxFile::readFromPath");
    _addErrStr("  Cannot find TimeStamp in column headers");
    _addErrStr("  File: ", _pathInUse);
    _close();
    return -1;
  }

  if (_debug) {
    cerr << "  _modelStr: " << _modelStr << endl;
    cerr << "  _timeStampIndex: " << _timeStampIndex << endl;
    cerr << "  _elevationIndex: " << _elevationIndex << endl;
    cerr << "  _azimuthIndex: " << _azimuthIndex << endl;
  }
  */
  _getFields(_rawHeaderInfo);

  // if the flag is set to aggregate sweeps into a volume on read,
  // call the method to handle that

  //_readRayQualifiers(_rayQualifiers);

  int nGatesPerRay = _getNGates(_rawHeaderInfo);
  vector<Field>::iterator it;
  for (it = _fields.begin(); it != _fields.end(); ++ it) {
    it->data.resize(nGatesPerRay);
  }

  int nRaysInFile = _getNRays(_rawHeaderInfo);
  // this is useless; it is not always correct.

  // read in ray data
  int iret = _readRayData(nRaysInFile, nGatesPerRay);

  if (iret) {
    _addErrStr("ERROR - HaloRadxFile::readFromPath");
    _close();
    return -1;
  }

  // close file

  _close();

  if (_debug) {
    cerr << "End of file" << endl;
  }
  
  _readPaths.push_back(path);

  // load the data into the read volume

  if (_loadReadVolume()) {
    return -1;
  }

  // set the packing from the rays

  _readVol->setPackingFromRays();

  // set format as read
  
  //_fileFormat = FILE_FORMAT_HALO_Photonics;


  return 0;

}

////////////////////////////////////////////////////////////
// Read in metadata
// Returns 0 on success, -1 on failure

int HaloRadxFile::_readHeaderData(string &xml)
  
{

  // int nHeaderLines = 0;
  int nLinesRead = 0;

  _ranges.clear();
  _clearHeaderData();

  _rawHeaderInfo.reserve(20);

  // read through the header records
  // read each line of the header, throw it into a hash table,
  // get the values from the hash table upon request  
  char line[65536];
  bool done = false;
  while (!feof(_file) && !done) {
    
    // get next line

    if (fgets(line, 65536, _file) == NULL) {
      break;
    }

    nLinesRead += 1;

    string ll(_stripLine(line));

    // skip any comment lines ...
    if (ll.find("****") != string::npos) {
      done = true;
    }

    // look for data column labels array - this is the last
    // entry in the header
    // WLS7 has "Date" instead of "Timestamp" to start column labels
    // but let's look for the absence of an equals ("=") which indicates key/value pair
    // Darn, there are also lines in the WLS7 Header that don't have "=", so I'm adding
    // a check of the header length which corresponds to the number of lines read
    // for the header.


/*
    if ( (ll.find("Timestamp") != string::npos) || 
         ((ll.find("=") == string::npos) && (nLinesRead >= nHeaderLines)) ) {

      // data columns array
      
      _columnLabels.clear();
      vector<string> toks;
      RadxStr::tokenize(ll, "\t", toks);
      for (size_t ii = 0; ii < toks.size(); ii++) {
        _columnLabels.push_back(toks[ii]);
      }
      if (_debug) {
        cerr << "Data column labels:" << endl;
        for (size_t ii = 0; ii < _columnLabels.size(); ii++) {
          cerr << "  col num " << ii << ": " << _columnLabels[ii] << endl;
        }
      }
      // done with header metadata
      return 0;
    }
*/
    // find key/value pairs
    size_t equals = ll.find(":");
    if (equals == string::npos) {
      continue;
    }
    
    string tag = ll.substr(0, equals);
      //_substituteChar(ll.substr(0, equals), ' ', '_');
    string valStr = ll.substr(equals+1, string::npos);
      //_substituteChar(ll.substr(equals+1, string::npos), ' ', '_');
    
    _rawHeaderInfo.insert(std::make_pair(tag, valStr));

    if (_debug) {
      cerr << "  header tag, valStr: " << tag << ", " << valStr << endl;
    }
    xml += RadxXml::writeString(tag, 2, valStr);

  } // while
  
  if (_debug) {
    cout << "contents:\n";
    unordered_map<string, string>::iterator it;
    for(it = _rawHeaderInfo.begin(); it != _rawHeaderInfo.end(); ++it) 
        cout << " " << it->first << " => " << it->second << '\n';
  }
  
  return 0;

}

/*
void HaloRadxFile::processHeader() {
    
    processFileName();
    processSystemId();
    processNGates();
    processRange();
    processGateLength();
    processPulsesRay();
    processNRays();
    processScanType();
    processFocusRange();
    processStartTime();
    processResolution();
    processEquation();
    processDataLine1();
    processDataLine2();
}
*/
/*
    if (tag.compare("Filename") == 0) {
      _processFileName(valStr);
    } else if (tag.compare())

    if (tag.find("ID_System") == 0) {
      _modelStr = valStr;
    }

    // lat/lon

    if (tag.find("GPS") == 0) {
      // tokenize the value string
      vector<string> lltoks;
      RadxStr::tokenize(valStr, "_,:Latong", lltoks);
      for (size_t itok = 0; itok < lltoks.size(); itok++) {
        string lltok = lltoks[itok];
        if (lltok.find('N') != string::npos ||
            lltok.find('S') != string::npos) {
          double latval;
          char ns;
          if (sscanf(lltok.c_str(), "%lg%1c", &latval, &ns) == 2) {
            int latDeg = (int) (latval / 100.0);
            _latitude = latDeg + (latval - latDeg * 100.0) / 60.0;
            if (ns == 'S') {
              _latitude *= -1.0;
            }
          }
        } else if (lltok.find('E') != string::npos ||
                   lltok.find('W') != string::npos) {
          double lonval;
          char ew;
          if (sscanf(lltok.c_str(), "%lg%1c", &lonval, &ew) == 2) {
            int lonDeg = (int) (lonval / 100.0);
            _longitude = lonDeg + (lonval - lonDeg * 100.0) / 60.0;
            if (ew == 'W') {
              _longitude *= -1.0;
            }
          }
        }
      } // itok
      if (_debug) {
        cerr << "  latitude: " << _latitude << endl;
        cerr << "  longitude: " << _longitude << endl;
      }
    } // if ((tag.find("GPS")

    // number of samples

    if (tag.find("Nb_Pulses") != string::npos) {
      int nSamples;
      if (sscanf(valStr.c_str(), "%d", &nSamples) == 1) {
        _nSamples = nSamples;
        if (_debug) {
          cerr << "  nSamples: " << _nSamples << endl;
        }
      }
    }

    // wavelength

    if (tag.find("Wavelen") != string::npos) {
      double wavelengthNm;
      if (sscanf(valStr.c_str(), "%lg", &wavelengthNm) == 1) {
        _wavelengthM = wavelengthNm * 1.0e-9;
        if (_debug) {
          cerr << "  wavelengthM: " << _wavelengthM << endl;
        }
      }
    }

    // instrument name
    processSystemId();
    if (tag.find("ID_System") != string::npos) {
      _instrumentName = valStr;
      if (_debug) {
        cerr << "  instrumentName: " << _instrumentName << endl;
      }
    }

    // site name

    if (tag.find("Localisation") != string::npos) {
      _siteName = valStr;
      if (_debug) {
        cerr << "  siteName: " << _siteName << endl;
      }
    }
    
    // range array

    if (tag.find("Altitudes") != string::npos) {
      vector<string> toks;
      RadxStr::tokenize(valStr, "\t", toks);
      for (size_t ii = 0; ii < toks.size(); ii++) {
        double dval;
        if (sscanf(toks[ii].c_str(), "%lg", &dval) == 1) {
          _ranges.push_back(dval);
        }
      }
      if (_ranges.size() != toks.size()) {
        _addErrStr("ERROR - HaloRadxFile::_readHeaderData");
        _addErrStr("  Bad range array: ", valStr);
        return -1;
      }
      if (_debug) {
        cerr << "Range array (m):";
        for (size_t ii = 0; ii < _ranges.size(); ii++) {
          cerr << " " << _ranges[ii];
        }
        cerr << endl;
      }
    }

    if (tag.find("Header") != string::npos) {
      nHeaderLines = stoi(valStr);
    }

    if (tag.find("DirectionOffset") != string::npos) {
      double dval;
      if (sscanf(valStr.c_str(), "%lg", &dval) == 1) {
        _directionOffset = dval;
        if (_debug) {
          cerr << "  direction offset: " << _directionOffset << endl;
        }
      }
    }

    if (tag.find("HeadingAngle") != string::npos) {
      double dval;
      if (sscanf(valStr.c_str(), "%lg", &dval) == 1) {
        _headingAngle = dval;
        if (_debug) {
          cerr << "  heading angle: " << _headingAngle << endl;
        }
      }
    }

}
*/

/*
Field *HaloRadxFile::_makeField() {

      Field *field = new Field();
      field->label = label;
      field->origName = origName;
      field->longName = _substituteChar(origName, ' ', '-');
      field.units = units;
      field.name = field.longName;
      field.standardName = field.longName;

      if (origName.find("Radial Wind Speed Dispersion") != string::npos) {
        field.name = "DISP";
      } else if (origName.find("Radial Wind Speed") != string::npos) {
        field.name = "VEL";
        field.standardName = "radial_velocity_of_scatterers_away_from_instrument";
        field.folds = true;
      } else if (origName.find("Carrier To Noise Ratio") != string::npos) {
        field.name = "CNR";
        field.standardName = "carrier_to_noise_ratio";
      } else if (origName.find("Wind Speed") != string::npos) {
        field.name = "WSPD";
      } else if (origName.find("Wind Direction") != string::npos) {
        field.name = "WDIR";
        field.units = "deg";
      } else if (origName.find("X-Wind") != string::npos) {
        field.standardName = "eastward_wind";
      } else if (origName.find("Y-Wind") != string::npos) {
        field.standardName = "northward_wind";
      } else if (origName.find("Z-Wind") != string::npos) {
        field.standardName = "upward_wind";
      }
}
*/

void HaloRadxFile::_findRayQualifiers(string &columnLabel) {
  _rayQualifiers.clear();

  string ll(_stripLine(columnLabel.c_str()));
    
  // we cannot use a tokenizer because there are embedded spaces inside the units
  // we need to just parse it by hand and look for the left and right parentheses
  //vector<string> toks;
  //RadxStr::tokenize(ll, "\t ()", toks);
  size_t ii = 0;
  int columnIndex = 0;  
  bool done = false;
  // now follow this format  <name> (<units>)
  while (ii < ll.size() && !done) {

      // we are in the fields 
      string units;
      size_t lpos = ll.find('(', ii);
      size_t rpos = ll.find(')', ii);
      if ((lpos != string::npos) && (rpos != string::npos)) {
        units = ll.substr(lpos+1, (rpos) - (lpos+1));
      
        //string label = colLabel.substr(nameStart, colLabel.size() - nameStart);
        //string origName = colLabel.substr(nameStart, nameEnd - nameStart);
        //string units = colLabel.substr(unitsStart, unitsEnd - unitsStart);

        size_t startIndexOfName = ll.find_first_not_of(" ", ii);
        string name = ll.substr(startIndexOfName, (lpos-1) - startIndexOfName);

        //units = _stripLine(units.c_str());
        //name = _stripLine(name.c_str());

        Field field;
        field.label = name;
        field.origName = name;
        field.longName = name;
        field.units = units;
        field.name = name;
        field.standardName = name;
        field.index = columnIndex;

        _rayQualifiers.push_back(field);

        ii = rpos + 1;
        columnIndex += 1;
      } else {
        done = true;
      }

  } // ii

  if (_debug) {
    cerr << "RayQualifiers:" << endl;
    for (size_t ii = 0; ii < _rayQualifiers.size(); ii++) {
      const Field &field = _rayQualifiers[ii];
      cerr << "  Field: " << field.name << endl;
      cerr << "    label: " << field.label << endl;
      cerr << "    orig name: " << field.origName << endl;
      cerr << "    long name: " << field.longName << endl;
      cerr << "    standard name: " << field.standardName << endl;
      cerr << "    units: " << field.units << endl;
      cerr << "    folds: " << string(field.folds?"Y":"N") << endl;
      cerr << "    index: " << field.index << endl;
    } // ii
  }
}

/*
int HaloRadxFile::_readRayQualifiers() {
  return 0;
}
*/

////////////////////////////////////////////////////////////
// Set up the field names and units for model 200

void HaloRadxFile::_findFields(string &columnLabel)
{

  _fields.clear();

  string ll(_stripLine(columnLabel.c_str()));
    
  // we cannot use a tokenizer because there are embedded spaces inside the units
  // we need to just parse it by hand and look for the left and right parentheses
  //vector<string> toks;
  //RadxStr::tokenize(ll, "\t ()", toks);

  // loop through the column labels

  //double firstRange = _ranges[0];
  // bool foundFirstRange = false;
  string rangeGate = "Range Gate";
  size_t ii = ll.find(rangeGate);
  if (ii != string::npos) {
    // found it
    ii += rangeGate.size();
  } 
/*
  // read the range if possible; looking for "Range Gate"
  if (toks.size() > 2) {
    if ((toks[0].compare("Range") == 0) && (toks[1].compare("Gate") == 0)) {
      // this is the Range Gate
    }
  }
 */


  int columnIndex = 1;  
  bool done = false;
  // now follow this format  <field name> (<units>)
  while (ii < ll.size() && !done) {

      // we are in the fields
      string units;
      size_t lpos = ll.find('(', ii);
      size_t rpos = ll.find(')', ii);
      if ((lpos != string::npos) && (rpos != string::npos)) {
        units = ll.substr(lpos+1, (rpos) - (lpos+1));
      
        //string label = colLabel.substr(nameStart, colLabel.size() - nameStart);
        //string origName = colLabel.substr(nameStart, nameEnd - nameStart);
        //string units = colLabel.substr(unitsStart, unitsEnd - unitsStart);

        size_t startIndexOfName = ll.find_first_not_of(" ", ii);
        string name = ll.substr(startIndexOfName, (lpos-1) - startIndexOfName);
        _addField(name, units, columnIndex);
      } else {
        // there are no units
        units = "";
        lpos = rpos = ll.size();
        size_t startIndexOfName = ll.find_first_not_of(" ", ii);
        if (startIndexOfName != string::npos) {
          string name = ll.substr(startIndexOfName, (rpos) - startIndexOfName);
          _addField(name, units, columnIndex);
        }      
        done = true;
      }
      ii = rpos + 1;
      columnIndex += 1;

  } // ii

  if (_debug) {
    cerr << "Fields:" << endl;
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      const Field &field = _fields[ii];
      cerr << "  Field: " << field.name << endl;
      cerr << "    label: " << field.label << endl;
      cerr << "    orig name: " << field.origName << endl;
      cerr << "    long name: " << field.longName << endl;
      cerr << "    standard name: " << field.standardName << endl;
      cerr << "    units: " << field.units << endl;
      cerr << "    folds: " << string(field.folds?"Y":"N") << endl;
      cerr << "    index: " << field.index << endl;
    } // ii
  }

}


void HaloRadxFile::_addField(string &name, string &units, int columnIndex) {
  Field field;
  field.label = name;
  field.origName = name;
  field.longName = name;
  field.units = units;
  field.name = name;
  field.standardName = name;
  field.index = columnIndex;

  _fields.push_back(field);  
}

/*
void HaloRadxFile::_checkForFieldQualifier(string columnLabel, size_t columnIndex) {

  if (((int) columnIndex != _timeStampIndex) &&
      ((int) columnIndex != _azimuthIndex) &&
      ((int) columnIndex != _rangeColumnIndex) &&
      ((int) columnIndex != _elevationIndex)) {
    // assume it is a field qualifier -- a 1D variable for this ray

    Field field;
    field.label = columnLabel;
    field.origName = columnLabel;
    field.longName = columnLabel; // _substituteChar(origName, ' ', '-');
    field.units = "";
    field.name = field.longName;
    field.standardName = field.longName;
    //field.setIsRayQualifier(true);


    if (columnLabel.find("Temperature") != string::npos) {
      field.units = "degrees C";     
    } 


    if (origName.find("Radial Wind Speed Dispersion") != string::npos) {
      field.name = "DISP";
    } else if (origName.find("Radial Wind Speed") != string::npos) {
      field.name = "VEL";
      field.standardName = "radial_velocity_of_scatterers_away_from_instrument";
      field.folds = true;
    } else if (origName.find("CNR") != string::npos) {
      field.name = "CNR";
      field.standardName = "carrier_to_noise_ratio";
    } else if (origName.find("Wind Speed") != string::npos) {
      field.name = "WSPD";
    } else if (origName.find("Wind Direction") != string::npos) {
      field.name = "WDIR";
      field.units = "deg";
    } else if (origName.find("X-Wind") != string::npos) {
      field.standardName = "eastward_wind";
    } else if (origName.find("Y-Wind") != string::npos) {
      field.standardName = "northward_wind";
    } else if (origName.find("Z-Wind") != string::npos) {
      field.standardName = "upward_wind";
    }


  // set the indexes into the ray data

  //for (size_t ii = 0; ii < _fields.size(); ii++) {
  //  Field &field = _fields[ii];
    //for (size_t jj = 0; jj < _columnLabels.size(); jj++) {
    //  string search = "m ";
    //  search += field.label;
    //  if (_columnLabels[jj].find(search) != string::npos) {
        //field.index.push_back(columnIndex);
    //  } // jj
  //  }
  //}    
    //_fieldQualifiers.push_back(field);
  }
}

*/

bool HaloRadxFile::_contains(string &s1, string &s2) {
  if (s1.find(s2) != std::string::npos) {
    return true;
  } else {
    return false;
  }
}

Radx::SweepMode_t HaloRadxFile::decodeScanType(string &scanType) {

  //lower(scanType);
  switch (scanType[0]) {
  case 'S':
  case 's':  // stare
    return Radx::SWEEP_MODE_POINTING;
  // 'W':
  // 'w':
  // return Radx::SWEEP_MODE_WIND_PROFILE;
  default:
    return Radx::SWEEP_MODE_NOT_SET;
  }
  //if (_contains(scanType, "Stare")) {

  //}
        /* sweep mode

    SWEEP_MODE_NOT_SET = -1, ///< Initialized but not yet set
    SWEEP_MODE_CALIBRATION = 0, ///< pointing for calibration
    SWEEP_MODE_SECTOR = 1,     ///< sector scan mode
    SWEEP_MODE_COPLANE = 2,    ///< co-plane dual doppler mode
    SWEEP_MODE_RHI = 3,        ///< range height vertical scanning mode
    SWEEP_MODE_VERTICAL_POINTING = 4, ///< vertical pointing for calibration
    SWEEP_MODE_IDLE = 7,       ///< between scans
    SWEEP_MODE_AZIMUTH_SURVEILLANCE = 8, /**< 360-degree azimuth mode,
                                          * (surveillance) 
    SWEEP_MODE_ELEVATION_SURVEILLANCE = 9, /**< 360-degree elevation
                                            * mode (Eldora) 
    SWEEP_MODE_SUNSCAN = 11,   ///< scanning the sun for calibrations
  Stare ?=  SWEEP_MODE_POINTING = 12,  ///< fixed pointing
    SWEEP_MODE_FOLLOW_VEHICLE = 13, ///< follow target vehicle
    SWEEP_MODE_EL_SURV = 14, ///< elevation surveillance (ELDORA)
    SWEEP_MODE_MANUAL_PPI = 15, /**< Manual PPI mode - elevation does
                                 * not step automatically 
    SWEEP_MODE_MANUAL_RHI = 16, /**< Manual RHI mode - azimuth does
                                 * not step automatically 
    SWEEP_MODE_SUNSCAN_RHI = 17,  ///< scanning the sun in RHI mode
    SWEEP_MODE_DOPPLER_BEAM_SWINGING = 18, ///< as in profiler or lidar
    SWEEP_MODE_COMPLEX_TRAJECTORY = 19,  ///< any sequential angle sequence
    SWEEP_MODE_ELECTRONIC_STEERING = 20,  ///< as in phased array
    SWEEP_MODE_LAST
  } SweepMode_t;      
  */
}


// I am willing to bet that the order of this information is fixed to
// Decimal time (hours) Azimuth (degrees) Elevation (degrees) Pitch (degrees) Roll (degrees)
// So, let's assume this and modify it if needed.
void HaloRadxFile::decipherField(RadxRay *ray, Field &field, string value) {

  string Time("time");
  string Az("az");
  string El("el");
  if (_contains(field.name, Time)) {
    RadxTime rtime(value);
    ray->setTime(rtime.utime(), (int) (rtime.getSubSec() * 1.0e9 + 0.5));
  } else if (_contains(field.name, Az)) {
    double az = 0.0;
    az = atof(value.c_str());
    ray->setAzimuthDeg(az);      
  } else if (_contains(field.name, El)) {
    double el = 90.0;
    el = atof(value.c_str());
    if (el > 180) {
      el -= 360.0;
    }
    ray->setElevationDeg(el);
  }
}

int HaloRadxFile::_readRayQualifiers(RadxRay *ray, char *line) {

      int error = -1;
      string ll(_stripLine(line));
      vector<string> toks;
      RadxStr::tokenize(ll, "\t ", toks);
      // so, the _rayQualifiers have the position of the data in the token string
      // then, how to associate them? Ah, I can index the tokens directly,
      // so just go through the vector/list and pull out the tokens as
      // needed.
      if (toks.size() != _rayQualifiers.size()) {
        cerr << "Error: expected " << _rayQualifiers.size() << 
          " ray qualifiers, found " << toks.size() << endl;
        return error;
      }

      // Let's assume the token order is the same as the RayQualifier order

      string value;

      // time is in hours
      value = toks[0];
      double timeInHours = atof(value.c_str());
      printf("timeInHours %15.12f original string: %s\n", timeInHours, value.c_str());
      int minutesInHour = 60;  // also seconds in minute
      // int secondsInHour = minutesInHour * 60;
      int hours = int(timeInHours);
      double minutesf = timeInHours - hours;
      int minutes = int(minutesf * minutesInHour);
      double secondsf = minutesf - minutes;
      int seconds = int(secondsf * 60); // seconds in minute
      double subseconds = (secondsf - seconds) * 1.0e3;
      RadxTime rtime(hours, minutes, seconds, subseconds);
      cerr.precision(10);
      cerr << "Ray time: " << hours << ":" << minutes 
        << ":" << seconds << "." << subseconds << endl;  
      ray->setTime(rtime.utime(), subseconds);
      double reconstituted = hours + minutes/minutesInHour + seconds/(60*60) + subseconds/1.0e3;
      cerr << "Ray time reconstituted: " << reconstituted << " vs. original "
       << timeInHours << " diff = " << timeInHours - reconstituted << endl;

      value = toks[1];   
      double az = 0.0;
      az = atof(value.c_str());
      ray->setAzimuthDeg(az);      

      value = toks[2];
      double el = 90.0;
      el = atof(value.c_str());
      if (el > 180) {
        el -= 360.0;
      }
      ray->setElevationDeg(el);

      // TODO: get the pitch and the roll
      RadxGeoref georef;

      value = toks[3];
      double pitch = 0.0;
      pitch = atof(value.c_str());
      georef.setPitch(pitch);

      value = toks[4];
      double roll = 0.0;
      roll = atof(value.c_str());
      georef.setRoll(roll);

      ray->setGeoref(georef);            

      ray->setVolumeNumber(0);
      ray->setSweepNumber(0); 
/*
      size_t idx = 0;
      for (idx = 0; idx < _rayQualifiers.size(); idx++) {
        Field field = _rayQualifiers[idx];
        string value = toks[field.index];
        decipherField(ray, field, value);
      }
      */
    
      // fixed angle
      if (_fixedAngle > -9990) {
        ray->setFixedAngleDeg(_fixedAngle);
      } else {
        if (_rhiMode) {
          ray->setFixedAngleDeg(az);
        } else {
          ray->setFixedAngleDeg(el);
        }
      }

      // range geometry
      string label("Range gate length (m)");
      float gateSpacingKm = identify(label, _rawHeaderInfo) / 1000.0;
      string startRangeLabel("");
      float startRangeKm =  0.0;  // identify(startRangeLabel, _rawHeaderInfo) / 1000.0; 
      if (_verbose) {
        cout << "range geom: startRangeKm " << startRangeKm << 
          ", gateSpacingKm " << gateSpacingKm << endl;
      }
      ray->setRangeGeom(startRangeKm, gateSpacingKm);



      if (_rhiMode) {
        ray->setSweepMode(Radx::SWEEP_MODE_RHI);
      } else {
        if (_azLimit1 == _azLimit2) {
          ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
        } else {
          ray->setSweepMode(Radx::SWEEP_MODE_SECTOR);
        }
      }
      return !error;
}

// Read in ray data
// Returns 0 on success, -1 on failure

// the format is ...
// <ray qualifiers>
// <gate 0> <field 1 data> <field 2 data> ...
// <gate N-1> <field 1 data> <field 2 data>
// <ray qualifiers>
// <gate 0> <field 1 data> <field 2 data> ...
// <gate N-1> <field 1 data> <field 2 data>
// <ray qualifiers>
// <gate 0> <field 1 data> <field 2 data> ...
// <gate N-1> <field 1 data> <field 2 data>

int HaloRadxFile::_readRayData(int  nRaysInFile, size_t nGatesPerRay) {
  // int error = -1;
  int nRays = 0;
  RadxRay *ray = NULL;
  // read the values for the ray qualifiers
  char line[65536];
  while (!feof(_file)) { // } && (nRays < nRaysInFile)) {
    // get data columns array
    if (fgets(line, 65536, _file) != NULL) {
      cerr << "reading ray " << nRays << endl;
      ray = new RadxRay();
      ray->setNGates(nGatesPerRay);
      _readRayQualifiers(ray, line);
      _readRayData(ray, _fields);
      // add ray to vector
      _rays.push_back(ray);  
      cout << "in readRayData: nGates = " << ray->getNGates() << endl;
      nRays += 1;
    }
  } // while
  if (nRays != nRaysInFile) {
    cerr << "Not enough rays in file: expected " << endl;
  }
  return 0;
}


////////////////////////////////////////////////////////////
// Read in ray data
// Returns 0 on success, -1 on failure

int HaloRadxFile::_readRayData(RadxRay *ray, vector<Field> &fields) {

  int error = -1;
  if (_fields.size() <= 0) return error;
  int nGates = _fields[0].data.size();
  int igate = 0;
  char line[65536];
  do {
     // get next line
     char *cc = fgets(line, 65536, _file);
     _readNStoreFieldData(line, fields, igate);
     // get next line
     // cc = fgets(line, 65536, _file);
     //if (cc) {} // suppress compiler warning
     igate += 1;
  } while (!feof(_file) && (igate < nGates));
  //if (igate == nGates) {
    _fillRay(ray, fields);
  //}  
  return !error;
}

// move data stored in fields into the ray; copy the data 
// because we want to reuse the memory in the fields
//
void HaloRadxFile::_fillRay(RadxRay *ray, vector<Field> &fields) {
  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    Field &field = _fields[ifield];
    RadxField *rfld = new RadxField(field.name, field.units);
    rfld->setLongName(field.longName);
    rfld->setStandardName(field.standardName);
    rfld->setMissingFl32(Radx::missingFl32);
    rfld->setRangeGeom(ray->getStartRangeKm(), ray->getGateSpacingKm());
    size_t nGates = field.data.size();
    RadxArray<Radx::fl32> data_;
    Radx::fl32 *data = data_.alloc(nGates);
    for (size_t jj = 0; jj < nGates; jj++) {
      data[jj] = field.data.at(jj);
    }    
    rfld->addDataFl32(nGates, data);
    ray->addField(rfld); 
  } // ifield   
}

void HaloRadxFile::_readNStoreFieldData(char *line, vector<Field> &fields,
  int gateIdx) {
  if (line != NULL) {
    string ll(_stripLine(line));
    
    // data columns array
    vector<string> toks;
    RadxStr::tokenize(ll, "\t ", toks);

    for (size_t ifield = 0; ifield < fields.size(); ifield++) {
      Field &field = fields[ifield];
      string tok = toks[field.index];
      Radx::fl32 fval = Radx::missingFl32;
      if (tok.find("NaN") == string::npos) {
        fval = atof(tok.c_str());
      }
      field.data[gateIdx] = fval;
    } // ifield  
  }
}

void HaloRadxFile::_addRayQualifiers(RadxRay *ray, vector<string> &toks) {

      for (size_t ifield = 0; ifield < _rayQualifiers.size(); ifield++) {
      
      const Field &field = _rayQualifiers[ifield];
      
      //if (isFieldRequiredOnRead(field.name)) {
        
        RadxField *rfld = new RadxField(field.name, field.units);
        rfld->setLongName(field.longName);
        rfld->setStandardName(field.standardName);
        if (field.folds) {
          rfld->setFieldFolds(-_nyquist, _nyquist);
        }
        rfld->setMissingFl32(Radx::missingFl32);
        rfld->setRangeGeom(_startRangeKm, _gateSpacingKm);
        rfld->setIsRayQualifier(true);
        
        //size_t nGates = 5; //field.index.size();
        //RadxArray<Radx::fl32> data_;
        //Radx::fl32 *data = data_.alloc(nGates);
/*
        for (size_t jj = 0; jj < nGates; jj++) {
          string tok = toks[field.index[jj]];
          Radx::fl32 fval = Radx::missingFl32;
          if (tok.find("NaN") == string::npos) {
            fval = atof(tok.c_str());
          }
          data[jj] = fval;
        }
        
        */
        //rfld->addDataFl32(nGates, data);

        ray->addField(rfld);

      //} // if (isFieldRequiredOnRead(field.name) ...

    } // ifield
}


//////////////////////////////////////
// open netcdf file for reading
// Returns 0 on success, -1 on failure

int HaloRadxFile::_openRead(const string &path)
  
{

  _close();
  _file = fopen(path.c_str(), "r");
  
  // Check that constructor succeeded
  
  if (_file == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - HaloRadxFile::_openRead");
    _addErrStr("  Cannot open file for reading, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void HaloRadxFile::_close()
  
{
  
  // close file if open
  
  if (_file) {
    fclose(_file);
    _file = NULL;
  }
  
}

/////////////////////////////////////////////////////////
// load up the read volume with the data from this object

int HaloRadxFile::_loadReadVolume()
  
{

  int nRays = _rays.size();
  if (nRays < 1) {
    if (_debug) {
      cerr << "WARNING - HaloRadxFile::_loadReadVolume" << endl;
      cerr << "  No rays" << endl;
    }
    return -1;
  }
  
  _readVol->clear();

  // set meta-data
 
  _readVol->setOrigFormat("HaloPhotonics");
  _readVol->setInstrumentType(Radx::INSTRUMENT_TYPE_LIDAR);
  _readVol->setPlatformType(Radx::PLATFORM_TYPE_FIXED);
  
  _readVol->setStartTime(_rays[0]->getTimeSecs(),
                         _rays[0]->getNanoSecs());
  _readVol->setEndTime(_rays[nRays-1]->getTimeSecs(),
                       _rays[nRays-1]->getNanoSecs());

  _readVol->setTitle("HaloPhotonics LIDAR");
  _readVol->setSource(_instrumentName);
  size_t nameStart = _pathInUse.find_last_of(PATH_SEPARATOR);
  string fileName = _pathInUse;
  if (nameStart != string::npos) {
    fileName = _pathInUse.substr(nameStart + 1);
  }

  _readVol->setSiteName(_siteName);
  _readVol->setInstrumentName(_instrumentName);
  _readVol->setHistory(fileName);
  _readVol->setComment(_comments);
  _readVol->setStatusXml(_configXml);

  _readVol->setLatitudeDeg(_latitude);
  _readVol->setLongitudeDeg(_longitude);
  _readVol->setAltitudeKm(_altitudeM / 1000.0);
  _readVol->setSensorHtAglM(0);
  _readVol->addWavelengthM(_wavelengthM);

  // add rays

  for (int ii = 0; ii < (int) _rays.size(); ii++) {
    _readVol->addRay(_rays[ii]);
  }

  // memory allocation for rays has passed to _readVol,
  // so free up pointer array

  _rays.clear();
  
  // load the sweep information from the rays
  //_readVol->loadRayInfoFromFields();
  _readVol->loadSweepInfoFromRays();
  
  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - HaloRadxFile::_loadReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - HaloRadxFile::_loadReadVolume");
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

  return 0;
  
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

int HaloRadxFile::writeToDir(const RadxVol &vol,
                            const string &dir,
                            bool addDaySubDir,
                            bool addYearSubDir)
  
{

  // Writing HaloPhotonics files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - HaloRadxFile::writeToDir" << endl;
  cerr << "  Writing HaloPhotonics format files not supported" << endl;
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

  return iret;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified path
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs
// Use getPathInUse() for path written

int HaloRadxFile::writeToPath(const RadxVol &vol,
                             const string &path)
  
{

  // Writing HaloPhotonics files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - HaloRadxFile::writeToPath" << endl;
  cerr << "  Writing HaloPhotonics format files not supported" << endl;
  cerr << "  Will write CfRadial file instead" << endl;

  // set up NcfRadxFile object

  NcfRadxFile ncfFile;
  ncfFile.copyWriteDirectives(*this);

  // perform write

  int iret = ncfFile.writeToPath(vol, path);

  // set return values

  _errStr = ncfFile.getErrStr();
  _pathInUse = ncfFile.getPathInUse();

  return iret;

}

/////////////////////////////////////////////////////////
// print data after read

void HaloRadxFile::print(ostream &out) const
  
{
  
  out << "=============== HaloRadxFile ===============" << endl;
  RadxFile::print(out);

  out << "  instrumentName: " << _instrumentName << endl;
  out << "  siteName: " << _siteName << endl;
  out << "  comments: " << _comments << endl;
  out << "  latitude: " << _latitude << endl;
  out << "  longitude: " << _longitude << endl;
  out << "  altitudeM: " << _altitudeM << endl;
  out << "  wavelengthM: " << _wavelengthM << endl;
  out << "  nyquist: " << _nyquist << endl;
  out << "  nSamples: " << _nSamples << endl;
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in Halo Photonics file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int HaloRadxFile::printNative(const string &path, ostream &out,
                             bool printRays, bool printData)
  
{

  clear();

  // is this a HaloPhotonics file?
  
  if (!isHaloPhotonics(path)) {
    _addErrStr("ERROR - HaloRadxFile::printNative");
    _addErrStr("  Not a HaloPhotonics file: ", path);
    return -1;
  }

  // print config

  _printConfig(path, out);

  // open file
  
  if (_openRead(path)) {
    _addErrStr("ERROR - HaloRadxFile::printNative");
    return -1;
  }
  
  // read through the records in the file

  bool gotData = false;
  char line[65536];
  while (!feof(_file)) {
    if (fgets(line, 65536, _file) == NULL) {
      if (!gotData) {
        _addErrStr("ERROR - HaloRadxFile::printNative");
        _addErrStr("  Premature end of file: ", path);
        _close();
        return -1;
      }
    }
    out << line;
    if (strncmp(line, "Timestamp", 9) == 0) {
      gotData = true;
      if (!printRays && !printData) {
        break;
      }
    }
  } // while

  _close();
  return 0;

}

////////////////////////////////////////////////////////////////
// print the config file

int HaloRadxFile::_printConfig(const string &path, ostream &out)
  
{

  RadxPath rpath(path);
  string cpath(rpath.getDirectory());
  cpath += rpath.getDelimiter();
  cpath += _configFileName;

  FILE *cfile;
  if ((cfile = fopen(cpath.c_str(), "r")) == NULL) {
    out << "Warning - no config file in dir: " << rpath.getDirectory() << endl;
    return -1;
  }

  out << "==============================================================" << endl;
  out << "Config file: " << cpath << endl;

  char line[65536];
  while (!feof(cfile)) {
    if (fgets(line, 65536, cfile) == NULL) {
      break;
    }
    // strip new line
    if (line[strlen(line)-1] == '\n') {
      line[strlen(line)-1] = '\0';
    }
    // strip return
    if (line[strlen(line)-1] == '\r') {
      line[strlen(line)-1] = '\0';
    }
    out << line << endl;
  } // while
  out << "==============================================================" << endl;

  fclose(cfile);
  return 0;

}

////////////////////////////////////////////////////////////////
// read in the config and load up XML

int HaloRadxFile::_loadConfigXml(const string &path)
  
{

  RadxPath rpath(path);
  string cpath(rpath.getDirectory());
  cpath += rpath.getDelimiter();
  cpath += _configFileName;

  FILE *cfile;
  if ((cfile = fopen(cpath.c_str(), "r")) == NULL) {
    return -1;
  }

  char line[65536];
  string outerTag;
  while (!feof(cfile)) {

    // get next line
    if (fgets(line, 65536, cfile) == NULL) {
      break;
    }

    // strip new line
    if (line[strlen(line)-1] == '\n') {
      line[strlen(line)-1] = '\0';
    }

    // strip return
    if (line[strlen(line)-1] == '\r') {
      line[strlen(line)-1] = '\0';
    }
    string ll(line);

    size_t openSb = ll.find('[');
    size_t closeSb = ll.find(']');
    if (openSb != string::npos && closeSb != string::npos) {
      // find outer tags
      if (outerTag.size() > 0) {
        _configXml += RadxXml::writeEndTag(outerTag, 1);
      }
      int outerTagLen = closeSb - openSb - 1;
      outerTag = ll.substr(openSb + 1, outerTagLen);
      outerTag = _substituteChar(outerTag, ' ', '_');
      if (outerTag.size() > 0) {
        _configXml += RadxXml::writeStartTag(outerTag, 1);
      }
    } else {
      // find key/value pairs
      size_t equals = ll.find(" = ");
      if (equals != string::npos) {
        string innerTag =
          _substituteChar(ll.substr(0, equals), ' ', '_');
        string innerVal =
          _substituteChar(ll.substr(equals+3, string::npos), '"', '\0');
        _configXml += RadxXml::writeString(innerTag, 2, innerVal);
      }
    }
    
    if (ll.size() == 0) {
      if (outerTag.size() > 0) {
        _configXml += RadxXml::writeEndTag(outerTag, 1);
      }
      outerTag.clear();
    }

  } // while

  // close file

  fclose(cfile);

  // complete XML

  if (outerTag.size() > 0) {
    _configXml += RadxXml::writeEndTag(outerTag, 1);
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// set status from XML

void HaloRadxFile::_setStatusFromXml(const string &xml)
  
{

  // set some metadata from XML

  string sval;
  if (RadxXml::readString(xml, "ID_System", sval) == 0) {
    _instrumentName = sval;
  }
  if (RadxXml::readString(xml, "Localisation", sval) == 0) {
    _siteName = sval;
  }
  if (RadxXml::readString(xml, "Comments", sval) == 0) {
    _comments = sval;
  }

  int ival;
  if (RadxXml::readInt(xml, 
                       "Algorythm_Settings.Nb_Pulses/LOS", ival) == 0) {
    _nSamples = ival;
  }

  double dval;
  if (RadxXml::readDouble(xml, 
                          "Laser_Settings.Wavelength", dval) == 0) {
    _wavelengthM = dval * 1.0e-9;
  }
  if (RadxXml::readDouble(xml, 
                          "Filters_Settings.Max_Mesurable_Speed", dval) == 0) {
    _nyquist = dval;
  }

}

////////////////////////////////////////////////////////////////
// set angles from XML

void HaloRadxFile::_setAnglesFromXml(const string &xml)
  
{

/*
  _azLimit1 = NAN;
  _azLimit2 = NAN;
  _elLimit1 = NAN;
  _elLimit2 = NAN;

  RadxXml::readDouble(xml, "Azimuth_Angle_1_(°)", _azLimit1);
  RadxXml::readDouble(xml, "Azimuth_Angle_2_(°)", _azLimit2);
  RadxXml::readDouble(xml, "Elevation_Angle_1_(°)", _elLimit1);
  RadxXml::readDouble(xml, "Elevation_Angle_2_(°)", _elLimit2);

  _fixedAngle = -9999;
  _rhiMode = false;

  #if defined(PGI)
  if (!std::isnan(_elLimit1) && std::isnan(_elLimit2)) {
    _fixedAngle = _elLimit1;
  } else if (!std::isnan(_azLimit1) && std::isnan(_azLimit2)) {
    _fixedAngle = _azLimit1;
    _rhiMode = true;
  }
  #else
  if (!std::isnan(_elLimit1) && std::isnan(_elLimit2)) {
    _fixedAngle = _elLimit1;
  } else if (!std::isnan(_azLimit1) && std::isnan(_azLimit2)) {
    _fixedAngle = _azLimit1;
    _rhiMode = true;
  }
  #endif
  */
}

int HaloRadxFile::findTimeStamp() {
  int index = -1;
  size_t ii = 0;
  bool done = false;

  while ( (ii < _columnLabels.size()) && !done ) {
    if ((_columnLabels[ii].find("Timestamp") != string::npos) || 
     (_columnLabels[ii].find("Date") != string::npos)) {
      index = ii;
      done = true;
    }
    ii += 1;
  }
  return index;
}


int HaloRadxFile::findElevationAngle() {
  int index = -1;
  size_t ii = 0;
  bool done = false;

  while ( (ii < _columnLabels.size()) && !done ) {
    if (findElevationAngle(_columnLabels[ii])) {
      index = ii;
      done = true;
    }
    ii += 1;
  }
  return index;
}

int HaloRadxFile::findAzimuthAngle() {
  int index = -1;
  size_t ii = 0;
  bool done = false;

  while ( (ii < _columnLabels.size()) && !done ) {
    if (findAzimuthAngle(_columnLabels[ii])) {
      index = ii;
      done = true;
    }
    ii += 1;
  }
  return index;
}

bool HaloRadxFile::findElevationAngle(string &columnLabel) {
  bool found = false;
  if (_modelID == 70 || _modelID == 200) {
    found = columnLabel.find("Elevation Angle") != string::npos;
  } else if (_modelID == 100) {
    found = columnLabel.find("Elevation") != string::npos;
  } else if (_modelID == 7 || _modelID == 866) {
    found = columnLabel.find("Position") != string::npos;
  } 
  return found;  
}

bool HaloRadxFile::findAzimuthAngle(string &columnLabel) {
  bool found = false;
  if (_modelID == 70 || _modelID == 200) {
    found = columnLabel.find("Azimuth Angle") != string::npos;
  } else if (_modelID == 100) {
    found = columnLabel.find("Azimuth") != string::npos;
  } else if ((_modelID == 7) || (_modelID == 866)) {
    found =  columnLabel.find("Position") != string::npos;
  } 
  return found;  
}

int HaloRadxFile::identifyModel(unordered_map<string, string> &dictionary) {

  unordered_map<string, string>::iterator it = dictionary.find("System ID");
  if (it != dictionary.end()) {
    return stoi(it->second);
  } else {
    std::cerr << "System ID Not found\n";
    return 0;
  }
}

int HaloRadxFile::identify(string &label, unordered_map<string, string> &dictionary) {

  unordered_map<string, string>::iterator it = dictionary.find(label.c_str());
  if (it != dictionary.end()) {
    return stoi(it->second);
  } else {
    std::cerr << label << " Not found\n";
    return 0;
  }
}

/*
string &HaloRadxFile::identifyModel(unordered_map<string, string> &dictionary) {

  unordered_map<string, string>::iterator it = dictionary.find("System ID");
  if (it != dictionary.end()) {
    return stoi(it->second);
  } else {
    std::cerr << "System ID Not found\n";
    return 0;
  }
}
*/

/*
void HaloRadxFile::identifyScanType(const string &path) {

  if (path.find("WLS100") != string::npos) {
    _modelID = 100;
  } else {
    _modelID = 200;  // assume model 200; change to model 100 when finding fields, as needed
    if (modelStr.find("WLS70") != string::npos) {
      _modelID = 70;
    } else if (_modelStr.find("WLS7-") != string::npos) {
      _modelID = 7;
    } else if (_modelStr.find("WLS866") != string::npos) {
      _modelID = 866;
    } 
  }
  
}
*/
////////////////////////////////////////
// substitute cc for spaces in a string

string HaloRadxFile::_substituteChar(const string &source, char find, char replace)
  
{

  string target;
  for (size_t ii = 0; ii < source.size(); ii++) {
    if (source[ii] == find) {
      if (replace != '\0') {
        target += replace;
      }
    } else {
      target += source[ii];
    }
  }
  return target;

}


 // int op_increase (int i) { return ++i; }
//unsigned char HaloRadxFile::_lowerIt(unsigned char c) { return std::tolower(c); }

////////////////////////////////////////
// strip eol from line

string HaloRadxFile::_stripLine(const char *line)
  
{

  int lineLen = strlen(line);

  // strip new line

  if (line[lineLen-1] == '\n') {
    lineLen--;
  }
  
  // strip return

  if (line[lineLen-1] == '\r') {
    lineLen--;
  }
  
  string stripped(line, lineLen);

  //for (string::iterator it = stripped.begin(); it != stripped.end(); ++it) {
  //  *it = std::tolower(*it);
  //} 

  return stripped;

}

    

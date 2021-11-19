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
// LeoRadxFile.cc
//
// Leosphere format for lidar radial data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2013
//
///////////////////////////////////////////////////////////////

#include <Radx/NcfRadxFile.hh>
#include <Radx/LeoRadxFile.hh>
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
using namespace std;

//////////////
// Constructor

LeoRadxFile::LeoRadxFile() : RadxFile()
  
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

LeoRadxFile::~LeoRadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void LeoRadxFile::clear()
  
{

  clearErrStr();
  _close();
  _ranges.clear();
  _clearRays();
  _modelStr.clear();

}

void LeoRadxFile::_clearRays()
{
  for (int ii = 0; ii < (int) _rays.size(); ii++) {
    delete _rays[ii];
  }
  _rays.clear();
}

void LeoRadxFile::_clearHeaderData() {
  _headingAngle = 0.0;
  _directionOffset = 0.0;
}

/////////////////////////////////////////////////////////
// Check if specified file is Leosphere format
// Returns true if supported, false otherwise

bool LeoRadxFile::isSupported(const string &path)

{
  
  if (isLeosphere(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a Leosphere file
// Returns true on success, false on failure

bool LeoRadxFile::isLeosphere(const string &path)
  
{

  _close();
  
  // open file
  
  if (_openRead(path)) {
    _addErrStr("ERROR - LeoRadxFile::isLeosphere");
    return false;
  }
  
  // read first line

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

  return false;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int LeoRadxFile::readFromPath(const string &path,
                              RadxVol &vol)
  
{

  _initForRead(path, vol);

  // is this a Leosphere file?
  
  if (!isLeosphere(_pathInUse)) {
    _addErrStr("ERROR - LeoRadxFile::readFromPath");
    _addErrStr("  Not a leosphere file: ", _pathInUse);
    return -1;
  }

  // initialize XML

  string platform = "LEOSPHERE_LIDAR";
  _configXml.clear();
  _configXml += RadxXml::writeStartTag(platform, 0);
  
  // read in config file and store as xml
  
  if (_loadConfigXml(_pathInUse)) {
    if (_debug) {
      cerr << "WARNING - no Config_AP.ini file" << endl;
    }
  }
  
  // open data file
  
  if (_openRead(_pathInUse)) {
    _addErrStr("ERROR - LeoRadxFile::readFromPath");
    return -1;
  }

  // read in header metadata
  // add to status xml

  _configXml += RadxXml::writeStartTag("Header", 1);
  if (_readHeaderData(_configXml)) {
    _addErrStr("ERROR - LeoRadxFile::readFromPath");
    _close();
    return -1;
  }

  identifyModel(_modelStr, path);

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
  
  // sanity check - need at least 2 gates

  if ((_ranges.size() < 2) && (_modelNum != 100)) {
    _addErrStr("ERROR - LeoRadxFile::readFromPath");
    _addErrStr("  No range array (Altitudes) found");
    _addErrStr("  File: ", _pathInUse);
    _close();
    return -1;
  }
  
  // set gate geom
  if (_modelNum != 100) {
    _startRangeKm = _ranges[0] / 1000.0;
    _gateSpacingKm = (_ranges[1] - _ranges[0]) / 1000.0;
  }
  
  // get column index for time, el and az

  _timeStampIndex = findTimeStamp();
  _elevationIndex = findElevationAngle();
  _azimuthIndex = findAzimuthAngle();

  if (_timeStampIndex < 0) {
    _addErrStr("ERROR - LeoRadxFile::readFromPath");
    _addErrStr("  Cannot find TimeStamp in column headers");
    _addErrStr("  File: ", _pathInUse);
    _close();
    return -1;
  }

  //if (_modelStr.find("WLS200") != string::npos) {
  if (_modelNum == 200) {
    if (_elevationIndex < 0 || _azimuthIndex < 0) {
      _addErrStr("ERROR - LeoRadxFile::readFromPath");
      _addErrStr("  Cannot find Elevation or Azimuth in column headers");
      _addErrStr("  File: ", _pathInUse);
      _close();
      return -1;
    }
  }

  if (_debug) {
    cerr << "  _modelStr: " << _modelStr << endl;
    cerr << "  _timeStampIndex: " << _timeStampIndex << endl;
    cerr << "  _elevationIndex: " << _elevationIndex << endl;
    cerr << "  _azimuthIndex: " << _azimuthIndex << endl;
  }
  

  // set up field vector
  switch(_modelNum) {
    case 70:
      _findFieldsModel70();
      break;
    case 7:
      _findFieldsModel7();
      break;
    case 866:
      _findFieldsModel866();
      break;
    default:
      bool isWLS100 = _findFieldsModel100();
      if (isWLS100) {
        _modelNum = 100;
      } else {
        _findFieldsModel200();
      }
  }

  // if the flag is set to aggregate sweeps into a volume on read,
  // call the method to handle that

  // read in ray data

  int iret = 0;
  switch(_modelNum) {
    case 70:
      iret = _readRayDataModel70();
      break;
    case 7:
      _readRayDataModel7();
      break;
    case 866:
      _readRayDataModel866();
      break;
    case 100:
      iret = _readRayDataModel100();
      break;
    case 200:
      iret = _readRayDataModel200();
      break;
    default:
      _addErrStr("ERROR - unknown model number");
  }

  if (iret) {
    _addErrStr("ERROR - LeoRadxFile::readFromPath");
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
  
  _fileFormat = FILE_FORMAT_LEOSPHERE;

  return 0;

}

////////////////////////////////////////////////////////////
// Read in metadata
// Returns 0 on success, -1 on failure

int LeoRadxFile::_readHeaderData(string &xml)
  
{

  int nHeaderLines = 0;
  int nLinesRead = 0;

  _ranges.clear();
  _clearHeaderData();

  // read through the header records
  
  char line[65536];
  while (!feof(_file)) {
    
    // get next line

    if (fgets(line, 65536, _file) == NULL) {
      break;
    }

    nLinesRead += 1;

    string ll(_stripLine(line));

    // skip any comment lines ...
    if (ll.find("*****") != string::npos) continue;

    // look for data column labels array - this is the last
    // entry in the header
    // WLS7 has "Date" instead of "Timestamp" to start column labels
    // but let's look for the absence of an equals ("=") which indicates key/value pair
    // Darn, there are also lines in the WLS7 Header that don't have "=", so I'm adding
    // a check of the header length which corresponds to the number of lines read
    // for the header.

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

    // find key/value pairs
    size_t equals = ll.find("=");
    if (equals == string::npos) {
      continue;
    }
    
    string tag =
      _substituteChar(ll.substr(0, equals), ' ', '_');
    string valStr =
      _substituteChar(ll.substr(equals+1, string::npos), ' ', '_');
    
    if (_debug) {
      cerr << "  header tag, valStr: " << tag << ", " << valStr << endl;
    }
    xml += RadxXml::writeString(tag, 2, valStr);

    // model type

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
        _addErrStr("ERROR - LeoRadxFile::_readHeaderData");
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

  } // while
  
  if (_debug) {
    cerr << "Premature end of file" << endl;
  }
  
  return -1;

}

////////////////////////////////////////////////////////////
// Set up the field names and units for model 200

void LeoRadxFile::_findFieldsModel200()
  
{

  _fields.clear();

  // loop through the column labels

  double firstRange = _ranges[0];
  bool foundFirstRange = false;

  for (size_t ii = 0; ii < _columnLabels.size(); ii++) {
    
    // read the range if possible

    string colLabel = _columnLabels[ii];

    double range;
    if (sscanf(colLabel.c_str(), "%lg", &range) != 1) {
      continue;
    }

    if (!foundFirstRange) {
      // we are in the fields for the first range
      foundFirstRange = true;
    }
    
    if (foundFirstRange) {
      if (fabs(range - firstRange) > 0.001) {
        // done
        break;
      }
      // we are in the fields for the first range

      size_t nameStart = colLabel.find("m ");
      if (nameStart == string::npos) {
        // bad field
        continue;
      }
      nameStart += 2;

      size_t unitsStart = colLabel.find("(");
      if (unitsStart == string::npos) {
        // bad field
        continue;
      }
      size_t nameEnd = unitsStart - 1;
      unitsStart += 1;

      size_t unitsEnd = colLabel.find(")");
      if (unitsStart == string::npos) {
        // bad field
        continue;
      }

      string label = colLabel.substr(nameStart, colLabel.size() - nameStart);
      string origName = colLabel.substr(nameStart, nameEnd - nameStart);
      string units = colLabel.substr(unitsStart, unitsEnd - unitsStart);

      Field field;
      field.label = label;
      field.origName = origName;
      field.longName = _substituteChar(origName, ' ', '-');
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

      _fields.push_back(field);

    }

  } // ii

  // add in the indexes into the ray data

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    Field &field = _fields[ii];
    for (size_t jj = 0; jj < _columnLabels.size(); jj++) {
      string search = "m ";
      search += field.label;
      if (_columnLabels[jj].find(search) != string::npos) {
        field.index.push_back(jj);
      } // jj
    }
  }

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
      cerr << "    nRanges: " << field.index.size() << endl;
      if (_verbose) {
        cerr << "    indexes:";
        for (size_t kk = 0; kk < field.index.size(); kk++) {
          cerr << " " << field.index[kk];
        }
        cerr << endl;
      }
    } // ii
  }

}


void LeoRadxFile::_checkForFieldQualifier(string columnLabel, size_t columnIndex) {

  if ((columnIndex != _timeStampIndex) && (columnIndex != _azimuthIndex)
    && (columnIndex != _rangeColumnIndex) && (columnIndex != _elevationIndex)) {
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

/*
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
*/

  // set the indexes into the ray data

  //for (size_t ii = 0; ii < _fields.size(); ii++) {
  //  Field &field = _fields[ii];
    //for (size_t jj = 0; jj < _columnLabels.size(); jj++) {
    //  string search = "m ";
    //  search += field.label;
    //  if (_columnLabels[jj].find(search) != string::npos) {
        field.index.push_back(columnIndex);
    //  } // jj
  //  }
  //}    
    _fieldQualifiers.push_back(field);
  }
}



////////////////////////////////////////////////////////////
// Set up the field names and units for model 866

void LeoRadxFile::_findFieldsModel866()
  
{

  _fields.clear();
  _fieldQualifiers.clear();

  // loop through the column labels

  double firstRange = _ranges[0];
  bool foundFirstRange = false;

  for (size_t ii = 0; ii < _columnLabels.size(); ii++) {
    
    // read the range if possible

    string colLabel = _columnLabels[ii];

    double range;
    if (sscanf(colLabel.c_str(), "%lg", &range) != 1) {
      _checkForFieldQualifier(colLabel, ii);
      continue;
    }

    if (!foundFirstRange) {
      // we are in the fields for the first range
      foundFirstRange = true;
    }
    
    if (foundFirstRange) {
      if (fabs(range - firstRange) > 0.001) {
        // done
        break;
      }
      // we are in the fields for the first range

      size_t nameStart = colLabel.find("m ");
      if (nameStart == string::npos) {
        // bad field
        continue;
      }
      nameStart += 2;

      size_t unitsStart = colLabel.find("(");
      if (unitsStart == string::npos) {
        // bad field
        continue;
      }
      size_t nameEnd = unitsStart - 1;
      unitsStart += 1;

      size_t unitsEnd = colLabel.find(")");
      if (unitsStart == string::npos) {
        // bad field
        continue;
      }

      string label = colLabel.substr(nameStart, colLabel.size() - nameStart);
      string origName = colLabel.substr(nameStart, nameEnd - nameStart);
      string units = colLabel.substr(unitsStart, unitsEnd - unitsStart);

      Field field;
      field.label = label;
      field.origName = origName;
      field.longName = _substituteChar(origName, ' ', '-');
      field.units = units;
      field.name = field.longName;
      field.standardName = field.longName;

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

      _fields.push_back(field);

    }

  } // ii

  // add in the indexes into the ray data

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    Field &field = _fields[ii];
    for (size_t jj = 0; jj < _columnLabels.size(); jj++) {
      string search = "m ";
      search += field.label;
      if (_columnLabels[jj].find(search) != string::npos) {
        field.index.push_back(jj);
      } // jj
    }
  }

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
      cerr << "    nRanges: " << field.index.size() << endl;
      if (_verbose) {
        cerr << "    indexes:";
        for (size_t kk = 0; kk < field.index.size(); kk++) {
          cerr << " " << field.index[kk];
        }
        cerr << endl;
      }
    } // ii
    _printFieldQualifiers();
  }

}

void LeoRadxFile::_printFieldQualifiers() {
    cerr << "Field Qualifiers:" << endl;
    for (size_t ii = 0; ii < _fieldQualifiers.size(); ii++) {
      const Field &field = _fieldQualifiers[ii];
      cerr << "  Field: " << field.name << endl;
      cerr << "    label: " << field.label << endl;
      cerr << "    orig name: " << field.origName << endl;
      cerr << "    long name: " << field.longName << endl;
      cerr << "    standard name: " << field.standardName << endl;
      cerr << "    units: " << field.units << endl;
      cerr << "    folds: " << string(field.folds?"Y":"N") << endl;
      cerr << "    nRanges: " << field.index.size() << endl;
      if (_verbose) {
        cerr << "    indexes:";
        for (size_t kk = 0; kk < field.index.size(); kk++) {
          cerr << " " << field.index[kk];
        }
        cerr << endl;
      }
    } // ii
}

////////////////////////////////////////////////////////////
// Set up the field names and units for model 70

void LeoRadxFile::_findFieldsModel70()
  
{

  _fields.clear();
  _fieldNames.clear();
  _fieldCols.clear();

  // loop through the column labels, finding the fields and
  // saving out their information

  for (size_t icol = 0; icol < _columnLabels.size(); icol++) {

    // read the range if possible

    string colLabel = _columnLabels[icol];

    // check for variable with name followed by range bin number

    size_t firstDigitPos = 0;
    for (size_t jj = 0; jj < colLabel.size(); jj++) {
      if (isdigit(colLabel[jj])) {
        firstDigitPos = jj;
        break;
      }
    }
    
    size_t lastDigitPos = 0;
    for (size_t jj = firstDigitPos; jj < colLabel.size(); jj++) {
      if (isdigit(colLabel[jj])) {
        lastDigitPos = jj;
      }
    }

    if ((firstDigitPos > 0) && (lastDigitPos == colLabel.size() - 1)) {
      // valid field
      string fieldName = colLabel.substr(0, firstDigitPos);
      _fieldNames.insert(fieldName); // adding to field set
      _fieldCols[colLabel] = icol; // adding to column map
    }

  } // icol

  if (_verbose) {
    map<string, size_t>::iterator it;
    for (it = _fieldCols.begin(); it != _fieldCols.end(); it++) {
      cerr << "  ==>> fieldName, colIndex: " 
           << it->first << ", " << it->second << endl;
    }
  }

  // loop through the field names set, creating fields for each one

  set<string>::iterator ifield;
  for (ifield = _fieldNames.begin(); ifield != _fieldNames.end(); ifield++) {
    
    string fieldName = *ifield;

    Field field;
    field.name = fieldName;
    
    if (fieldName.find("Vhm") != string::npos) {
      field.standardName = "Vhm";
      field.longName = "Vhm";
      field.units = "m/s";
    } else if (fieldName.find("dVh") != string::npos) {
      field.standardName = "dVh";
      field.longName = "dVh";
      field.units = "m/s";
    } else if (fieldName.find("VhMax") != string::npos) {
      field.standardName = "VhMax";
      field.longName = "VhMax";
      field.units = "m/s";
    } else if (fieldName.find("VhMin") != string::npos) {
      field.standardName = "VhMin";
      field.longName = "VhMin";
      field.units = "m/s";
    } else if (fieldName.find("Dir") != string::npos) {
      field.standardName = "Dir";
      field.longName = "Dir";
      field.units = "m/s";
    } else if (fieldName.find("um") != string::npos) {
      field.standardName = "um";
      field.longName = "um";
      field.units = "m/s";
    } else if (fieldName.find("du") != string::npos) {
      field.standardName = "du";
      field.longName = "du";
      field.units = "m/s";
    } else if (fieldName.find("vm") != string::npos) {
      field.standardName = "vm";
      field.longName = "vm";
      field.units = "m/s";
    } else if (fieldName.find("dv") != string::npos) {
      field.standardName = "dv";
      field.longName = "dv";
      field.units = "m/s";
    } else if (fieldName.find("wm") != string::npos) {
      field.standardName = "wm";
      field.longName = "wm";
      field.units = "m/s";
    } else if (fieldName.find("dw") != string::npos) {
      field.standardName = "dw";
      field.longName = "dw";
      field.units = "m/s";
    } else if (fieldName.find("CNRm") != string::npos) {
      field.standardName = "CNRm";
      field.longName = "CNRm";
      field.units = "m/s";
    } else if (fieldName.find("dCNR") != string::npos) {
      field.standardName = "dCNR";
      field.longName = "dCNR";
      field.units = "m/s";
    } else if (fieldName.find("CNRmax") != string::npos) {
      field.standardName = "CNRmax";
      field.longName = "CNRmax";
      field.units = "m/s";
    } else if (fieldName.find("CNRmin") != string::npos) {
      field.standardName = "CNRmin";
      field.longName = "CNRmin";
      field.units = "m/s";
    } else if (fieldName.find("sigmaFreqm") != string::npos) {
      field.standardName = "sigmaFreqm";
      field.longName = "sigmaFreqm";
      field.units = "m/s";
    } else if (fieldName.find("Avail") != string::npos) {
      field.standardName = "Avail";
      field.longName = "Avail";
      field.units = "m/s";
    }

    _fields.push_back(field);

  } // ifield
  
  // add in the indexes into the ray data
  
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    Field &field = _fields[ii];
    field.index.clear();
    for (size_t kk = 0; kk < _ranges.size(); kk++) {
      char searchName[1024];
      sprintf(searchName, "%s%d", field.name.c_str(), (int) kk + 1);
      for (size_t jj = 0; jj < _columnLabels.size(); jj++) {
        if (_columnLabels[jj] == searchName) {
          field.index.push_back(jj);
          break;
        }
      } // jj
    } // kk
  } // ii

  if (_debug) {
    cerr << "Fields:" << endl;
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      const Field &field = _fields[ii];
      cerr << "  Field: " << field.name << endl;
      cerr << "    label: " << field.label << endl;
      cerr << "    name: " << field.name << endl;
      cerr << "    long name: " << field.longName << endl;
      cerr << "    standard name: " << field.standardName << endl;
      cerr << "    units: " << field.units << endl;
      cerr << "    folds: " << string(field.folds?"Y":"N") << endl;
      cerr << "    nRanges: " << field.index.size() << endl;
      if (_verbose) {
        cerr << "    indexes:";
        for (size_t kk = 0; kk < field.index.size(); kk++) {
          cerr << " " << field.index[kk];
        }
        cerr << endl;
      }
    } // ii
  }

}

////////////////////////////////////////////////////////////
// Set up the field names and units for model 70

bool LeoRadxFile::_findFieldsModel100()
  
{
  bool isModel100 = true;

  _fields.clear();
  _fieldNames.clear();
  _fieldCols.clear();
  _fieldQualifiers.clear();

  // loop through the column labels, finding the fields and
  // saving their information
  // For WLS100, there is no header, each row of data contains the "address" of the data
  // the "address" is the timestamp, the azimuth, elevation, and the range gate.
  // There is a Range field in the column names.  Pull out this column
  // and use it to contruct the start range, and gate spacing.
  // There is no digit labeling which gate the field data belong.
  // Timestamp                   Configuration ID Scan ID LOS ID Azimuth [<F8>]  Elevation [<F8>] Range [m] RWS [m/s] DRWS [m/s] CNR [db]
  // 2017-02-11 12:00:00.303032      6             47      6      14.7331         74.9996         50        -0.456187  1.44906   -9.41139

  // just hardcoding the columns of data

  _rangeColumnIndex = 0;
  size_t lastFieldIndex = 0;

  for (size_t icol = 0; icol < _columnLabels.size(); icol++) {

    string colLabel = _columnLabels[icol];

    // assume all the data fields are listed after the Range column
    if (_rangeColumnIndex > 0) {

      // valid field


      size_t bracketPos = colLabel.find('[');

      // brackets appear with the field name
      //if (bracketPos == string::npos) {
      string fieldName = colLabel.substr(0, bracketPos-1);
 
        _fieldNames.insert(fieldName); // adding to field set
        _fieldCols[colLabel] = icol; // adding to column map

        // create field

        Field field;
        field.name = fieldName;
      
        if (fieldName.find("CNR") != string::npos) {
          field.standardName = "CNR";
          field.longName = "Carrier to Noise Ratio";
          field.units = "dB";
        } else if (fieldName.find("DRWS") != string::npos) {
          field.standardName = "Averaged Doppler Spectrum";
          field.longName = "Averaged Dopper Spectrum width converted";
          field.units = "m/s";
        } else if (fieldName.find("RWS") != string::npos) {
          field.standardName = "Radial Wind Speed";
          field.longName = "Radial Wind Speed";
          field.units = "m/s";
        } 

        // add the column index into the ray data
      
        field.index.clear();
        lastFieldIndex += 1;
        field.index.push_back(_rangeColumnIndex + lastFieldIndex);

        _fields.push_back(field);
      //}
    } else {
      _checkForFieldQualifier(colLabel, icol);
    }

    // read the range if possible
    if (colLabel.find("Range") != string::npos) {
      // This is our range column
      _rangeColumnIndex = 6;
    }

  } // icol

  if (_verbose) {
    map<string, size_t>::iterator it;
    for (it = _fieldCols.begin(); it != _fieldCols.end(); it++) {
      cerr << "  ==>> fieldName, colIndex: " 
           << it->first << ", " << it->second << endl;
    }
  }

  if (_rangeColumnIndex == 0) {
    isModel100 = false;
  } else {
  
    if (_debug) {
      cerr << "Fields:" << endl;
      for (size_t ii = 0; ii < _fields.size(); ii++) {
        const Field &field = _fields[ii];
        cerr << "  Field: " << field.name << endl;
        cerr << "    label: " << field.label << endl;
        cerr << "    name: " << field.name << endl;
        cerr << "    long name: " << field.longName << endl;
        cerr << "    standard name: " << field.standardName << endl;
        cerr << "    units: " << field.units << endl;
        cerr << "    folds: " << string(field.folds?"Y":"N") << endl;
        cerr << "    nRanges: " << field.index.size() << endl;
        if (_verbose) {
          cerr << "    indexes:";
          for (size_t kk = 0; kk < field.index.size(); kk++) {
            cerr << " " << field.index[kk];
          }
          cerr << endl;
        }
      } // ii
      _printFieldQualifiers();
    }
  }
  return isModel100;
}


////////////////////////////////////////////////////////////
// Set up the field names and units for model 7

void LeoRadxFile::_findFieldsModel7()
  
{

  _fields.clear();
  _fieldNames.clear();
  _fieldCols.clear();
  _fieldQualifiers.clear();

  // loop through the column labels, finding the fields and
  // saving out their information

  for (size_t icol = 0; icol < _columnLabels.size(); icol++) {

    // read the range if possible

    string colLabel = _columnLabels[icol];

    // check for variable with name followed by range bin number

    size_t firstDigitPos = 0;
    for (size_t jj = 0; jj < colLabel.size(); jj++) {
      if (isdigit(colLabel[jj])) {
        firstDigitPos = jj;
        break;
      }
    }
    
    size_t lastDigitPos = 0;
    for (size_t jj = firstDigitPos; jj < colLabel.size(); jj++) {
      if (isdigit(colLabel[jj])) {
        lastDigitPos = jj;
      }
    }
    
    /*
    size_t spaceDashPos = 0;
    size_t jj = 0;
    bool found = false;
    while ( jj < colLabel.size() && !found) {
      if ( (colLabel[jj] == ' ') || (colLabel[jj] == '-') ) {
        spaceDashPos = jj;
        found = true;
      }
      jj += 1;
    }
    */

    if ((firstDigitPos > 0) && (lastDigitPos == colLabel.size() - 1)) {
      // valid field
      string fieldName = colLabel.substr(0, firstDigitPos);
      _fieldNames.insert(fieldName); // adding to field set
      _fieldCols[colLabel] = icol; // adding to column map
    } else {
      _checkForFieldQualifier(colLabel, icol);
    }

  } // icol

  if (_verbose) {
    map<string, size_t>::iterator it;
    for (it = _fieldCols.begin(); it != _fieldCols.end(); it++) {
      cerr << "  ==>> fieldName, colIndex: " 
           << it->first << ", " << it->second << endl;
    }
  }

  // loop through the field names set, creating fields for each one

  set<string>::iterator ifield;
  for (ifield = _fieldNames.begin(); ifield != _fieldNames.end(); ifield++) {
    
    string fieldName = *ifield;

    Field field;
    field.name = fieldName;
    
    if (fieldName.find("Azi") != string::npos) {
      field.standardName = "Azi";
      field.longName = "averaged wind direction";
      field.units = "rd";
    } else if (fieldName.find("Vh") != string::npos) {
      field.standardName = "Vh";
      field.longName = "horizontal wind speed";
      field.units = "m/s";
    } else if (fieldName.find("u-") != string::npos) {
      field.standardName = "u";
      field.longName = "wind vector projection along x-axis";
      field.units = "m/s";
    } else if (fieldName.find("v-") != string::npos) {
      field.standardName = "v";
      field.longName = "wind vector projection along y-axis";
      field.units = "m/s";
    } else if (fieldName.find("w-") != string::npos) {
      field.standardName = "w";
      field.longName = "wind vector projection along z-axis";
      field.units = "m/s";
    } else if (fieldName.find("CNR") != string::npos) {
      field.standardName = "CNR";
      field.longName = "Carrier to Noise Ratio";
      field.units = "dB";
    } else if (fieldName.find("RWSD") != string::npos) {
      field.standardName = "Averaged Doppler Spectrum";
      field.longName = "Averaged Dopper Spectrum width converted";
      field.units = "m/s";
    } else if (fieldName.find("RWS") != string::npos) {
      field.standardName = "Radial Wind Speed";
      field.longName = "Radial Wind Speed";
      field.units = "m/s";
    }

    _fields.push_back(field);

  } // ifield
  
  // connect the field indexes into the ray data
  /*
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    Field &field = _fields[ii];
    field.index.clear();
    for (size_t kk = 0; kk < _ranges.size(); kk++) {
      char searchName[1024];
      sprintf(searchName, "%s%d", field.name.c_str(), (int) kk + 1);
      for (size_t jj = 0; jj < _columnLabels.size(); jj++) {
        if (_columnLabels[jj] == searchName) {
          field.index.push_back(jj);
          break;
        }
      } // jj
    } // kk
  } // ii
  */

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    Field &field = _fields[ii];
    field.index.clear();
  }

  for (size_t cli = 0; cli < _columnLabels.size(); cli++) {
    size_t ii = 0;
    bool found = false;
    while (ii < _fields.size() && !found) {
      // find the field in the column label
      Field &field = _fields[ii];
      if (_columnLabels[cli].find(field.name) != string::npos) {
        found = true;
        field.index.push_back(cli);
      }
      ii += 1;
    } 
  } // cli


  if (_debug) {
    cerr << "Fields:" << endl;
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      const Field &field = _fields[ii];
      cerr << "  Field: " << field.name << endl;
      cerr << "    label: " << field.label << endl;
      cerr << "    name: " << field.name << endl;
      cerr << "    long name: " << field.longName << endl;
      cerr << "    standard name: " << field.standardName << endl;
      cerr << "    units: " << field.units << endl;
      cerr << "    folds: " << string(field.folds?"Y":"N") << endl;
      cerr << "    nRanges: " << field.index.size() << endl;
      if (_verbose) {
        cerr << "    indexes:";
        for (size_t kk = 0; kk < field.index.size(); kk++) {
          cerr << " " << field.index[kk];
        }
        cerr << endl;
      }
    } // ii
    _printFieldQualifiers();
  }

}

////////////////////////////////////////////////////////////
// Read in ray data for model 200
// Returns 0 on success, -1 on failure

int LeoRadxFile::_readRayDataModel200()
  
{

  // read through the data records

  char line[65536];
  while (!feof(_file)) {
    
    // get next line
    
    if (fgets(line, 65536, _file) == NULL) {
      break;
    }
    string ll(_stripLine(line));
    
    // data columns array
    
    vector<string> toks;
    RadxStr::tokenize(ll, "\t", toks);
    
    // if (_verbose) {
    //   cerr << "====>>>>> Data values:" << endl;
    //   for (size_t ii = 0; ii < _columnLabels.size(); ii++) {
    //     cerr << "  col num " << ii << ", " << _columnLabels[ii] 
    //          << ": " << toks[ii] << endl;
    //   }
    //   cerr << "==================================" << endl;
    // }

    // new ray

    RadxRay *ray = new RadxRay;

    // time

    RadxTime rtime(toks[_timeStampIndex]);
    ray->setTime(rtime.utime(), (int) (rtime.getSubSec() * 1.0e9 + 0.5));

    ray->setVolumeNumber(0);
    ray->setSweepNumber(0);
    
    // elevation and azimuth

    double el = 90.0;
    if (_elevationIndex >= 0) {
      el = atof(toks[_elevationIndex].c_str());
      if (el > 180) {
        el -= 360.0;
      }
    }
    ray->setElevationDeg(el);

    double az = 0.0;
    if (_azimuthIndex >= 0) {
      az = atof(toks[_azimuthIndex].c_str());
    }
    ray->setAzimuthDeg(az);
    
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

    ray->setRangeGeom(_startRangeKm, _gateSpacingKm);

    // sweep mode

    if (_rhiMode) {
      ray->setSweepMode(Radx::SWEEP_MODE_RHI);
    } else {
      if (_azLimit1 == _azLimit2) {
        ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
      } else {
        ray->setSweepMode(Radx::SWEEP_MODE_SECTOR);
      }
    }
    
    // fields

    for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
      
      const Field &field = _fields[ifield];
      
      if (isFieldRequiredOnRead(field.name)) {
        
        RadxField *rfld = new RadxField(field.name, field.units);
        rfld->setLongName(field.longName);
        rfld->setStandardName(field.standardName);
        if (field.folds) {
          rfld->setFieldFolds(-_nyquist, _nyquist);
        }
        rfld->setMissingFl32(Radx::missingFl32);
        rfld->setRangeGeom(_startRangeKm, _gateSpacingKm);
        
        size_t nGates = field.index.size();
        RadxArray<Radx::fl32> data_;
        Radx::fl32 *data = data_.alloc(nGates);

        for (size_t jj = 0; jj < nGates; jj++) {
          string tok = toks[field.index[jj]];
          Radx::fl32 fval = Radx::missingFl32;
          if (tok.find("NaN") == string::npos) {
            fval = atof(tok.c_str());
          }
          data[jj] = fval;
        }
        
        rfld->addDataFl32(nGates, data);

        ray->addField(rfld);

      } // if (isFieldRequiredOnRead(field.name) ...

    } // ifield

    // add ray to vector

    _rays.push_back(ray);
    
  } // while
  
  return 0;

}

////////////////////////////////////////////////////////////
// Read in ray data for model 866
// Returns 0 on success, -1 on failure

int LeoRadxFile::_readRayDataModel866()
  
{

  // read through the data records

  char line[65536];
  while (!feof(_file)) {
    
    // get next line
    
    if (fgets(line, 65536, _file) == NULL) {
      break;
    }
    string ll(_stripLine(line));
    
    // data columns array
    
    vector<string> toks;
    RadxStr::tokenize(ll, "\t", toks);
    
    // if (_verbose) {
    //   cerr << "====>>>>> Data values:" << endl;
    //   for (size_t ii = 0; ii < _columnLabels.size(); ii++) {
    //     cerr << "  col num " << ii << ", " << _columnLabels[ii] 
    //          << ": " << toks[ii] << endl;
    //   }
    //   cerr << "==================================" << endl;
    // }

    // new ray

    RadxRay *ray = new RadxRay;

    // time

    RadxTime rtime(toks[_timeStampIndex]);
    ray->setTime(rtime.utime(), (int) (rtime.getSubSec() * 1.0e9 + 0.5));

    ray->setVolumeNumber(0);
    ray->setSweepNumber(0);
    
    // elevation and azimuth

    double el = 90.0;
    if (_elevationIndex >= 0) {
      el = atof(toks[_elevationIndex].c_str());
      if (el > 180) {
        el -= 360.0;
      }
    }
    ray->setElevationDeg(el);

    double az = 0.0;
    if (_azimuthIndex >= 0) {
      az = atof(toks[_azimuthIndex].c_str());
    }
    // azimuth is relative to instrument coordinate system
    // so, add the DirectionOffset
    az += _directionOffset;
    ray->setAzimuthDeg(az);
    
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

    ray->setRangeGeom(_startRangeKm, _gateSpacingKm);

    // sweep mode

    if (_rhiMode) {
      ray->setSweepMode(Radx::SWEEP_MODE_RHI);
    } else {
      if (_azLimit1 == _azLimit2) {
        ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
      } else {
        ray->setSweepMode(Radx::SWEEP_MODE_SECTOR);
      }
    }
    
    // field qualifiers
    // since field qualifiers are 1D, just grab the first column index
    //string tok = toks[field.index[0]];
    _addFieldQualifiers(ray, toks);

    /*
    for (size_t ifield = 0; ifield < _fieldQualifiers.size(); ifield++) {
      
      const Field &field = _fieldQualifiers[ifield];
      
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
        
        size_t nGates = field.index.size();
        RadxArray<Radx::fl32> data_;
        Radx::fl32 *data = data_.alloc(nGates);

        for (size_t jj = 0; jj < nGates; jj++) {
          string tok = toks[field.index[jj]];
          Radx::fl32 fval = Radx::missingFl32;
          if (tok.find("NaN") == string::npos) {
            fval = atof(tok.c_str());
          }
          data[jj] = fval;
        }
        
        rfld->addDataFl32(nGates, data);

        ray->addField(rfld);

      //} // if (isFieldRequiredOnRead(field.name) ...

    } // ifield
    */

    // fields

    for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
      
      const Field &field = _fields[ifield];
      
      if (isFieldRequiredOnRead(field.name)) {
        
        RadxField *rfld = new RadxField(field.name, field.units);
        rfld->setLongName(field.longName);
        rfld->setStandardName(field.standardName);
        if (field.folds) {
          rfld->setFieldFolds(-_nyquist, _nyquist);
        }
        rfld->setMissingFl32(Radx::missingFl32);
        rfld->setRangeGeom(_startRangeKm, _gateSpacingKm);
        
        size_t nGates = field.index.size();
        RadxArray<Radx::fl32> data_;
        Radx::fl32 *data = data_.alloc(nGates);

        for (size_t jj = 0; jj < nGates; jj++) {
          string tok = toks[field.index[jj]];
          Radx::fl32 fval = Radx::missingFl32;
          if (tok.find("NaN") == string::npos) {
            fval = atof(tok.c_str());
          }
          data[jj] = fval;
        }
        
        rfld->addDataFl32(nGates, data);

        ray->addField(rfld);

      } // if (isFieldRequiredOnRead(field.name) ...

    } // ifield

    // add ray to vector

    _rays.push_back(ray);
    
  } // while
  
  return 0;

}

void LeoRadxFile::_addFieldQualifiers(RadxRay *ray, vector<string> &toks) {

      for (size_t ifield = 0; ifield < _fieldQualifiers.size(); ifield++) {
      
      const Field &field = _fieldQualifiers[ifield];
      
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
        
        size_t nGates = field.index.size();
        RadxArray<Radx::fl32> data_;
        Radx::fl32 *data = data_.alloc(nGates);

        for (size_t jj = 0; jj < nGates; jj++) {
          string tok = toks[field.index[jj]];
          Radx::fl32 fval = Radx::missingFl32;
          if (tok.find("NaN") == string::npos) {
            fval = atof(tok.c_str());
          }
          data[jj] = fval;
        }
        
        rfld->addDataFl32(nGates, data);

        ray->addField(rfld);

      //} // if (isFieldRequiredOnRead(field.name) ...

    } // ifield
}

////////////////////////////////////////////////////////////
// Read in ray data for model 70
// Returns 0 on success, -1 on failure

int LeoRadxFile::_readRayDataModel70()
  
{

  // read through the data records

  char line[65536];
  while (!feof(_file)) {
    
    // get next line
    
    if (fgets(line, 65536, _file) == NULL) {
      break;
    }
    string ll(_stripLine(line));

    // substitute underscore in date string to make it a single token
    for (size_t ii = 0; ii < 16; ii++) {
      if (isspace(ll[ii])) {
        ll[ii] = '_';
      }
    }
    
    // data columns array
    
    vector<string> toks;
    RadxStr::tokenize(ll, " \t", toks);
    
    // if (_verbose) {
    //   cerr << "====>>>>> Data values:" << endl;
    //   for (size_t ii = 0; ii < _columnLabels.size(); ii++) {
    //     cerr << "  col num " << ii << ", " << _columnLabels[ii] 
    //          << ": " << toks[ii] << endl;
    //   }
    //   cerr << "==================================" << endl;
    // }

    // new ray

    RadxRay *ray = new RadxRay;

    // time
    
    int year, month, day, hour, min;
    char cc;
    if (sscanf(toks[_timeStampIndex].c_str(), "%2d/%2d/%4d%c%2d:%2d",
               &day, &month, &year, &cc, &hour, &min) == 6) {
      RadxTime rtime(year, month, day, hour, min, 0);
      ray->setTime(rtime.utime());
    } else {
      // cannot read date/time
      cerr << "Cannot read date/time, skipping" << endl;
      cerr << "  line: " << ll.substr(0, 60) << " ..... " << endl;
      continue;
    }

    ray->setVolumeNumber(0);
    ray->setSweepNumber(0);
    
    // elevation and azimuth

    double el = 90.0;
    if (_elevationIndex >= 0) {
      el = atof(toks[_elevationIndex].c_str());
      if (el > 180) {
        el -= 360.0;
      }
    }
    ray->setElevationDeg(el);

    double az = 0.0;
    if (_azimuthIndex >= 0) {
      az = atof(toks[_azimuthIndex].c_str());
    }
    ray->setAzimuthDeg(az);
    
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

    ray->setTrueScanRateDegPerSec(0);
    ray->setTargetScanRateDegPerSec(0);

    // range geometry

    ray->setRangeGeom(_startRangeKm, _gateSpacingKm);

    // sweep mode

    if (_rhiMode) {
      ray->setSweepMode(Radx::SWEEP_MODE_RHI);
    } else {
      if (_azLimit1 == _azLimit2) {
        ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
      } else {
        ray->setSweepMode(Radx::SWEEP_MODE_SECTOR);
      }
    }
    
    // fields

    for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
      
      const Field &field = _fields[ifield];
      
      if (isFieldRequiredOnRead(field.name)) {
        
        RadxField *rfld = new RadxField(field.name, field.units);
        rfld->setLongName(field.longName);
        rfld->setStandardName(field.standardName);
        if (field.folds) {
          rfld->setFieldFolds(-_nyquist, _nyquist);
        }
        rfld->setMissingFl32(Radx::missingFl32);
        rfld->setRangeGeom(_startRangeKm, _gateSpacingKm);
        
        size_t nGates = field.index.size();
        RadxArray<Radx::fl32> data_;
        Radx::fl32 *data = data_.alloc(nGates);

        for (size_t jj = 0; jj < nGates; jj++) {
          string tok = toks[field.index[jj]];
          Radx::fl32 fval = Radx::missingFl32;
          if (tok.find("NaN") == string::npos) {
            fval = atof(tok.c_str());
          }
          data[jj] = fval;
        }
        
        rfld->addDataFl32(nGates, data);

        ray->addField(rfld);

      } // if (isFieldRequiredOnRead(field.name) ...

    } // ifield

    // add ray to vector

    _rays.push_back(ray);
    
  } // while
  
  return 0;

}

////////////////////////////////////////////////////////////
// Read in ray data for model 100
// Returns 0 on success, -1 on failure

int LeoRadxFile::_readRayDataModel100()
  
{

  // read through the data records
  // there is one line for each gate. 
  // We don't know how many Ranges/Gates until we hit a new timestamp
  // while (!eof()) {
  //    if (oldTimestamp != newTimestamp)
  //       we are at the end of a ray,
  //    else 
  //       add another gate to the ray
  // }


  string oldTimeStamp("");
  char line[65536];
  RadxRay *ray = NULL;
  bool firstTime = true;
  Radx::fl32 previousRange = 0.0;

  while (!feof(_file)) {
    
    // get next line
    
    if (fgets(line, 65536, _file) == NULL) {
      break;
    }
    string ll(_stripLine(line));

    // substitute underscore in date string to make it a single token
    for (size_t ii = 0; ii < 16; ii++) {
      if (isspace(ll[ii])) {
        ll[ii] = '_';
      }
    }
    
    // data columns array
    
    vector<string> toks;
    RadxStr::tokenize(ll, " \t", toks);
    
    // if (_verbose) {
    //   cerr << "====>>>>> Data values:" << endl;
    //   for (size_t ii = 0; ii < _columnLabels.size(); ii++) {
    //     cerr << "  col num " << ii << ", " << _columnLabels[ii] 
    //          << ": " << toks[ii] << endl;
    //   }
    //   cerr << "==================================" << endl;
    // }

    string newTimeStamp = toks[_timeStampIndex];

    // time
    
    int year, month, day, hour, min, sec, nsec;
    char cc;
    if (sscanf(toks[_timeStampIndex].c_str(), "%4d-%2d-%2d%c%2d:%2d:%2d.%6d",
               &year, &month, &day, &cc, &hour, &min, &sec, &nsec) == 8) {
      //RadxTime rtime(year, month, day, hour, min, sec, nsec);
    } else {
      // cannot read date/time
      cerr << "Cannot read date/time, skipping" << endl;
      cerr << "  line: " << ll.substr(0, 60) << " ..... " << endl;
      continue;
    }

    if (oldTimeStamp.compare(newTimeStamp) != 0) {

      // we have a new ray

      if (!firstTime) {

        // save the accumulated data from the previous ray
        for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
        
          Field &field = _fields[ifield];

          if (isFieldRequiredOnRead(field.name)) {
            
            RadxField *rfld = new RadxField(field.name, field.units);
            rfld->setLongName(field.longName);
            rfld->setStandardName(field.standardName);
            if (field.folds) {
              rfld->setFieldFolds(-_nyquist, _nyquist);
            }
            rfld->setMissingFl32(Radx::missingFl32);   

            size_t nGates = field.data.size();
            RadxArray<Radx::fl32> data_;
            Radx::fl32 *data = data_.alloc(nGates);

            for (size_t jj = 0; jj < nGates; jj++) {
              data[jj] = field.data.at(jj);
            }

            //rfld->setRangeGeom(_startRangeKm, _gateSpacingKm);
            rfld->addDataFl32(nGates, data);
            ray->addField(rfld);
          } // if (isFieldRequiredOnRead(field.name) ...

          field.data.clear();
        }

        // add the previous ray to vector
        _rays.push_back(ray);
      }

      firstTime = false;

      // create a new ray
      ray = new RadxRay;
      RadxTime rtime(year, month, day, hour, min, sec, nsec);
      ray->setTime(rtime.utime());
      ray->setVolumeNumber(0);
      ray->setSweepNumber(0);

      string tok = toks[_rangeColumnIndex];
      Radx::fl32 fval = atof(tok.c_str());
      _startRangeKm = fval/1000.00;
      //previousRange = fval;

    }

    oldTimeStamp = newTimeStamp;
    
    // elevation and azimuth

    double el = 90.0;
    if (_elevationIndex >= 0) {
      el = atof(toks[_elevationIndex].c_str());
      if (el > 180) {
        el -= 360.0;
      }
    }
    ray->setElevationDeg(el);

    double az = 0.0;
    if (_azimuthIndex >= 0) {
      az = atof(toks[_azimuthIndex].c_str());
    }
    ray->setAzimuthDeg(az);
    
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

    ray->setTrueScanRateDegPerSec(0);
    ray->setTargetScanRateDegPerSec(0);

    // range geometry

    string tok = toks[_rangeColumnIndex];
    Radx::fl32 fval = atof(tok.c_str());
    _gateSpacingKm = (fval/1000.00 - previousRange);
    ray->setRangeGeom(_startRangeKm, _gateSpacingKm);
    previousRange = fval/1000.00;

    // sweep mode

    if (_rhiMode) {
      ray->setSweepMode(Radx::SWEEP_MODE_RHI);
    } else {
      if (_azLimit1 == _azLimit2) {
        ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
      } else {
        ray->setSweepMode(Radx::SWEEP_MODE_SECTOR);
      }
    }
    
    // field qualifiers
    // since field qualifiers are 1D, just grab the first column index
    //string tok = toks[field.index[0]];
    _addFieldQualifiers(ray, toks);

    // fields

    for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
      
      Field &field = _fields[ifield];
      
        //for (size_t jj = 0; jj < nGates; jj++) {
          string tok = toks[field.index[0]];
          Radx::fl32 fval = Radx::missingFl32;
          if (tok.find("NaN") == string::npos) {
            fval = atof(tok.c_str());
          }
          //data[jj] = fval;
        //}
        field.data.push_back(fval);

    } // ifield
    
  } // while
  
  return 0;

}

////////////////////////////////////////////////////////////
// Read in ray data for model 7
// Returns 0 on success, -1 on failure

int LeoRadxFile::_readRayDataModel7()
  
{

  // read through the data records

  char line[65536];
  while (!feof(_file)) {
    
    // get next line
    
    if (fgets(line, 65536, _file) == NULL) {
      break;
    }
    string ll(_stripLine(line));

    // substitute underscore in date string to make it a single token
    for (size_t ii = 0; ii < 16; ii++) {
      if (isspace(ll[ii])) {
        ll[ii] = '_';
      }
    }
    
    // data columns array
    
    vector<string> toks;
    RadxStr::tokenize(ll, " \t", toks);
    
    // if (_verbose) {
    //   cerr << "====>>>>> Data values:" << endl;
    //   for (size_t ii = 0; ii < _columnLabels.size(); ii++) {
    //     cerr << "  col num " << ii << ", " << _columnLabels[ii] 
    //          << ": " << toks[ii] << endl;
    //   }
    //   cerr << "==================================" << endl;
    // }

    // new ray

    RadxRay *ray = new RadxRay;

    // time
    
    int year, month, day, hour, min, sec, nsec;
    char cc;
    if (sscanf(toks[_timeStampIndex].c_str(), "%2d/%2d/%4d%c%2d:%2d:%2d.%2d",
               &day, &month, &year, &cc, &hour, &min, &sec, &nsec) == 8) {
      RadxTime rtime(year, month, day, hour, min, sec);
      ray->setTime(rtime.utime());
    } else {
      // cannot read date/time
      cerr << "Cannot read date/time, skipping" << endl;
      cerr << "  line: " << ll.substr(0, 60) << " ..... " << endl;
      continue;
    }

    ray->setVolumeNumber(0);
    ray->setSweepNumber(0);
    
    // elevation and azimuth

    double el = 90.0;
    if (_elevationIndex >= 0) {
      el = atof(toks[_elevationIndex].c_str());
      if (el > 180) {
        el -= 360.0;
      }
    }
    ray->setElevationDeg(el);

    double az = 0.0;
    if (_azimuthIndex >= 0) {
      az = atof(toks[_azimuthIndex].c_str());
    }
    // azimuth is relative to instrument coordinate system
    // so, add the DirectionOffset and HeadingAngle
    az += _directionOffset + _headingAngle;
    ray->setAzimuthDeg(az);
    
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

    ray->setTrueScanRateDegPerSec(0);
    ray->setTargetScanRateDegPerSec(0);

    // range geometry

    ray->setRangeGeom(_startRangeKm, _gateSpacingKm);

    // sweep mode

    if (_rhiMode) {
      ray->setSweepMode(Radx::SWEEP_MODE_RHI);
    } else {
      if (_azLimit1 == _azLimit2) {
        ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
      } else {
        ray->setSweepMode(Radx::SWEEP_MODE_SECTOR);
      }
    }
    
    // field qualifiers
    // since field qualifiers are 1D, just grab the first column index
    //string tok = toks[field.index[0]];
    _addFieldQualifiers(ray, toks);

    // fields

    for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
      
      const Field &field = _fields[ifield];
      
      if (isFieldRequiredOnRead(field.name)) {
        
        RadxField *rfld = new RadxField(field.name, field.units);
        rfld->setLongName(field.longName);
        rfld->setStandardName(field.standardName);
        if (field.folds) {
          rfld->setFieldFolds(-_nyquist, _nyquist);
        }
        rfld->setMissingFl32(Radx::missingFl32);
        rfld->setRangeGeom(_startRangeKm, _gateSpacingKm);
        
        size_t nGates = field.index.size();
        RadxArray<Radx::fl32> data_;
        Radx::fl32 *data = data_.alloc(nGates);

        for (size_t jj = 0; jj < nGates; jj++) {
          string tok = toks[field.index[jj]];
          Radx::fl32 fval = Radx::missingFl32;
          if (tok.find("NaN") == string::npos) {
            fval = atof(tok.c_str());
          }
          data[jj] = fval;
        }
        
        rfld->addDataFl32(nGates, data);

        ray->addField(rfld);

      } // if (isFieldRequiredOnRead(field.name) ...

    } // ifield

    // add ray to vector

    _rays.push_back(ray);
    
  } // while
  
  return 0;

}

//////////////////////////////////////
// open netcdf file for reading
// Returns 0 on success, -1 on failure

int LeoRadxFile::_openRead(const string &path)
  
{

  _close();
  _file = fopen(path.c_str(), "r");
  
  // Check that constructor succeeded
  
  if (_file == NULL) {
    int errNum = errno;
    _addErrStr("ERROR - LeoRadxFile::_openRead");
    _addErrStr("  Cannot open file for reading, path: ", path);
    _addErrStr("  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void LeoRadxFile::_close()
  
{
  
  // close file if open
  
  if (_file) {
    fclose(_file);
    _file = NULL;
  }
  
}

/////////////////////////////////////////////////////////
// load up the read volume with the data from this object

int LeoRadxFile::_loadReadVolume()
  
{

  int nRays = _rays.size();
  if (nRays < 1) {
    if (_debug) {
      cerr << "WARNING - LeoRadxFile::_loadReadVolume" << endl;
      cerr << "  No rays" << endl;
    }
    return -1;
  }
  
  _readVol->clear();

  // set meta-data
 
  _readVol->setOrigFormat("LEOSPHERE");
  _readVol->setInstrumentType(Radx::INSTRUMENT_TYPE_LIDAR);
  _readVol->setPlatformType(Radx::PLATFORM_TYPE_FIXED);
  
  _readVol->setStartTime(_rays[0]->getTimeSecs(),
                         _rays[0]->getNanoSecs());
  _readVol->setEndTime(_rays[nRays-1]->getTimeSecs(),
                       _rays[nRays-1]->getNanoSecs());

  _readVol->setTitle("LEOSPHERE LIDAR");
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

  _readVol->loadSweepInfoFromRays();
  
  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - LeoRadxFile::_loadReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - LeoRadxFile::_loadReadVolume");
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

int LeoRadxFile::writeToDir(const RadxVol &vol,
                            const string &dir,
                            bool addDaySubDir,
                            bool addYearSubDir)
  
{

  // Writing Leosphere files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - LeoRadxFile::writeToDir" << endl;
  cerr << "  Writing Leosphere format files not supported" << endl;
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

int LeoRadxFile::writeToPath(const RadxVol &vol,
                             const string &path)
  
{

  // Writing Leosphere files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - LeoRadxFile::writeToPath" << endl;
  cerr << "  Writing Leosphere format files not supported" << endl;
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

void LeoRadxFile::print(ostream &out) const
  
{
  
  out << "=============== LeoRadxFile ===============" << endl;
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
// Print native data in leosphere file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int LeoRadxFile::printNative(const string &path, ostream &out,
                             bool printRays, bool printData)
  
{

  clear();

  // is this a Leosphere file?
  
  if (!isLeosphere(path)) {
    _addErrStr("ERROR - LeoRadxFile::printNative");
    _addErrStr("  Not a leoshpere file: ", path);
    return -1;
  }

  // print config

  _printConfig(path, out);

  // open file
  
  if (_openRead(path)) {
    _addErrStr("ERROR - LeoRadxFile::printNative");
    return -1;
  }
  
  // read through the records in the file

  bool gotData = false;
  char line[65536];
  while (!feof(_file)) {
    if (fgets(line, 65536, _file) == NULL) {
      if (!gotData) {
        _addErrStr("ERROR - LeoRadxFile::printNative");
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

int LeoRadxFile::_printConfig(const string &path, ostream &out)
  
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

int LeoRadxFile::_loadConfigXml(const string &path)
  
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

void LeoRadxFile::_setStatusFromXml(const string &xml)
  
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

void LeoRadxFile::_setAnglesFromXml(const string &xml)
  
{

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
  if (!isnan(_elLimit1) && isnan(_elLimit2)) {
    _fixedAngle = _elLimit1;
  } else if (!isnan(_azLimit1) && isnan(_azLimit2)) {
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
  
}

int LeoRadxFile::findTimeStamp() {
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


int LeoRadxFile::findElevationAngle() {
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

int LeoRadxFile::findAzimuthAngle() {
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

bool LeoRadxFile::findElevationAngle(string &columnLabel) {
  bool found = false;
  if (_modelNum == 70 || _modelNum == 200) {
    found = columnLabel.find("Elevation Angle") != string::npos;
  } else if (_modelNum == 100) {
    found = columnLabel.find("Elevation") != string::npos;
  } else if (_modelNum == 7 || _modelNum == 866) {
    found = columnLabel.find("Position") != string::npos;
  } 
  return found;  
}

bool LeoRadxFile::findAzimuthAngle(string &columnLabel) {
  bool found = false;
  if (_modelNum == 70 || _modelNum == 200) {
    found = columnLabel.find("Azimuth Angle") != string::npos;
  } else if (_modelNum == 100) {
    found = columnLabel.find("Azimuth") != string::npos;
  } else if ((_modelNum == 7) || (_modelNum == 866)) {
    found =  columnLabel.find("Position") != string::npos;
  } 
  return found;  
}

void LeoRadxFile::identifyModel(string &modelStr, const string &path) {

  if (path.find("WLS100") != string::npos) {
    _modelNum = 100;
  } else {
    _modelNum = 200;  // assume model 200; change to model 100 when finding fields, as needed
    if (modelStr.find("WLS70") != string::npos) {
      _modelNum = 70;
    } else if (_modelStr.find("WLS7-") != string::npos) {
      _modelNum = 7;
    } else if (_modelStr.find("WLS866") != string::npos) {
      _modelNum = 866;
    } 
  }
}

////////////////////////////////////////
// substitute cc for spaces in a string

string LeoRadxFile::_substituteChar(const string &source, char find, char replace)
  
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

////////////////////////////////////////
// strip eol from line

string LeoRadxFile::_stripLine(const char *line)
  
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

  return stripped;

}

    

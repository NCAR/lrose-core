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
// InputFile.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2006
//
///////////////////////////////////////////////////////////////

#ifndef InputFile_HH
#define InputFile_HH

#include <string>
#include <vector>
#include <iostream>
#include <toolsa/TaXml.hh>
#include "Params.hh"
#include "Blob.hh"
#include "Tilt.hh"
using namespace std;

////////////////////////
// This class

class InputFile {
  
public:

  // constructor
  
  InputFile(const Params &params,
            const char *input_field_name,
            const char *output_field_name,
            const char *output_units,
            bool required);
  
  // destructor
  
  ~InputFile();

  // clear data

  void clear();

  // read file
  // returns 0 on success, -1 on failure
  
  int read(const string &path,
           const string &name,
           time_t time,
           int nn);
  
  // get methods

  const string &getInputFieldName() const { return _inputFieldName; }
  const string &getOutputFieldName() const { return _outputFieldName; }
  const string &getOutputUnits() const { return _outputUnits; }
  bool getRequired() const { return _required; }
  bool getFound() const { return _found; }
  const string &getPath() const { return _path; }
  const string &getName() const { return _name; }
  time_t getFileTime() const { return _fileTime; }
  int getNn() const { return _nn; }

  time_t getVolTime() const { return _volTime; }

  double getRadarAlt() const { return _radarAlt; }
  double getRadarLat() const { return _radarLat; }
  double getRadarLon() const { return _radarLon; }
  const string &getRadarId() const { return _radarId; }
  const string &getRadarName() const { return _radarName; }
  const string &getRadarType() const { return _radarType; }
  double getRadarWavelength() const { return _radarWavelength; }
  double getRadarBeamwidth() const { return _radarBeamwidth; }

  const string &getScanName() const { return _scanName; }
  time_t getScanTime() const { return _scanTime; }
  int getScanNumEle() const { return _scanNumEle; }
  double getScanFirstEle() const { return _scanFirstEle; }
  double getScanLastEle() const { return _scanLastEle; }
  const string &getPolarization() const { return _polarization; }
  const string &getPulseWidth() const { return _pulseWidth; }
  double getAntennaSpeed() const { return _antennaSpeed; }

  int getNTilts() const { return (int) _tilts.size(); }
  const vector<Tilt *> &getTilts() const { return _tilts; }
  int getNBlobs() const { return (int) _blobs.size(); }
  const vector<Blob *> &getBlobs() const { return _blobs; }

  // print
  
  void print(ostream &out) const;
  
protected:
  
private:

  const Params &_params;
  string _inputFieldName;
  string _outputFieldName;
  string _outputUnits;
  bool _required;

  bool _found;
  string _path;
  string _name;
  time_t _fileTime;
  int _nn;

  time_t _volTime;

  double _radarAlt;
  double _radarLat;
  double _radarLon;
  string _radarId;
  string _radarName;
  string _radarType;
  double _radarWavelength;
  double _radarBeamwidth;

  string _scanName;
  time_t _scanTime;
  int _scanNumEle;
  double _scanFirstEle;
  double _scanLastEle;
  string _polarization;
  string _pulseWidth;
  double _antennaSpeed;

  vector<Tilt *> _tilts;
  vector<Blob *> _blobs;

  int _decodeXml(const string &xmlBuf);
  
  int _decodeDateTime(const vector<TaXml::attribute> &attributes,
                      time_t &time);
  
  int _decodeBlobs(const char *fileBuf, int fileLen);

};

#endif


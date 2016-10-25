/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1999
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1999/03/14 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/////////////////////////////////////////////////////////////
// GemInputField.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
///////////////////////////////////////////////////////////////

#ifndef GemInputField_HH
#define GemInputField_HH

#include <string>
#include <vector>
#include <iostream>
#include <Radx/RadxXml.hh>
#include "GemBlob.hh"
#include "GemSweep.hh"
using namespace std;

////////////////////////
// This class

class GemInputField {
  
public:

  // constructor
  
  GemInputField(const string &fileName,
             const string &filePath,
             time_t fileTime,
             const string &fieldName,
             const string &standardName,
             const string &longName,
             const string &units,
             bool debug,
             bool verbose);
  
  // destructor
  
  ~GemInputField();

  // clear data

  void clear();

  // read file
  // returns 0 on success, -1 on failure
  
  int read();
  
  // get methods

  const string &getFileName() const { return _fileName; }
  const string &getFilePath() const { return _filePath; }
  const string &getFieldName() const { return _fieldName; }
  const string &getStandardName() const { return _standardName; }
  const string &getLongName() const { return _longName; }
  const string &getUnits() const { return _units; }

  time_t getFileTime() const { return _fileTime; }
  time_t getVolTime() const { return _volTime; }
  const string &getVolType() const { return _volType; }
  bool getIsRhi() const { return _isRhi; }
  bool getIsSector() const { return _isSector; }
  
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
  const double &getPulseWidthUs() const { return _pulseWidthUs; }
  double getAntennaSpeed() const { return _antennaSpeed; }

  int getNSweeps() const { return (int) _sweeps.size(); }
  const vector<GemSweep *> &getSweeps() const { return _sweeps; }
  int getNBlobs() const { return (int) _blobs.size(); }
  const vector<GemBlob *> &getGemBlobs() const { return _blobs; }

  const string &getXmlStr() const { return _xmlStr; }
  const string &getErrStr() const { return _errStr; }

  // print
  
  void print(ostream &out) const;
  
protected:
  
private:

  bool _debug;
  bool _verbose;
  string _xmlStr;
  string _errStr;
  
  string _fileName;
  string _filePath;

  string _fieldName;
  string _standardName;
  string _longName;
  string _units;
  
  time_t _fileTime;
  time_t _volTime;
  string _volType;

  double _startAzi;
  double _stopAzi;
  
  bool _isSector;
  bool _isRhi;

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
  double _pulseWidthUs;
  double _antennaSpeed;

  vector<GemSweep *> _sweeps;
  vector<GemBlob *> _blobs;

  int _decodeXml(const string &xmlBuf);
  
  int _decodeRadxTime(const vector<RadxXml::attribute> &attributes,
                      time_t &time);
  
  int _decodeBlobs(const char *fileBuf, int fileLen);

};

#endif


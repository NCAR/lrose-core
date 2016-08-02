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
// Tilt.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2006
//
///////////////////////////////////////////////////////////////

#ifndef Tilt_HH
#define Tilt_HH

#include <string>
#include <vector>
#include <iostream>
#include <dataport/bigend.h>
#include <toolsa/TaXml.hh>
#include "Params.hh"
class Blob;
using namespace std;

////////////////////////
// This class

class Tilt {
  
public:

  // constructor
  
  Tilt(const Params &params, int num);
  
  // copy constructor
  
  Tilt(const Tilt &orig, int num);
  
  // destructor
  
  ~Tilt();

  // clear data
  
  void clear();
  
  // set info from XML
  
  int decodeInfoXml(const string &xmlBuf);

  // set particular fields

  void setNGates(int nGates) { _nGates = nGates; }
  void setNSamples(int nSamples) { _nSamples = nSamples; }
  void setStartRange(double startRange) { _startRange = startRange; }
  void setGateSpacing(double gateSpacing) { _gateSpacing = gateSpacing; }
  void setPrf(double prf) { _prf = prf; }
  void setPulseWidth(double pulseWidth) { _pulseWidth = pulseWidth; }
  void setAntennaSpeed(double antennaSpeed) { _antennaSpeed = antennaSpeed; }

  void setRadarConst(double val) { _radarConst = val; }
  void setRadarConstH(double val) { _radarConstH = val; }
  void setRadarConstV(double val) { _radarConstV = val; }
  void setXmitPeakPowerKw(double val) { _xmitPeakPowerKw = val; }
  void setNoisePowerDbz(double val) { _noisePowerDbz = val; }

  int setAzAngles(const Blob &blob);
  int setFieldData(const Blob &blob);

  // get methods

  time_t getNum() const { return _num; }
  time_t getStartTime() const { return _startTime; }
  const string &getFieldName() const { return _fieldName; }

  int getNSamples() const { return _nSamples; }

  double getElev() const { return _elev; }
  int getNAz() const { return _nAz; }
  int getNGates() const { return _nGates; }
  double getStartRange() const { return _startRange; }
  double getGateSpacing() const { return _gateSpacing; }

  double getPrf() const { return _prf; }
  double getPulseWidth() { return _pulseWidth; }
  double getAntennaSpeed() const { return _antennaSpeed; }

  double getRadarConst() const { return _radarConst; }
  double getRadarConstH() const { return _radarConstH; }
  double getRadarConstV() const { return _radarConstV; }
  double getXmitPeakPowerKw() const { return _xmitPeakPowerKw; }
  double getNoisePowerDbz() const { return _noisePowerDbz; }

  double getMinValue() const { return _minValue; }
  double getMaxValue() const { return _maxValue; }

  int getDataBlobId() const { return _dataBlobId; }
  int getAnglesBlobId() const { return _anglesBlobId; }

  int getDataByteWidth() const { return _dataByteWidth; }
  int getAnglesByteWidth() const { return _anglesByteWidth; }
  
  const vector<double> &getAzAngles() const { return _azAngles; };
  const ui08 *getFieldData() const { return _fieldData; }

  // print
  
  void print(ostream &out) const;
  
protected:
  
private:

  const Params &_params;
  int _num;

  time_t _startTime;
  string _fieldName;

  double _elev;
  int _nAz;
  int _nSamples;
  int _nGates;
  double _startRange;
  double _gateSpacing;

  double _prf;
  double _pulseWidth;
  double _antennaSpeed;

  double _radarConst;
  double _radarConstH;
  double _radarConstV;
  double _xmitPeakPowerKw;
  double _noisePowerDbz;

  double _minValue;
  double _maxValue;
  
  int _dataBlobId;
  int _anglesBlobId;
  
  int _dataByteWidth;
  int _anglesByteWidth;

  vector<double> _azAngles;
  ui08 *_fieldData;

  int _decodeDateTime(const vector<TaXml::attribute> &attributes,
                      time_t &time);
  
};

#endif


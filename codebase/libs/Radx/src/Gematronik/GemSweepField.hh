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
// GemSweepField.hh
//
// Field for a given sweep
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
///////////////////////////////////////////////////////////////

#ifndef GemSweepField_HH
#define GemSweepField_HH

#include <string>
#include <vector>
#include <iostream>
#include <Radx/Radx.hh>
#include <Radx/RadxXml.hh>
class GemBlob;
using namespace std;

////////////////////////
// This class

class GemSweepField {
  
public:

  // constructor
  
  GemSweepField(int num, bool debug, bool verbose);
  
  // copy constructor
  
  GemSweepField(const GemSweepField &orig, int num, bool debug, bool verbose);
  
  // destructor
  
  ~GemSweepField();

  // clear data
  
  void clear();
  
  // set info from XML
  
  int decodeInfoXml(const string &xmlBuf);

  // set particular fields
  
  int setAngles(const GemBlob &blob);
  int setFieldData(const GemBlob &blob);

  // get methods

  int getVolNum() const { return _volNum; }
  int getSweepNum() const { return _sweepNum; }
  time_t getStartTime() const { return _startTime; }
  const string &getFieldName() const { return _fieldName; }

  size_t getNSamples() const { return _nSamples; }

  double getFixedAngle() const { return _fixedAngle; }
  size_t getNAngles() const { return _nAngles; }
  size_t getNGates() const { return _nGates; }
  double getStartRange() const { return _startRange; }
  double getGateSpacing() const { return _gateSpacing; }

  double getAngleRes() const { return _angleRes; }
  bool getIsIndexed() const { return _isIndexed; }

  double getPrf() const { return _prf; }
  double getHighPrf() const { return _highPrf; }
  double getLowPrf() const { return _lowPrf; }
  double getPrfRatio() const {
    if (_highPrf < 0 || _lowPrf < 0) {
      return Radx::missingMetaDouble;
    } else {
      return _highPrf / _lowPrf;
    }
  }
  bool getIsStaggered() const { return _isStaggered; }

  double getNyquist() const { return _nyquist; }
  int getPulseWidthIndex() const { return _pulseWidthIndex; }
  double getPulseWidthUs() const { return _pulseWidthUs; }
  double getAntennaSpeed() const { return _antennaSpeed; }

  double getRadarConst() const { return _radarConst; }
  double getRadarConstH() const { return _radarConstH; }
  double getRadarConstV() const { return _radarConstV; }

  bool getIsDualPol() const { return (_radarConstH > 0 && _radarConstV > 0); }

  double getXmitPeakPowerKw() const { return _xmitPeakPowerKw; }
  double getXmitPeakPowerKwH() const { return _xmitPeakPowerKwH; }
  double getXmitPeakPowerKwV() const { return _xmitPeakPowerKwV; }

  double getNoisePowerDbzH() const { return _noisePowerDbzH; }
  double getNoisePowerDbzV() const { return _noisePowerDbzV; }

  double getNoiseDbmH() const { return _noiseDbmH; }
  double getNoiseDbmV() const { return _noiseDbmV; }

  double getBeamWidthH() const { return _beamWidthH; }
  double getBeamWidthV() const { return _beamWidthV; }

  double getAntGainH() const { return _antGainH; }
  double getAntGainV() const { return _antGainV; }

  double getTxPathLossH() const { return _txPathLossH; }
  double getTxPathLossV() const { return _txPathLossV; }
  double getRxPathLossH() const { return _rxPathLossH; }
  double getRxPathLossV() const { return _rxPathLossV; }
  double getRadomeLoss() const { return _radomeLoss; }

  double getIfMhz() const { return _ifMhz; }

  double getMinValue() const { return _minValue; }
  double getMaxValue() const { return _maxValue; }

  int getDataBlobId() const { return _dataBlobId; }
  int getAnglesBlobId() const { return _anglesBlobId; }

  int getDataByteWidth() const { return _dataByteWidth; }
  int getAnglesByteWidth() const { return _anglesByteWidth; }
  
  const vector<double> &getAngles() const { return _angles; };
  const Radx::ui08 *getFieldData() const { return _fieldData; }

  // print
  
  void print(ostream &out) const;
  
protected:
  
private:

  bool _debug;
  bool _verbose;

  int _volNum;
  int _sweepNum;
  
  time_t _startTime;
  string _fieldName;

  double _fixedAngle;
  int _nAngles;
  vector<double> _angles;
  int _nSamples;
  int _nGates;
  double _startRange;
  double _gateSpacing;

  double _angleRes;
  bool _isIndexed;

  double _prf, _highPrf, _lowPrf;
  bool _isStaggered;

  double _nyquist;
  int _pulseWidthIndex;
  double _pulseWidthUs;
  double _antennaSpeed;

  double _radarConst;
  double _radarConstH;
  double _radarConstV;
  
  double _xmitPeakPowerKw;
  double _xmitPeakPowerKwH;
  double _xmitPeakPowerKwV;
  
  double _noisePowerDbzH;
  double _noisePowerDbzV;
  double _noiseDbmH;
  double _noiseDbmV;

  double _beamWidthH;
  double _beamWidthV;

  double _antGainH;
  double _antGainV;

  double _txPathLossH;
  double _txPathLossV;
  double _rxPathLossH;
  double _rxPathLossV;
  double _radomeLoss;

  double _ifMhz;

  double _minValue;
  double _maxValue;
  
  int _dataBlobId;
  int _anglesBlobId;
  
  int _dataByteWidth;
  int _anglesByteWidth;

  Radx::ui08 *_fieldData;

  vector<double> _pulseWidths;

  int _decodeDateTime(const vector<RadxXml::attribute> &attributes,
                      time_t &time);
  
  void _initPulseWidths();
  double _getPulseWidth(int index);
  double _getValFromList(const string &listStr, int index);
  int _parseList(const char *listStr, vector<double> &list);

};

#endif


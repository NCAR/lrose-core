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
// Ascii2Radx.hh
//
// Ascii2Radx object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2016
//
///////////////////////////////////////////////////////////////

#ifndef Ascii2Radx_HH
#define Ascii2Radx_HH

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <cstdio>
#include <Radx/Radx.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxField.hh>
class RadxVol;
class RadxFile;
class RadxRay;
class VarTransform;
using namespace std;

////////////////////////////////////////////////
// structs for ROS2 compressed data from ITALY

typedef struct {
 
  char signature[8]; /* string "ROS2_V" (ROS2 Volume) */
  Radx::si64 date; /* unix timestamp */
  Radx::fl32 rad_lat; /* radar latitude */
  Radx::fl32 rad_lon; /* radar latitude */
  Radx::fl32 rad_alt; /* radar latitude */
 
  char name[64]; /* name of scan (if available) */
  char type[8]; /* type of scan (CW=0, CCW=1, EL=2, AZ=3) */
  Radx::fl32 PRF; /* pulse repetition frequency */
  Radx::si32 l_pulse; /* length of pulse [nanosec] */
  Radx::fl32 l_bin; /* length of range bin [m] */
 
  Radx::fl32 nyquist_v; /* nyquist (unabiguous) velocity [m/s] */
  Radx::fl32 freq ; /* transmitted frequency [megahertz] */
 
  char Z; /* reflectivity */
  char Z_pos; /* position of Z field in the radar beam */
  char D; /* differential reflectivity */
  char D_pos; /* position of D field in the radar beam */
  char P; /* differential phase shift */
  char P_pos; /* position of P field in the radar beam */
  char R; /* rho */
  char R_pos; /* position of R field in the radar beam */
  char L; /* linear depolarization ratio */
  char L_pos; /* position of L field in the radar beam */
  char V; /* doppler velocity */
  char V_pos; /* position of V field in the radar beam */
  char S; /* spread of doppler velocity */
  char S_pos; /* position of S field in the radar beam */
  char IQ; /* IQ data */
  char IQ_pos; /* position of IQ field in the radar beam */
  char uZ; /* uncorrected reflectivity */
  char uZ_pos; /* position of uZ field in the radar beam */
  char uV; /* uncorrected doppler velocity */
  char uV_pos; /* position of uV field in the radar beam */
  char uS; /* uncorrected spread of doppler velocity */
  char uS_pos; /* position of uS field in the radar beam */
  char PART; /* particle */
  char PART_pos; /* position of PART field in the radar beam */
  char RR; /* rain (polarimetric relationship) */
  char RR_pos; /* position of RR field in the radar beam */
 
  Radx::fl32 tz_height; /* thermal zero height (NaN=not available) */
 
  char K; /* Kdp, specific differential phase */
  char K_pos; /* position of Kdp field in the radar beam */
 
  char spare[510]; /* for future implementations */

} ros2_vol_hdr_t;

typedef struct {
  Radx::si16 sweep; /* sweep_id (0 - n). Used as EOV flag if set to -1 */
  double time; /* UNIX timestamp */
  Radx::fl32 el;
  Radx::fl32 az;
  Radx::si16 n_bins; /* number of range bins */
  Radx::si16 n_pulses; /* number of integrated pulses */
  Radx::si16 n_values; /* number of following Radx::fl32 values */
  char compression; /* 0=nessuna, 1=Zlib */
  Radx::si32 beam_length; /* length of following data section. */
  char data_type; /* 1=uchar, 2=fl32, 3=ui16, 4=half */
} ros2_beam_hdr_t;

class Ascii2Radx {
  
public:

  // constructor
  
  Ascii2Radx (int argc, char **argv);

  // destructor
  
  ~Ascii2Radx();

  // run 

  int Run();

  // data members

  int OK;

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  vector<string> _readPaths;

  vector<VarTransform *> _varTrans;

  int _volNum;
  int _sweepNum;

  int _year, _month, _day, _hour, _min, _sec;
  RadxTime _volStartTime;

  int _nSamples;
  int _nGates;
  int _nAz;
  int _nPtsData;

  double _latitude;
  double _longitude;
  double _altitudeM;

  double _antennaGain;
  double _beamWidth;

  double _antennaSpeedAz;
  double _antennaSpeedEl;

  double _frequencyHz;
  double _peakPowerWatts;
  double _prf;
  double _pulseWidthSec;
  double _rxBandWidthHz;

  double _noiseLevelDbm;
  double _dynamicRangeDb;

  double _gateSpacingM;
  double _startRangeM;
  
  double _azimuthResDeg;
  double _radarConstant;

  double _elevDeg;
  double _startAz;

  RadxArray<double> _fieldData_;
  double *_fieldData;

  double _nyquist;
  int _dataType;

  int _runFilelist();
  int _runRealtimeWithLdata();
  int _runRealtimeNoLdata();
  int _handleFile(const string &filePath,
                RadxVol &vol);
  void _finalizeVol(RadxVol &vol);
  void _convertFields(RadxVol &vol);
  void _setupWrite(RadxFile &file);
  void _setGlobalAttr(RadxVol &vol);
  int _writeVol(RadxVol &vol);

  int _handleBufrAscii(const string &readPath,
                       RadxVol &vol);
  int _readBufrMetaData(FILE *inFile);
  int _readBufrFieldData(FILE *inFile);
  int _readBufrMetaVariable(FILE *inFile,
                            string varLabel, int &ival,
                            string precedingLabel = "");
  int _readBufrMetaVariable(FILE *inFile,
                            string varLabel, double &dval,
                            string precedingLabel = "");
  int _readBufrDataValue(FILE *inFile,
                         string varLabel, double &dval);
  
  int _handleItalyAscii(const string &readPath,
                        RadxVol &vol);

  int _readAsciiVolHeader(const string &line);

  int _readAsciiBeamHeader(const string &line,
                           RadxTime &beamTime,
                           double &el,
                           double &az,
                           size_t &nGates);

  int _decodeAsciiBeamField(const string &line,
                            size_t nGates,
                            RadxRay *ray);

  int _handleItalyRos2(const string &readPath,
                       RadxVol &vol);
  
  int _printRos2ToFile(const string &readPath, FILE *out);
  
  int _ros2Uncompress(unsigned char *in, int n_in,
                      unsigned char *out, int n_out);
  
  void _ros2PrintValues(int dataType, int position, int n_bins,
                        char* beam, FILE* out);
  
  void _addFieldToRay(int fieldId,
                      int dataType,
                      int position,
                      int n_bins,
                      char* beam,
                      RadxRay *ray);
  
  void _convertRos2ArrayToFloat(int fieldId,
                                int dataType,
                                int position,
                                int n_bins,
                                char* beam,
                                vector<Radx::fl32> &floats);

  void _convertItalyAscii2Floats(int fieldId,
                                 int dataType,
                                 size_t nGates,
                                 const vector<double> &doublesIn,
                                 vector<Radx::fl32> &floatsOut);
  
  void _setItalyFieldNames(int fieldId, RadxField *field);

  void _computeItalyScaleAndBias(int fieldId, int dataType,
                                 double &scale,
                                 double &bias, double &range);
    
};

#endif

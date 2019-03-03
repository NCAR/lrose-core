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
// RvSim.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2003
//
///////////////////////////////////////////////////////////////

#ifndef RvSim_H
#define RvSim_H

#include <string>
#include <vector>
#include <radar/RadarComplex.hh>
#include <radar/RadarFft.hh>
#include "Args.hh"
#include "Params.hh"

using namespace std;

typedef struct {
  float i;
  float q;
} iq_t;

////////////////////////
// This class

class RvSim {
  
public:

  typedef struct {
    double dbm;
    double power;
    double vel;
    double width;
  } moments_t;

  // constructor

  RvSim (int argc, char **argv);

  // destructor
  
  ~RvSim();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  RadarFft *_fft;
  
  int _beamNum;
  int _gateNum;

  int _nSamples;
  int _nBeams;
  int _nGates;

  static const int _phaseCodeN = 8;
  static const int _phaseCodeM = 64;
  RadarComplex_t _phaseCode[_phaseCodeM];
  
   void _initPhaseCodes();

  void _createSimData(vector<iq_t> &iqVec,
		      vector<moments_t> &trip1Moments,
		      vector<moments_t> &trip2Moments);

  void _createGateData(const moments_t &trip1Mom,
		       const moments_t &trip2Mom,
		       vector<iq_t> &gateIQ,
		       vector<moments_t> &trip1Moments,
		       vector<moments_t> &trip2Moments);

  void _createGaussian(double power,
		       double vel,
		       double width,
		       RadarComplex_t *volts);

  void _encodeTrip(int tripNum,
		   const RadarComplex_t *trip,
		   RadarComplex_t *tripEncoded);

  void _cohere2Trip1(const RadarComplex_t *IQ,
		     RadarComplex_t *trip1);
  
  void _printComplex(ostream &out,
		     const string &heading,
		     const RadarComplex_t *comp);

  void _printVector(ostream &out,
		    const string &heading,
		    const RadarComplex_t *comp);

  double _computePower(const RadarComplex_t *IQ);

  void _momentsByPp(const RadarComplex_t *IQ, double prtSecs,
		    double &power, double &vel, double &width);

  void _momentsByFft(const RadarComplex_t *IQ, double prtSecs,
		     double &power, double &vel, double &width);

  double _computeSpectralNoise(const double *magCentered);
  
  void _writeSpectraFile(const string &heading,
			 const RadarComplex_t *comp);
  
  int _writeTmpFile(const string &tmpPath,
		    const vector<iq_t> &iqVec,
		    const vector<moments_t> &trip1Moments,
		    const vector<moments_t> &trip2Moments);

};

#endif


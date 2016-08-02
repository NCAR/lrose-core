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
// WxSpecSim.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2003
//
///////////////////////////////////////////////////////////////

#ifndef WxSpecSim_H
#define WxSpecSim_H

#include <string>
#include <vector>
#include <netcdf.hh>
#include "Args.hh"
#include "Params.hh"
#include "Complex.hh"
#include "Fft.hh"

using namespace std;

typedef struct {
  float i;
  float q;
} iq_t;

////////////////////////
// This class

class WxSpecSim {
  
public:

  typedef struct {
    double dbm;
    double power;
    double vel;
    double width;
  } moments_t;

  // constructor

  WxSpecSim (int argc, char **argv);

  // destructor
  
  ~WxSpecSim();

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
  Fft *_fft;
  
  int _nSamples;
  int _nGates;
  int _gateNum;

  static const int _phaseCodeN = 8;
  static const int _phaseCodeM = 64;
  Complex_t _phaseCode[_phaseCodeM];
  
  void _createSimData(vector<iq_t> &iqVec,
		      vector<moments_t> &momArray);

  void _createGateData(double dbm,
                       double vel,
                       double width);
  
  void _createGaussian(double power,
		       double vel,
		       double width,
		       vector<Complex_t> &volts);
  
  void _printComplex(ostream &out,
		     const string &heading,
		     const vector<Complex_t> &comp);

  void _printVector(ostream &out,
		    const string &heading,
		    const vector<Complex_t> &comp);

  double _computePower(const vector<Complex_t> &IQ);
  
  void _momentsByPp(const vector<Complex_t> &IQ, double prtSecs,
		    double &power, double &vel, double &width);
  
  void _momentsByFft(const vector<Complex_t> &IQ, double prtSecs,
		     double &power, double &vel, double &width);
  
  double _computeSpectralNoise(const vector<double> &magCentered);
  
  void _writeSpectraFile(const string &heading,
			 const vector<Complex_t> &comp);
  
};

#endif


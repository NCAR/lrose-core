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
// TsSim.h
//
// TsSim object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
///////////////////////////////////////////////////////////////

#ifndef TsSim_H
#define TsSim_H

#include <tdrp/tdrp.h>
#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"
#include "Fft.hh"
#include <radar/RadarComplex.hh>
using namespace std;

class TsSim {
  
public:

  // constructor

  TsSim (int argc, char **argv);

  // destructor
  
  ~TsSim();

  // run 

  int Run();

  // data members

  int OK;

protected:
  
private:

  string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;

  int _runTest();
  int _runFft();
  int _runSprt();

  void _createTimeSeries(double power,
                         double vel,
                         double width,
                         int nSamples,
                         RadarComplex_t *iq);
  
  void _printComplex(ostream &out,
		     const string &heading,
                     int nSamples,
		     const RadarComplex_t *comp,
                     bool shift);

  void _printVector(ostream &out,
		    const string &heading,
                    int nSamples,
		    const RadarComplex_t *comp,
                    bool shift);

  void _printArray(ostream &out,
                   const string &heading,
                   int nSamples,
                   const double *array);

  void _computeWindowRect(int n, double *window);
  void _computeWindowVonhann(int n, double *window);
  void _computeWindowHanning(int n, double *window);
  void _computeWindowBlackman(int n, double *window);
  void _applyWindow(int nSamples,
                    const double *window,
                    RadarComplex_t *iq) const;

  void _computeOneSidedAutoCorr(int nSamples,
                                double power,
                                double width,
                                double nyquist,
                                double *corr) const;
  
  void _computeRealCrossCorr(int nSamples,
                             const double *aa,
                             const double *bb,
                             double *corr) const;

  void _createTimeSeries2(double power,
                          double vel,
                          double width,
                          int nSamples,
                          RadarComplex_t *iq);
  
  void _computeMomentsFromSpec(const RadarComplex_t *spec,
			       int nn,
			       double &power,
			       double &vel,
			       double &width);

  double _computePower(const RadarComplex_t *val,
		       int nn);
  
  
  void _loadPowerSpec(const RadarComplex_t *in,
		      int nn,
		      double *power);
  
  void _velWidthFromFft(const double *powerSpec,
			int nn,
			double &vel,
			double &width);
  
};

#endif

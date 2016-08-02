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
// Channel.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2006
//
///////////////////////////////////////////////////////////////

#ifndef Channel_HH
#define Channel_HH

#include <string>
#include <vector>
#include <iostream>
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class Channel {
  
public:
  
  // constructor
  
  Channel(const Params &params,
          const string &short_label,
          const string &long_label,
          double coupling_loss,
          double peak_power);
  
  // destructor
  
  ~Channel();

  // add data point

  void addDataPoint(double siggen,
                    double ifd);

  // compute the calibation fit
  // returns 0 on success, -1 on failure
  
  int computeFit();

  // print

  void print(ostream &out);

protected:
private:

  const Params &_params;

  string _shortLabel;
  string _longLabel;
  double _couplingLoss;
  double _peakPowerW;
  
  vector<double> _siggenDbm;
  vector<double> _ifdDbm;
  vector<double> _ifdMinusNoise;
  vector<double> _waveguideDbm;
  vector<double> _gain;
  vector<double> _gainError;

  double _meanNoiseIfd;
  double _meanNoiseGuide;
  double _meanGain;
  double _radarConst;
  double _dbz0;

  double _intercept;
  double _slope;
  double _slope_y_on_x;
  double _slope_x_on_y;
  double _corr;

  void _computeRadarConstant();
  int _computeMeanNoise();
  void _subtractNoise();
  void _subtractCouplerLoss();
  void _computeGain();
  int _computeMeanGain();
  void _computeGainError();
  int _computeRegression();

};

#endif


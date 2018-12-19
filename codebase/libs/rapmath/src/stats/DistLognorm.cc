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
///////////////////////////////////////////////////////////////
// DistNorm.cc
//
// Normal distributions
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2018
//
///////////////////////////////////////////////////////////////

#include <rapmath/DistLognorm.hh>
#include <iostream>

////////////////////////////////////////////////////
// constructor

DistLognorm::DistLognorm() :
        Distribution()
        
{
  _use3Params = false;
}

////////////////////////////////////////////////////
// destructor

DistLognorm::~DistLognorm()
        
{

}

////////////////////////////////////////////////////
// set to use 3 params
// default is to use 2 params

void DistLognorm::setUse3Params(bool state)
  
{
  _use3Params = state;
}

////////////////////////////////////////////////////
// set lower bound
// forces use of 3 params

void DistLognorm::setLowerBound(double val)
  
{
  _use3Params = true;
  _lowerBound = val;
}

//////////////////////////////////////////////////////////////////
// initialize the stats

void DistLognorm::_clearStats()

{

  Distribution::_clearStats();

  _meanLn = NAN;
  _sdevLn = NAN;
  _varianceLn = NAN;
  _lowerBound = 0.0;

}

////////////////////////////////////////////////////
// perform a fit

int DistLognorm::performFit()
  
{

  if (std::isnan(_histMin)) {
    computeHistogram();
  }

  _histPdf.clear();

  if (_nVals < 2) {
    _meanLn = NAN;
    _sdevLn = NAN;
    _varianceLn = NAN;
    _lowerBound = 0.0;
    return -1;
  }

  double nn = _nVals;
  double sumx = 0.0;
  double sumx2 = 0.0;
  
  for (size_t ii = 0; ii < _nVals; ii++) {
    double xx = log(_values[ii] - _lowerBound);
    sumx += xx;
    sumx2 += xx * xx;
  }

  _meanLn = sumx / nn;
  _varianceLn = (sumx2 - (sumx * sumx) / nn) / nn;

  if (_varianceLn >= 0.0) {
    _sdevLn = sqrt(_varianceLn);
  } else {
    _sdevLn = 0.0;
  }

  _mode = exp(_meanLn - _sdevLn * _sdevLn);
  _median = exp(_meanLn);

  for (size_t jj = 0; jj < _histNBins; jj++) {
    double xx = _histMin + jj * _histDelta;
    _histPdf.push_back(getPdf(xx));
  }

  _pdfAvail = true;

  computeHistCdf();

  return 0;

}

////////////////////////////////////////////////////
// get the pdf value for a given x
// a fit must have been performed

double DistLognorm::getPdf(double xx)
  
{
  xx -= _lowerBound;
  if (xx <= 0.0) {
    return 0.0;
  }
  double aa  = (log(xx) - _meanLn) / _sdevLn;
  double pdf = exp(-0.5 * aa * aa) / (xx * _sdevLn * sqrt2Pi);
  return pdf;
}

////////////////////////////////////////////////////
// get the cdf value for a given x
// a fit must have been performed

double DistLognorm::getCdf(double xx)
  
{
  double aa  = (log(xx) - _meanLn) / _sdevLn;
  double bb = erf(aa / sqrt2);
  double cdf = (1.0 + bb) * 0.5;
  return cdf;
}


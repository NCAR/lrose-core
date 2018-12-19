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

#include <rapmath/DistNormal.hh>
#include <iostream>

////////////////////////////////////////////////////
// constructor

DistNormal::DistNormal() :
        Distribution()
        
{
  
}

////////////////////////////////////////////////////
// destructor

DistNormal::~DistNormal()
        
{

}

////////////////////////////////////////////////////
// perform a fit

int DistNormal::performFit()
  
{

  if (std::isnan(_histMin)) {
    computeHistogram();
  }

  _histPdf.clear();

  computeSdev();

  if (_nVals < 2) {
    return -1;
  }

  _median = _mean;
  _mode = _mean;
  
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

double DistNormal::getPdf(double xx)
  
{
  double aa  = (xx - _mean) / _sdev;
  double pdf = exp(-0.5 * aa * aa) / (_sdev * sqrt2Pi);
  return pdf;
}

////////////////////////////////////////////////////
// get the cdf value for a given x
// a fit must have been performed

double DistNormal::getCdf(double xx)
  
{
  double aa  = (xx - _mean) / _sdev;
  double bb = erf(aa / sqrt2);
  double cdf = (1.0 + bb) * 0.5;
  return cdf;
}


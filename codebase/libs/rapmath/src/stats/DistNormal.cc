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
  computeSdev();
  if (_values.size() < 2) {
    return -1;
  }
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

//////////////////////////////////////////////////////////////////
// compute ChiSq goodness of fit test
// kk is number of intervals used in test

void DistNormal::computeChiSq(size_t nIntervals)
  
{
  
  if (std::isnan(_mean) || std::isnan(_sdev)) {
    performFit();
  }
  
  if (_hist.size() < 1) {
    computeHistogram();
  }
  double nValues = _values.size();
  // double intervalCount = nValues / (double) nIntervals;
  double intervalProb = 1.0 / (double) nIntervals;
  
  if (_debug) {
    cerr << "====>> DistNormal::computeChiSq <<====" << endl;
    cerr << "  nIntervals: " << nIntervals << endl;
    // cerr << "  intervalCount: " << intervalCount << endl;
  }

  size_t startIndex = 0;
  size_t endIndex = 0;
  // double sumHistCount = 0.0;
  double sumHistProb = 0.0;
  double sumChisq = 0.0;
  double nChisq = 0.0;
  // double sumPdfCount = 0.0;
  double sumPdfProb = 0.0;
  for (size_t jj = 0; jj < _hist.size(); jj++) {
    // sumHistCount += _hist[jj];
    sumHistProb += _hist[jj] / nValues;
    double xx = _histMin + (jj + 0.5) * _histDelta;
    double pdfProb = getPdf(xx) * _histDelta;
    // double pdfCount = pdfProb * nValues;
    // sumPdfCount += pdfCount;
    sumPdfProb += pdfProb;
    if ((sumHistProb > intervalProb) || (jj == _hist.size() - 1)) {
      endIndex = jj;
      // double xStart = _histMin + startIndex * _histDelta;
      // double xEnd = _histMin + (endIndex + 1) * _histDelta;
      // double xDelta = xEnd - xStart;
      // double xMid = (xStart + xEnd) / 2.0;
      // double meanCount = sumHistCount / (double) (endIndex - startIndex + 1);
      // double histDensity = histMean / (double) _values.size();
      // double pdf = getPdf(xMid);
      // double pdfCount = (pdf / (xEnd - xStart)) * _values.size();
      double error = sumHistProb - sumPdfProb;
      double chiFac = (error * error) / sumPdfProb;
      sumChisq += chiFac;
      nChisq++;
      if (_verbose) {
        cerr << "============================================" << endl;
        cerr << "-------->> jj: " << jj << endl;
        cerr << "-------->> startIndex: " << startIndex<< endl;
        cerr << "-------->> endIndex: " << endIndex << endl;
        cerr << "-------->> sumHistProb: " << sumHistProb << endl;
        cerr << "-------->> sumPdfProb: " << sumPdfProb << endl;
        // cerr << "-------->> xStart: " << xStart << endl;
        // cerr << "-------->> xEnd: " << xEnd << endl;
        // cerr << "-------->> xMid: " << xMid << endl;
        // cerr << "-------->> xDelta: " << xDelta << endl;
        // cerr << "-------->> meanCount: " << meanCount << endl;
        // cerr << "-------->> pdf: " << pdf << endl;
        // cerr << "-------->> pdfCount: " << pdfCount << endl;
        cerr << "-------->> error: " << error << endl;
        cerr << "-------->> chiFac: " << chiFac << endl;
      }
      startIndex = endIndex + 1;
      // sumHistCount = 0.0;
      sumHistProb = 0.0;
      // sumPdfCount = 0.0;
      sumPdfProb = 0.0;
    }
  } // jj

  _chiSq = sumChisq;

  if (_debug) {
    cerr << "  chiSq: " << _chiSq << endl;
  }

}


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

#ifndef STATISTIC_H
#define STATISTIC_H

class Statistic {

public:

long numTot;
long numNaN;
long numNegInf;
long numPosInf;
long numGood;

double dmin;
double dmax;
double dsum;
double dsumsq;


Statistic() {
  numTot = 0;
  numNaN = 0;
  numNegInf = 0;
  numPosInf = 0;
  numGood = 0;
  dmin   = numeric_limits<double>::quiet_NaN();
  dmax   = numeric_limits<double>::quiet_NaN();
  dsum   = 0;
  dsumsq = 0;
} // end constructor



void addOb(
  double val)
{
  numTot++;
  if (std::isnan(val)) numNaN++;
  else if (std::isinf(val) == -1) numNegInf++;
  else if (std::isinf(val) == 1) numPosInf++;
  else {
    numGood++;
    if (std::isnan( dmin) || val < dmin) dmin = val;
    if (std::isnan( dmax) || val > dmax) dmax = val;
    dsum += val;
    dsumsq += val * val;
  }
} // end addOb



void print( int prec) {
  int width = prec + 7;
  int svprec = cout.precision( 15);
  cout
    << "Statistic: "
    << "  numTot: " << numTot;

  cout << setprecision( prec);
  if (numGood > 0) {
    cout << "  min: " << setw( width) << dmin;
    cout << "  max: " << setw( width) << dmax;
    cout << "  mean: " << setw( width) << (dsum / numGood);
  }

  if (numGood > 1) {
    double variance = (dsumsq - dsum * dsum / numGood) / (numGood-1);
    double stddev = 0;
    if (variance > 0) stddev = sqrt( variance);
    cout << "  stddev: " << setw( width) << stddev;
  }

  cout << setprecision( 15);
  if (numGood != numTot) cout << "  numGood: " << numGood;
  if (numNaN != 0) cout << "  numNaN: " << numNaN;
  if (numNegInf != 0) cout << "  numNegInf: " << numNegInf;
  if (numPosInf != 0) cout << "  numPosInf: " << numPosInf;
  cout << endl;
  cout.precision( svprec);
}




///void throwerr( const char * msg) {
///  cout << "Statistic: throwerr: " << msg << endl;
///  cout.flush();
///  throw msg;
///}


}; // end class Statistic

#endif



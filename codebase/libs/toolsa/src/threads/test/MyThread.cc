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
// MyThread.cc
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2017
//
///////////////////////////////////////////////////////////////
//
// Class for testing TaThread
//
///////////////////////////////////////////////////////////////

#include "Params.hh"
#include "MyThread.hh"
#include <cmath>

// Constructor

MyThread::MyThread(const Params &params) :
        TaThread(),
        _params(params)
{
  _count = 0;
}  


MyThread::~MyThread() 
{
}

// override run method

void MyThread::run()
{
  
  // perform computations

  _count = 0;
  _sum = 0.0;
  
  for (double val = _startVal; val < _endVal + 0.5; val++) {
    
    double ang = fmod((val / 1000.0), 3.0);
    double sinVal = sin(ang);
    double cosVal = cos(ang);
    double rr = 99.0;
    double xx = rr * cosVal;
    double yy = rr * sinVal;
    double ang2 = atan2(yy, xx);
    if (fabs(ang2 - ang) > 0.0001) {
      cerr << "ERROR - ang, ang2: " << ang << ", " << ang2 << endl;
    }
    _count++;
    _sum += ang2;
    
  } // igate
    
}


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
///////////////////////////////////////////////////////////////
// TTest.cc
//
// TTest object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 1999
//
///////////////////////////////////////////////////////////////
//
// TTest is a test shell for C++
//
///////////////////////////////////////////////////////////////

#include "TTest.hh"
#include <string>
#include <cmath>
#include <malloc.h>
#include <iostream>
#include <toolsa/uusleep.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <didss/DataFileNames.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaStr.hh>
#include <radar/BeamHeight.hh>
#include "NanoTime.hh"
using namespace std;

// Constructor

TTest::TTest(int argc, char **argv) :
  _args("TTest")

{

  OK = TRUE;

  // set programe name

  _progName = strdup("TTest");

  // get command line args
  
  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    OK = FALSE;
    return;
  }

  return;

}

// destructor

TTest::~TTest()

{
  
  
}

//////////////////////////////////////////////////
// Run

int TTest::Run()
{
  
  _effectiveMult = 4.0 / 3.0;
  _effectiveMultNex = 1.21;
  _earthRadiusKm = 6375.636;
  _h0 = 0.0;

  BeamHeight bht;
  bht.setInstrumentHtKm(_h0);

  for (double elev = 0.0; elev < 20.25; elev += 0.5) {
    
    fprintf(stderr, "===========================================================\n");
    
    for (double range = 50.0; range < 400.5; range += 50.0) {

      _effRadiusKm = _earthRadiusKm * _effectiveMult;
  
      double nexradHt = _nexradHt(range, elev);
      double zrnicHt = _zrnicHt(range, elev);
      double zengHt = _zengHt(range, elev);
      double williamsHt = _williamsHt(range, elev);
      double myHt = bht.computeHtKm(elev, range);

      fprintf(stderr, "elev range nexrad myHt  zeng williams: %5.1f %6.1f %7.3f %7.3f %7.3f %7.3f\n",
              elev, range, nexradHt, myHt, zrnicHt, williamsHt);
      
      _effRadiusKm = _earthRadiusKm * _effectiveMultNex;

      double nexradHt2 = _nexradHt(range, elev);
      double zrnicHt2 = _zrnicHt(range, elev);
      double zengHt2 = _zengHt(range, elev);
      double williamsHt2 = _williamsHt(range, elev);

      fprintf(stderr, "1.21 earth nexrad zrnic zeng williams: %5.1f %6.1f %7.3f %7.3f %7.3f %7.3f\n",
              elev, range, nexradHt2, zrnicHt2, zengHt2, williamsHt2);

      fprintf(stderr, "==>> delta nexrad zrnic zeng williams: %5.1f %6.1f %7.3f %7.3f %7.3f %7.3f\n",
              elev, range,
              nexradHt2 - nexradHt, 
              zrnicHt2 - zrnicHt,
              zengHt2 - zengHt,
              williamsHt2 - williamsHt);

      fprintf(stderr, "--------------------------------------\n");

      // double nexradHt2 = _nexradHt(range, elev);
      // fprintf(stderr, "elev range ncar nexrad: %5.1f %6.1f %7.3f %7.3f\n",
      //         elev, range, nexradHt, nexradHt2);
        
    }

  }
  
  return 0;

}

double TTest::_nexradHt(double rangeKm, double elevDeg)

{

  double elevRad = elevDeg * DEG_TO_RAD;

  double ht = rangeKm * sin(elevRad) +
    (rangeKm * rangeKm) / (2.0 * _effRadiusKm) + _h0;
  
  return ht;

}

double TTest::_zrnicHt(double rangeKm, double elevDeg)

{

  double elevRad = elevDeg * DEG_TO_RAD;

  double term1 = rangeKm * rangeKm + _effRadiusKm * _effRadiusKm;
  double term2 = rangeKm * 2.0 * _effRadiusKm * sin(elevRad);
  
  double ht = sqrt(term1 + term2) - _effRadiusKm + _h0;
  
  return ht;

}

double TTest::_cosineHt(double rangeKm, double elevDeg)

{

  double elevRad = elevDeg * DEG_TO_RAD;
  double thetaRad = rangeKm / _effRadiusKm;
  
  double rdiff = _effRadiusKm - _earthRadiusKm;

  double term1 =
    (rdiff * rdiff) + 
    (_effRadiusKm * _effRadiusKm) -
    (2.0 * rdiff * _effRadiusKm * cos(thetaRad));

  double ht = sqrt(term1) - _earthRadiusKm + rangeKm * sin(elevRad) + _h0;
  
  return ht;

}

double TTest::_williamsHt(double rangeKm, double elevDeg)

{

  double rdiff = _effRadiusKm - _earthRadiusKm;
  double elevRad = elevDeg * DEG_TO_RAD;

  double thetaE = rangeKm / _effRadiusKm;
  double Xe = _effRadiusKm * sin(thetaE);
  double Ye = _effRadiusKm * cos(thetaE);

  double Ya = Ye - rdiff;
  
  double theta = atan2(Xe, Ya);

  double X = _earthRadiusKm * sin(theta);
  double Y = _earthRadiusKm * cos(theta) + rdiff;

  double dX = Xe - X;
  double dY = Ye - Y;

  double hSq = dX * dX + dY * dY;
  
  double ht = sqrt(hSq) + rangeKm * sin(elevRad) + _h0;
  
  return ht * 3.0;

}

double TTest::_zengHt(double rangeKm, double elevDeg)

{

  double elevRad = elevDeg * DEG_TO_RAD;

  double xx = pow(_effRadiusKm + _h0, 2.0);
  double yy = pow(rangeKm, 2.0);
  double zz = 2.0 * (_effRadiusKm + _h0) * rangeKm * sin(elevRad);

  double ht = sqrt(xx + yy + zz) - _effRadiusKm;
  
  return ht;

}


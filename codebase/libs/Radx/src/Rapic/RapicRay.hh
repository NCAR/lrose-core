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
// RapicRay.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2013
//
///////////////////////////////////////////////////////////////

#ifndef RapicRay_HH
#define RapicRay_HH

#include <iostream>
using namespace std;
class sRadl;
class ScanParams;

////////////////////////
// This class

class RapicRay {
  
public:

  // constructor
  
  RapicRay (const sRadl *radial, const ScanParams &sParams,
            bool isBinary, double target_elev);
  
  RapicRay (const RapicRay *beam, int ngates);

  // destructor
  
  ~RapicRay();

  // print

  void print(ostream &out);

  // print full
  
  void printFull(ostream &out);

  // data

  time_t timeSecs;
  double azimuth;
  double elevation;
  int nGates;
  int *vals;
  
protected:
  
private:
  
};

#endif

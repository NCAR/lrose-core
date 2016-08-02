// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:30:34 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// TsStatus.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2007
//
///////////////////////////////////////////////////////////////
//
// Get status from time series
//
////////////////////////////////////////////////////////////////

#ifndef TsStatus_H
#define TsStatus_H

#include "Args.hh"

#include <string>
#include <vector>
#include <cstdio>

#include <rvp8_rap/TsReader.hh>
#include <rvp8_rap/RapComplex.hh>
#include <rvp8_rap/Socket.hh>

using namespace std;

////////////////////////
// This class

class TsStatus {
  
public:

  // constructor

  TsStatus (const Args &args);

  // destructor
  
  ~TsStatus();

  // Get status in XML form
  // returns 0 on success, -1 on failure
  
  int getStatusXml(string &xml);

protected:
  
private:

  const Args &_args;
  TsReader _reader;

  void _computeRangeInfo(double &startRange,
			 double &maxRange,
			 double &gateSpacing);

};

#endif

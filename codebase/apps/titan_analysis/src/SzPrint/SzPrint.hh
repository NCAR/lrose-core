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
// SzPrint.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2004
//
///////////////////////////////////////////////////////////////
//
// SzPrint prints out MDV data in table format for performing
// SZ comparisons. Output is to stdout.
//
////////////////////////////////////////////////////////////////

#ifndef SzPrint_H
#define SzPrint_H

#include <string>
#include "Args.hh"
#include "Params.hh"
#include <Mdv/DsMdvxInput.hh>
using namespace std;

////////////////////////
// This class

class SzPrint {
  
public:

  // constructor

  SzPrint (int argc, char **argv);

  // destructor
  
  ~SzPrint();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  bool _doNcarLongPp;
  bool _doNcarShortPp;
  bool _doNcarShortFft;

  int _readFile(DsMdvx &mdvx, const char *path);

  int _doPrint(const DsMdvx &rph,
	       const DsMdvx &msz,
	       const DsMdvx &longPrt,
	       const DsMdvx &ncarLongPp,
	       const DsMdvx &ncarShortPp,
	       const DsMdvx &ncarShortFft);

  void _computeIndices(MdvxField *shortDbzFld,
		       MdvxField *longDbzFld,
		       int ix, int iy,
		       int nGatesFirstTrip,
		       int &iRangeShort,
		       int &iRangeLong,
		       int &tripNum,
		       double &range,
		       double &dbz1,
		       double &dbz2,
		       double &pow1,
		       double &pow2);

  void _computeVelCensoring(const fl32 *vel,
			    int ngates,
			    fl32 missing_val,
			    double nyquist,
			    int *vcensor);

  double _computeInterest(double xx,
			  double x0, double x1);
  
  void _computeVelSpeckleCensoring(const fl32 *vel,
				   int ngates,
				   fl32 missing_val,
				   double nyquist,
				   int *vcensor);
};

#endif


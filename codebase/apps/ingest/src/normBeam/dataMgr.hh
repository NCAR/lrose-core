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


#ifndef dataMgr_HH
#define dataMgr_HH

#define MAX_FIELDS 20

#include <string>
#include <rapformats/DsRadarMsg.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

class Socket;

////////////////////////
// This class

class dataMgr {
  
public:

  // constructor

  dataMgr (int argc, char **argv);

  // destructor
  
  ~dataMgr();

  // run 

  int Run();
  
  // data members
  
  bool isOK;

protected:
  
private:

  const static int _outputMissingVal = -9999;
  const static int _outputByteWidth = 4;
  const static float _outputScale = 1.0;
  const static float _outputBias = 0.0;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  
  double _gateSpacing;
  double _startRange;
  int _numGates;
  float *_scale;
  float *_bias;
  int *_missingVal;
  int *_byteWidth;
  
  int _fieldNumToProcess;
  int _numFields;

  DsFieldParams *_outFieldParams[MAX_FIELDS];

  int _run();

  //
  // Constants/varibales to allow us to keep track of
  // how many consectutive getDsBeam failures have occurred.
  // If the maximum is exceeded, we exit.
  //
  int _numGetDsBeamFails;
  const static int _maxNumGetDsBeamFails = 120;

  int _getNewBeam(void *oldData, float *newData);

  void _interpBeam(float *inField, 
		   int numGates,
		   double minVal,
		   double maxVal);

};

#endif


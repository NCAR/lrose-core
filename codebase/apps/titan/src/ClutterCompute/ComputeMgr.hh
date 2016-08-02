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
// ComputeMgr.hh
//
// Manages the computations for median
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2002
//
/////////////////////////////////////////////////////////////

#ifndef COMPUTEMGR_HH
#define COMPUTEMGR_HH

#include <string>
#include <vector>
#include <set>
#include <Mdv/DsMdvxInput.hh>
#include "Params.hh"
using namespace std;

typedef multiset<fl32, less<fl32> > fieldSet;

class ComputeMgr {
  
public:

  ComputeMgr(const Params &params);
  ~ComputeMgr();

  int scanFiles(DsMdvxInput &input);
  int allocArrays();
  int computeMedian(DsMdvxInput &input);
  int writeOutput(DsMdvxInput &input);


protected:

  const Params &_params;

  int _nPointsField;
  int _nVolumes;
  int _nFields;
  int _nPointsPerPass;
  time_t _startTime, _endTime;
  vector<fl32 *> _fieldMedians;
  vector<fl32 *> _fieldVolumes;
  
  void _setupRead(DsMdvx &mdvx);

  static int _fl32Compare(const void *i, const void *j);

private:

};

#endif


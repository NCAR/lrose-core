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
// ClutterMap.hh
//
// ClutterMap object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2004
//
///////////////////////////////////////////////////////////////

#ifndef ClutterMap_H
#define ClutterMap_H

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <tdrp/tdrp.h>

#include "Args.hh"
#include "Params.hh"
#include "Trigger.hh"
#include <string>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>

using namespace std;

class ClutterMap {
  
public:

  // constructor

  ClutterMap (int argc, char **argv);

  // destructor
  
  ~ClutterMap();

  // run 

  int Run();

  // data members

  int isOK;

protected:
  
private:

  const static fl32 missingVal = -9999.0;
  
  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  time_t _dataStartTime;
  time_t _dataEndTime;
  Trigger *_trigger;

  int _nTimes;
  int _nx, _ny, _nz;
  int _nPointsGrid;
  fl32 *_dbzThresh;
  fl32 *_nn;
  fl32 *_sumDbz;
  fl32 *_meanDbz;
  fl32 *_frac;
  
  Mdvx::master_header_t _mhdr;
  Mdvx::field_header_t _fhdr;
  Mdvx::vlevel_header_t _vhdr;

  int _respondToTrigger();
  int _initArrays(time_t initTime);
  void _allocArrays();
  void _freeArrays();
  void _initMinDbz(const Mdvx::field_header_t &fhdr);
  int _accumStatsFromRaw(const vector<time_t> &times);
  int _accumStatsFromIntermediate(const vector<time_t> &times);
  void _computeMeanDbz();
  void _identifyClutter();

  int _writeIntermediate(time_t start_time,
			 time_t end_time,
			 time_t centroid_time,
			 const Mdvx::master_header_t &mhdrIn,
			 const Mdvx::field_header_t &fhdrIn,
			 const Mdvx::vlevel_header_t &vhdrIn,
			 int nTimes,
			 const fl32 *nn,
			 const fl32 *meanDbz);
  
  int _writeFinal(time_t start_time,
		  time_t end_time,
		  time_t centroid_time,
		  const Mdvx::master_header_t &mhdrIn,
		  const Mdvx::field_header_t &fhdrIn,
		  const Mdvx::vlevel_header_t &vhdrIn,
		  int nTimes,
		  const fl32 *frac,
		  const fl32 *meanDbz);

};

#endif


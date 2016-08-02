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
// File2Fmq.hh
//
// File2Fmq object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2001
//
///////////////////////////////////////////////////////////////

#ifndef File2Fmq_H
#define File2Fmq_H

#include <string>
#include <vector>
#include "Params.hh"
#include "Field.hh"
#include <Fmq/DsRadarQueue.hh>
#include <dd_sweepfiles.hh>
using namespace std;

////////////////////////
// This class

class File2Fmq {
  
public:

  typedef enum {
    DORADE_SI08 = 1,
    DORADE_SI16 = 2,
    DORADE_SI32 = 3,
    DORADE_FL32 = 4
  } dorade_data_t;

  // constructor

  File2Fmq(const Params &params,
	   dd_mapper &mapr,
	   dd_sweepfile_access &sac,
	   DsRadarQueue &r_queue);
  
  // destructor
  
  ~File2Fmq();

  // put the params

  int writeParams(const char *input_path);

  // write the beam data

  int writeBeams(int vol_num, int tilt_num,
		 time_t start_time);

protected:
  
private:

  static const fl32 MISSING_FL32;
  static const ui16 MISSING_UI16;
  static const ui08 MISSING_UI08;

  const Params &_params;
  dd_mapper &_mapr;
  dd_sweepfile_access &_sac;
  DsRadarQueue &_rQueue;

  int _timeDimId;
  int _maxCellsDimId;

  int _scanId;
  int _dataByteWidth;
  int _missingVal;
  int _nTimes;
  int _maxCells;
  time_t _baseTime;
  double _fixedAngle;

  double *_timeOffsetData;
  fl32 *_elevationData;
  fl32 *_azimuthData;

  vector<Field *> _fields;

  void _setVars();

  void _findFields();
  
  void _convertToFl32(int ipos,
		      int ncells,
		      si32 doradeBad,
		      double doradeScale,
		      double doradeBias,
		      fl32 *fdata);

  void _convertToOutput(fl32 *fdata,
			int ifield,
			int nfields,
			int ncells,
			double outScale,
			double outBias,
			void *outputData);
    
  int _findNameInList(const string &name,
		      const string &list);

  void _printCells(PARAMETER *fparam, int ipos,
		   int ncells, int doradeBad,
		   double doradeScale, double doradeBias);
  
  
};

#endif


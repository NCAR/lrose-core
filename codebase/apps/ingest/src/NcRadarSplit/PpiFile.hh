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
// PpiFile.hh
//
// PpiFile object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2001
//
///////////////////////////////////////////////////////////////

#ifndef PpiFile_H
#define PpiFile_H

#include <string>
#include <vector>
#include "Params.hh"
#include <Ncxx/Nc3File.hh>
using namespace std;

////////////////////////
// This class

class PpiFile {
  
public:

  // constructor

  PpiFile (const Params &params,
	   const Nc3File &ncf,
	   int start_beam,
	   int end_beam,
	   int tilt_num,
	   int vol_num);

  // destructor
  
  ~PpiFile();

  // check the file
  
  int checkFile();

  // set the vars

  void setVars();

  // write the file

  int write();

protected:
  
private:

  const Params &_params;
  const Nc3File &_ncfIn;

  int _startBeam;
  int _endBeam;
  int _tiltNum;
  int _volNum;

  int _timeDimId;
  int _maxCellsDimId;

  int _nTimes;
  int _maxCells;
  time_t _baseTime;

  Nc3Var *_baseTimeVar;
  Nc3Var *_fixedAngleVar;
  Nc3Var *_timeOffsetVar;
  Nc3Var *_azimuthVar;
  Nc3Var *_elevationVar;
  
  Nc3Values *_timeOffsetVals;
  Nc3Values *_elevationVals;
  Nc3Values *_azimuthVals;

  double *_timeOffsetData;
  float *_elevationData;
  float *_azimuthData;

  int _createTmp(const char *tmp_path,
		 time_t start_time,
		 float fixed_angle,
		 int n_beams);

};

#endif


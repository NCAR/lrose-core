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
// frenchRadar2Dsr.hh
//
// frenchRadar2Dsr object
//
// Watches over MDV data in/out
//
// October 2002
//
///////////////////////////////////////////////////////////////


#ifndef frenchRadar2Dsr_H
#define frenchRadar2Dsr_H

#include "Params.hh"
#include <Fmq/DsRadarQueue.hh>   
#include <toolsa/MsgLog.hh>
#include <vector>
using namespace std;

class frenchRadar2Dsr {
  
public:
  
  // constructor. Sets up DsMdvx object.
  frenchRadar2Dsr (Params *TDRP_params);

  // 
  void  frenchRadar2DsrFile( char *filename); 
  
  // destructor. Write Mdv data out.
  ~frenchRadar2Dsr();

  
protected:
  
private:

  Params *      _params;
  DsRadarQueue  _radarQueue; 
  MsgLog        _msgLog;
  int           _volumeNum;
  DsRadarMsg    _radarMsg; 

  const static int _nFields=1;
  const static int _badVal = -999;
  const static double _scale = 1.0;
  const static double _bias = 0.0;

  DsFieldParams *_fieldParams[_nFields];

};

#endif

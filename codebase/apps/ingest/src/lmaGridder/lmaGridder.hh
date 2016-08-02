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
// lmaGridder.hh
//
// lmaGridder object
//
//
///////////////////////////////////////////////////////////////


#ifndef lmaGridder_H
#define lmaGridder_H

#include "Params.hh"
#include <toolsa/MsgLog.hh>
#include <vector>

#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/DsMdvx.hh>

using namespace std;

class lmaGridder {
  
public:
  
  // constructor.
  lmaGridder (Params *TDRP_params);

  // 
  void  addToGrid(double lat, 
		  double lon, 
		  double alt, 
		  time_t entryTime);
  
  // destructor.
  ~lmaGridder();

  
protected:
  
private:

  void _resetData();
  void _incPoint(double lat,
		 double lon,
		 double alt);
  void _outputGrids();

  bool _firstPoint;
  float *_twoDdata;
  float *_threeDdata;

  Mdvx::field_header_t _twoDfhdr,  _threeDfhdr;
  Mdvx::vlevel_header_t _twoDvhdr, _threeDvhdr;
  Mdvx::master_header_t _Mhdr;
  MdvxProj *_Proj;
  unsigned long _numAdded;
  int _count;
  time_t _endTime;

  Params *      _params;

  typedef struct {
    double lat;
    double lon;
    double alt;
    time_t time;
  } _lma_entry_t;

  vector <_lma_entry_t> _entries;

};

#endif

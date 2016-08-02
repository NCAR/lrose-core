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
////////////////////////////////////////////////////////////////////
// <titan/TitanComplexTrack.hh>
//
// Complex track object for TitanServer
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, 80305-3000, USA
//
// Jan 2001
//
////////////////////////////////////////////////////////////////////

#ifndef TitanComplexTrack_HH
#define TitanComplexTrack_HH


#include <titan/TitanSimpleTrack.hh>
#include <toolsa/MemBuf.hh>
#include <vector>
using namespace std;

class TitanComplexTrack
{

  friend class TitanServer;

public:

  // constructor
  
  TitanComplexTrack();
  
  // Copy constructor

  TitanComplexTrack(const TitanComplexTrack &rhs);

  // destructor
  
  virtual ~TitanComplexTrack();

  // Assignment

  TitanComplexTrack &operator=(const TitanComplexTrack &rhs);

  // clear the object

  void clear();

  // data access

  const complex_track_params_t &complex_params() const {
    return _complex_params; }

  const vector<TitanSimpleTrack *> &simple_tracks() const {
    return _simple_tracks;
  }
  
  // assemble into buffer
  
  void assemble(MemBuf &buf, bool clear_buffer = false) const;
  
  // disassemble from buffer
  // Sets len_used to the length of the buffer used while disassembling.
  // Returns 0 on success, -1 on failure.
  
  int disassemble(const void *buf, int buf_len, int &len_used);
  
  // Print
  
  void print(FILE *out,
	     const storm_file_params_t &sparams,
	     const track_file_params_t &tparams);
  
  void printXML(FILE *out,
	     const storm_file_params_t &sparams,
	     const track_file_params_t &tparams);
  
protected:

  complex_track_params_t _complex_params;
  vector<TitanSimpleTrack *> _simple_tracks;
  
private:
  
  TitanComplexTrack &_copy(const TitanComplexTrack &rhs);
  
};

#endif



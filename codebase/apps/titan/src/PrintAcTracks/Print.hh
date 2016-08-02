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
// Print.hh
//
// Printing object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
/////////////////////////////////////////////////////////////

#ifndef PRINT_HH
#define PRINT_HH

#include <iostream>
#include <cstdio>
#include <Spdb/Spdb.hh>
using namespace std;

class Print {
  
public:

  // constructor

  Print(FILE *out, ostream &ostr, bool debug = false);

  // destructor

  virtual ~Print();

  // print methods

  void ascii(int data_len, void *data);
  void ac_vector(int data_len, void *data);
  void chunk_hdr(const Spdb::chunk_t &chunk);
  void ac_posn(int data_len,  void *data);
  void ac_posn_summary(time_t valid_time, int data_len,  void *data);
  void ac_posn_wmod(int data_len,  void *data);
  void ac_posn_wmod_summary(time_t valid_time, int data_len,  void *data);
  void ac_route(void *data);
  void ac_data(void *data);

protected:
  
private:

  FILE *_out;
  ostream &_ostr;
  bool _debug;
  
};

#endif


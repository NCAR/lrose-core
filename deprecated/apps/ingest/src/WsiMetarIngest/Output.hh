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
// Output.hh
//
// Output file class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 1999
//
///////////////////////////////////////////////////////////////

#ifndef Output_HH
#define Output_HH

#include <cstdio>
#include <string>
#include <toolsa/MemBuf.hh>
#include <didss/LdataInfo.hh>
#include "Params.hh"
using namespace std;

class Output {
  
public:

  // constructor

  Output(const string &prog_name, const Params &params);

  // destructor
  
  virtual ~Output();

  // open - open tmp file for writing

  int open ();

  // close - close tmp file

  void close ();

  // save - rename tmp file to permanent output file name

  int save (time_t data_time);

  // putChar - put a character

  int putChar(char cc);

  // putBuf - put contents of a membuf

  int putBuf(MemBuf &buf);

  // check for valid object

  bool isOK() { return (_isOK); }

protected:

private:

  bool _isOK;
  const string &_progName;
  const Params &_params;
  string _tmpDirPath;
  string _tmpFilePath;
  LdataInfo *_ldata;
  FILE *_fp;

};

#endif


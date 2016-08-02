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
// Input.hh
//
// Base and derived classes for dealing with different TAF inputs.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2000
//
///////////////////////////////////////////////////////////////

#ifndef Input_HH
#define Input_HH

#include <cstdio>
#include <string>
#include "Params.hh"
using namespace std;

/////////////////////
// Input - base class

class Input {
  
public:

  static const int MaxLine = 1024;

  // constructor
  
  Input(const string &prog_name, const Params &params);

  // destructor
  
  virtual ~Input();

  // open input file
  
  int open(const char *file_path);

  // close file

  void close();
  
  // read the next TAF from the file
  // returns 0 on success, -1 on failure (no more TAFS)
  
  virtual int readNext(string &tafStr);

  // get the header string after a readNext()

  const string &getHeadStr() { return _headStr; }
  
protected:
  
  const string &_progName;
  const Params &_params;

  string _filePath;
  FILE *_in;
  char _line[MaxLine];
  char _saved[MaxLine];
  string _headStr;
  bool _inTafBlock;

  void _fgets_save(char *buffer);
  char *_fgets(char *buffer);

private:

};

////////////////////////////////////////////////////////
// NnnnInput
//
// Derived class for NNNN input
//

class NnnnInput : public Input {
  
public:

  // constructor
  
  NnnnInput(const string &prog_name, const Params &params);

  // read the next TAF from the NNNN file
  // returns 0 on success, -1 on failure (no more TAFS)
  
  virtual int readNext(string &tafStr);

protected:

};

#endif


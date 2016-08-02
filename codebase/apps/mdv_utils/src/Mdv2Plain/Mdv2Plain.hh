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
// Mdv2Plain.hh
//
// Mdv2Plain object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1999
//
///////////////////////////////////////////////////////////////

#ifndef Mdv2Plain_H
#define Mdv2Plain_H

#include <string>
#include "Args.hh"
#include "Params.hh"
#include <dsserver/DsLdataInfo.hh>
#include <Mdv/DsMdvxInput.hh>
using namespace std;

class TaFile;

////////////////////////
// This class

class Mdv2Plain {
  
public:

  // constructor

  Mdv2Plain (int argc, char **argv);

  // destructor
  
  ~Mdv2Plain();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsMdvxInput _input;
  DsLdataInfo _ldataInfo;
  
  void _addRowsNorthToSouth(MemBuf &fldData, MdvxField &fld);
  int _processNextFile(DsMdvx &mdvx);
  int _write(const DsMdvx &mdvx, int nBytesVol);
  void _printInfo(const DsMdvx &mdvx, int &nBytesVol, ostream &out);
  int _writeFortranReclen(TaFile &out, si32 rec_len);
  int _writeFl32(TaFile &out, fl32 val);

};

#endif


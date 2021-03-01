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
// ConvStrat.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// MAY 2014
//
////////////////////////////////////////////////////////////////////
//
// ConvStrat finds stratiform regions in a Cartesian radar volume
//
/////////////////////////////////////////////////////////////////////

#ifndef ConvStrat_H
#define ConvStrat_H

#include <string>
#include <Mdv/DsMdvxInput.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxChunk.hh>
#include <toolsa/TaArray.hh>
#include <radar/ConvStratFinder.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class ConvStrat {
  
public:

  // constructor

  ConvStrat (int argc, char **argv);

  // destructor
  
  ~ConvStrat();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const fl32 _missing;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsMdvxInput _input;
  DsMdvx _inMdvx, _outMdvx;
  DsMdvx _tempMdvx;
  ConvStratFinder _finder;

  MdvxField *_tempField;
  MdvxField _shallowHtField;
  MdvxField _deepHtField;

  int _doRead();
  void _addFields();
  int _readTempProfile(time_t dbzTime,
                       const MdvxField *dbzField);
  void _computeHts(double tempC,
                   MdvxField &htField,
                   const string &fieldName,
                   const string &longName,
                   const string &units);
  int _doWrite();

  MdvxField *_makeField(Mdvx::field_header_t &fhdrTemplate,
                        Mdvx::vlevel_header_t &vhdr,
                        const fl32 *data,
                        Mdvx::encoding_type_t outputEncoding,
                        string fieldName,
                        string longName,
                        string units);

  MdvxField *_makeField(Mdvx::field_header_t &fhdrTemplate,
                        Mdvx::vlevel_header_t &vhdr,
                        const ui08 *data,
                        Mdvx::encoding_type_t outputEncoding,
                        string fieldName,
                        string longName,
                        string units);

};

#endif

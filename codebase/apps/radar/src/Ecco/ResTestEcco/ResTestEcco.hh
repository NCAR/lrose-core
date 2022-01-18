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
// ResTestEcco.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2022
//
////////////////////////////////////////////////////////////////////
//
// ResTestEcco tests Ecco for different grid resolutions.
// It does so by degrading the resolution of the input data set and
// comparing TDBZ for the different grid resolutions.
//
/////////////////////////////////////////////////////////////////////

#ifndef ResTestEcco_H
#define ResTestEcco_H

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

class ResTestEcco {
  
public:

  // constructor

  ResTestEcco (int argc, char **argv);

  // destructor
  
  ~ResTestEcco();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  fl32 _missingFloat;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsMdvxInput _input;
  DsMdvx _inMdvx;
  const MdvxField *_dbzField;
  DsMdvx *_outMdvx;
  vector<DsMdvx *> _results;
  ConvStratFinder _finder;

  MdvxField *_resReducedField;

  // kernel computations

  typedef struct {
    int jx, jy;
    ssize_t offset;
  } kernel_t;
  vector<kernel_t> _kernelOffsets;
  int _nxKernel, _nyKernel;

  int _processResolution(int resNum, double resFactor);
  MdvxField *_createDbzReducedRes(const MdvxField *dbzFieldIn,
                                  double resFactor);
  void _computeKernel(const Mdvx::field_header_t &fhdrIn,
                      double resFactor);

  int _doRead();
  void _addFields();
  int _doWrite(int resNum, double resFactor);

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

  void _clearResults();

};

#endif

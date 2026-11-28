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
// Parf2Mdv.hh
//
// Parf2Mdv object
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 2007
//
///////////////////////////////////////////////////////////////
//
// Parf2Mdv reads netCDF data and writes Mdv files
//
///////////////////////////////////////////////////////////////////////

#ifndef PARF2Mdv_H
#define PARF2Mdv_H

#include <string>
#include <toolsa/pmu.h>
#include <didss/DsInputPath.hh>
#include <didss/LdataInfo.hh>
#include <dsdata/TriggerInfo.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <euclid/Pjg.hh>
#include "Args.hh"
#include "Params.hh"
#include "ParfReader.hh"
#include <euclid/Pjg.hh>
using namespace std;

////////////////////////
// This class

class Parf2Mdv {
  
public:

  // constructor

  Parf2Mdv (int argc, char **argv);

  // destructor
  
  ~Parf2Mdv();

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

  DsInputPath *_fileTrigger;

  ParfReader *_parfReader;

  fl32 *_data;
  
  fl32 _missing;

  void _clear();

  int _processData(char *inputPath); 
 
  int _writeMdv();

  int _setMasterHeader(DsMdvx::master_header_t &masterHdr);

  int _setFieldHeader(Mdvx::field_header_t &fieldHdr, int i);

  int _setVlevelHeader(Mdvx::vlevel_header_t &vlevelHdr);
  
  int _createFieldDataArray();

};

#endif


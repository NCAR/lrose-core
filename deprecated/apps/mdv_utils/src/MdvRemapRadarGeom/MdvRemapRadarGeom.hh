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
// MdvRemapRadarGeom.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2011
//
///////////////////////////////////////////////////////////////
//
// MdvRemapRadarGeom remaps the beam geometry of
// polar radar data in an MDV file
//
///////////////////////////////////////////////////////////////

#ifndef MdvRemapRadarGeom_HH
#define MdvRemapRadarGeom_HH

#include "Args.hh"
#include "Params.hh"
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
using namespace std;

class DsInputPath;

class MdvRemapRadarGeom {
  
public:

  // constructor

  MdvRemapRadarGeom (int argc, char **argv);

  // destructor
  
  ~MdvRemapRadarGeom();

  // run 

  int Run();

  // data members

  bool isOK;
  
protected:
  
private:
  
  string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;
  DsInputPath *_input;

  class LutEntry {
  public:
    LutEntry(int start_index,
             int end_index,
             int mid_index,
             int n_gates) {
      startIndex = start_index;
      endIndex = end_index;
      midIndex = mid_index;
      nGates = n_gates;
    }
    int startIndex;
    int endIndex;
    int midIndex;
    int nGates;
  };
  vector<LutEntry> _lut;

  int _processFile(const char *file_path);

  void _setupRead(DsMdvx &mdvx);

  MdvxField *_remapField(MdvxField &inField,
                         const char *outputName,
                         Params::comb_method_t combMethod);

  void _remapBeam(Mdvx::field_header_t &fhdr,
                  int inNgates, int outNgates,
                  fl32 *inBeam, fl32 *outBeam,
                  Params::comb_method_t combMethod);
  
  void _computeRemapLut(Mdvx::field_header_t &fhdr,
                        int inNgates,
                        int outNgates);

};

#endif

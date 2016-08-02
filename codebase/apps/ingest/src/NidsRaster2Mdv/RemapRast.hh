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
// RemapRast.hh
//
// Remap the raster data onto a smaller/larger cart grid
//
// Marty Petach, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
/////////////////////////////////////////////////////////////

#ifndef RemapRast_HH
#define RemapRast_HH

#include <string>
#include <iostream>
#include <dataport/port_types.h>
#include <rapformats/nids_file.h>
#include "Params.hh"
#include "OutputMdv.hh"
using namespace std;

class RemapRast {
  
public:

  // constructor

  RemapRast(const string &prog_name, const Params &params,
            const string &radar_name,
            const string &output_dir);

  // destructor

  ~RemapRast();

  // process a file

  int processRastFile(const string &filePath);

protected:
  
private:

  const string &_progName;
  const Params &_params;
  string _radarName;
  string _outputDir;

  si32 _nptsGrid;

  int _outputVal[16];
  double _scale, _bias;

  OutputMdv _out;

  int _uncompressRasters(const NIDS_raster_header_t &rhdr,
                         int nRows,                                   
                         ui08 *rawPtr,                           
                         ui08 *rasters);

  void _computeScaleAndBias(NIDS_header_t &nhdr);

  void _doRemapping(ui08 *rasters);

};

#endif


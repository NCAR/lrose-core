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
// Mdv2Image.hh
//
// Mdv2Image object
//
// Original : Mike Dixon's PrintMdv 
// F. Hage December 2006.
///////////////////////////////////////////////////////////////

#ifndef Mdv2Image_H
#define Mdv2Image_H

#include <Mdv/DsMdvxThreaded.hh>
#include "Args.hh"
#include "Params.hh"
#include <string>

#include <X11/Xlib.h>
#include <Imlib2.h>

using namespace std;

class Mdv2Image {
  
public:

  // constructor

  Mdv2Image (int argc, char **argv);

  // destructor
  
  ~Mdv2Image();

  // run 

  int Run();

  // data members

  int OK;

protected:

  time_t _readSearchTime;
  time_t _latestValidModTime;
  time_t _dataTime;
  
  int _getVolume(DsMdvx *mdvx);
  int _getAllHeaders(DsMdvx *mdvx);
  int _getVsection(DsMdvx *mdvx);
  int _getTimeHeight(DsMdvx *mdvx);
  
  void _output_image(MdvxField *field);
  void _output_image_rgba(MdvxField *field);
  void _output_image_values(MdvxField *field);
  void _output_mdv_obj(const DsMdvx *mdvx);
  string _build_fname(MdvxField *field, int plane);

private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

};

#endif

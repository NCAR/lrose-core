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
// VaisalaLtg2Mdv.hh
//
// Defines VaisalaLtg2Mdv class
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#ifndef GRIB_NC2MDV_H
#define GRIB_NC2MDV_H

#include "Params.hh"

using namespace std;

class VaisalaLtg2Mdv {
  
public:

  // Constructor. Makes a copy of a pointer to the TDRP parameters.
  VaisalaLtg2Mdv (Params *P);


  // Destructor. Does nothing, but avoids use of the default destructor.
  ~VaisalaLtg2Mdv ();

  // Main routine - converts a netCDF file to an MDV file.
  void VaisalaLtg2MdvFile( char *FilePath );

  // public data

protected:
  
private:

  // Note that variables and methods that are private to a class
  // typically start with an underscore at RAL.

  // Local copy of a pointer to TDRP parameters.
  Params *_params;

  // Small local routine to check netCDF reads.
  void _checkStatus(int status, char *exitStr);

};

#endif






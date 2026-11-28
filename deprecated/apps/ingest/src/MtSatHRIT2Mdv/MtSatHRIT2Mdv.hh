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
// MtSatHRIT2Mdv.hh
//
// Defines MtSatHRIT2Mdv class
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#ifndef MTSAT_MDV_H
#define MTSAT_MDV_H

#include "Params.hh"

using namespace std;

class MtSatHRIT2Mdv {
  
public:

  // Constructor. Makes a copy of a pointer to the TDRP parameters.
  MtSatHRIT2Mdv (Params *P);


  // Destructor. Does nothing, but avoids use of the default destructor.
  ~MtSatHRIT2Mdv ();

  // Main routine - converts a netCDF file to an MDV file.
  void MtSatHRIT2MdvFile( char *FilePath );

  // public data

protected:
  
private:

  // Note that variables and methods that are private to a class
  // typically start with an underscore at RAL.

  // Local copy of a pointer to TDRP parameters.
  Params *_params;

  int _findFile( char *dir, char *nameSubString, 
		 char *dateSubString, char *fileName,
		 bool isVis, Params *TDRP_params);

  

};

#endif






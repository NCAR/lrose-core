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
// nsslMosaic2Mdv.hh
//
// nsslMosaic2Mdv object
//
// Watches over MDV data in/out
//
// October 2002
//
///////////////////////////////////////////////////////////////

#ifndef nsslMosaic2Mdv_H
#define nsslMosaic2Mdv_H

#include <vector>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxRemapLut.hh> 

#include "FieldHandler.hh"
#include "Params.hh"

using namespace std;

class nsslMosaic2Mdv
{
  
public:
  
  // constructor. Sets up DsMdvx object.
  nsslMosaic2Mdv (Params *TDRP_params);

  // 
  void  nsslMosaic2MdvFile( char *filename); 
  
  // destructor. Write Mdv data out.
  ~nsslMosaic2Mdv();

  
protected:
  
private:

  /////////////////////
  // Private members //
  /////////////////////

  Params *_params;

  // List of objects used to process the input fields

  vector< FieldHandler* > _fieldHandlerList;

  // Look up table for remapping the fields.

  MdvxRemapLut _lut;

  /////////////////////
  // Private methods //
  /////////////////////

  /**********************************************************************
   * _updateMasterHeader() - Update the master header for the output file
   *                         based on the information in this field.
   */

  void _updateMasterHeader(DsMdvx &mdv_file,
			   const MdvxField &field,
			   const string &filename);
  

};

#endif

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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: MergerFloat32.hh,v 1.5 2016/03/04 02:22:11 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header: MergerFloat32
// 
// Author: G M Cunning
// 
// Date:	Sun Feb  4 15:06:34 2001
// 
// Description:	this is a concrete class that merges FLOAT32 encoded fields.
// 
// 
// 
// 


# ifndef    MERGER_FLOAT32_H
# define    MERGER_FLOAT32_H

// C++ include files

// System/RAP include files
#include <Mdv/Mdvx.hh>

// Local include files
#include "Merger.hh"
using namespace std;



class MergerFloat32 : public Merger {
  
public:

  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  MergerFloat32(string prog_name, Params *params);

  // destructor
  virtual ~MergerFloat32();

  // merge those fields
  bool mergeField(InputFile *in_file, const int& i, OutputFile* out_file);

protected:

  ///////////////////////
  // protected members //
  ///////////////////////
  

  ///////////////////////
  // protected methods //
  ///////////////////////

private:

  /////////////////////
  // private members //
  /////////////////////


  /////////////////////
  // private methods //
  /////////////////////

  bool _isValid(const Mdvx::field_header_t& fhdr, const fl32& val);
  bool _isMissing(const Mdvx::field_header_t& fhdr, const fl32& val); 
  bool _isBad(const Mdvx::field_header_t& fhdr, const fl32& val); 

};

# endif     /* MERGER_FLOAT32_H */

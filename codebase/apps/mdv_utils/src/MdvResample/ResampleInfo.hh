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

/**
 *
 * @file ResampleInfo.hh
 *
 * @class ResampleInfo
 *
 * Class for passing info into the threaded resampling step
 *  
 * @date 1/15/2014
 *
 */

#ifndef ResampleInfo_HH
#define ResampleInfo_HH

#include <dataport/port_types.h>
#include <Mdv/DsMdvx.hh>

class MdvResample;

/** 
 * @class ResampleInfo
 */

class ResampleInfo
{
public:

  /**
   * @brief Constructor
   */

  inline ResampleInfo(int z, int y, fl32 *plane_ptr,
		      fl32 *input_plane_ptr,
		      const Mdvx::field_header_t *field_hdr,
		      const Mdvx::field_header_t *input_field_hdr,
		      const MdvResample *alg) :
    _y(y), _z(z), _plane_ptr(plane_ptr),
    _input_plane_ptr(input_plane_ptr), _field_hdr(field_hdr),
    _input_field_hdr(input_field_hdr), _alg(alg)
  {
  }

  
  /**
   * @brief Destructor
   */

  inline virtual ~ResampleInfo() {}
  
  int _y, _z;
  fl32 *_plane_ptr;
  fl32 *_input_plane_ptr;
  const Mdvx::field_header_t *_field_hdr;
  const Mdvx::field_header_t *_input_field_hdr;
  const MdvResample *_alg;

protected:
private:

};


#endif

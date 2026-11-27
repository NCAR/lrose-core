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

/***************************************************************************
 * convert_params.cc
 *
 * Converts enumerated type values in the parameter file into the values
 * needed by the called library/class routines.
 *
 * Nancy Rehak
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * January 1997
 *
 ****************************************************************************/

#include "kavm2mdv.h"
#include "kavm2mdv_tdrp.h"

#include <Mdv/mdv/mdv_macros.h>
using namespace std;

int convert_param_mdv_encoding_type(int param_encoding_type)
{
  switch(param_encoding_type)
  {
  case ENCODE_NATIVE :
    return(MDV_NATIVE);
    
  case ENCODE_INT8 :
    return(MDV_INT8);
    
  case ENCODE_INT16 :
    return(MDV_INT16);
    
  case ENCODE_FLOAT32 :
    return(MDV_FLOAT32);
    
  case ENCODE_PLANE_RLE8:
    return(MDV_PLANE_RLE8);
    
  default:
    return(MDV_PLANE_RLE8);
  
  } /* endswitch - param_encoding_type */

}


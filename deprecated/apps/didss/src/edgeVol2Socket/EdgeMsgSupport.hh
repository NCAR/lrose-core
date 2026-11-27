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

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/06 23:53:42 $
 *   $Id: EdgeMsgSupport.hh,v 1.2 2016/03/06 23:53:42 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * EdgeMsgSupport : Types used in EDGE messages.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef EdgeMsgSupport_HH
#define EdgeMsgSupport_HH


class EdgeMsgSupport
{

public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    EDGE_STATUS_MOMENT = 32,
    CORRECTED_REFLECTIVITY_MOMENT = 8,
    UNCORRECTED_REFLECTIVITY_MOMENT = 4,
    VELOCITY_MOMENT = 2,
    SPECTRUM_WIDTH_MOMENT = 1
  } moment_type_t;
  
  typedef enum
  {
    COMPRESSION_NONE = 0,
    COMPRESSION_LZW = 1,
    COMPRESSION_RLE = 2
  } compression_type_t;
  

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * binaryToDeg() - Convert a binary value into the equivalent value in
   *                 degrees.
   */

  static double binaryToDeg(const unsigned int binary_value)
  {
    return (double)binary_value / 65536.0 * 360.0;
  }
  

  /*********************************************************************
   * degToBinary() - Convert a value given in degrees into the equivalent
   *                 binary value.
   */

  static unsigned int degToBinary(const double deg_value)
  {
    return (unsigned int)(deg_value * 65536.0 / 360.0);
  }
  

};

#endif

   

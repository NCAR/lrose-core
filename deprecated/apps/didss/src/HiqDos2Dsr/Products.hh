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
/************************************************************************
 * Products: Base class for HiQ product handler classes.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Products_hh
#define Products_hh

#include "HiqBeamMsg.hh"
#include "HiqRadarMsg.hh"

using namespace std;


class Products
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructor
   */

  Products(const bool debug = false);


  /*********************************************************************
   * Destructor
   */

  virtual ~Products();


  /*********************************************************************
   * fillProducts() - Fill the products arrays based on the information
   *                  in the current HiQ messages.  The calling method
   *                  must have allocated enough space for each of the
   *                  product buffers.
   *
   * Returns true on success, false on failure.
   */

  virtual bool fillProducts(const HiqRadarMsg &radar_msg,
			    const HiqBeamMsg &beam_msg,
			    float *reflectivity_buffer,
			    float *coherent_reflectivity_buffer,
			    float *velocity_buffer,
			    float *spectrum_width_buffer,
			    float *ncp_buffer,
			    float *power_buffer) = 0;


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;

};

#endif

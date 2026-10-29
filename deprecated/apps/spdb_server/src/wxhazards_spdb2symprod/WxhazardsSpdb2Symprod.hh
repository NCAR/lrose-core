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
 *   $Date: 2016/03/07 18:41:15 $
 *   $Id: WxhazardsSpdb2Symprod.hh,v 1.2 2016/03/07 18:41:15 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * WxhazardsSpdb2Symprod.hh : header file for the WxhazardsSpdb2Symprod
 *                            program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef WxhazardsSpdb2Symprod_HH
#define WxhazardsSpdb2Symprod_HH

/*
 **************************** includes **********************************
 */

#include <sys/time.h>

#include <spdbFormats/ConvRegionHazard.hh>
#include <SpdbServer/SpdbServer.h>
#include <symprod/symprod.h>

#include "Args.hh"
#include "Params.hh"

/*
 ******************************* defines ********************************
 */


/*
 ******************************* structures *****************************
 */

/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */

class WxhazardsSpdb2Symprod
{
 public:

  // Destructor

  ~WxhazardsSpdb2Symprod(void);
  
  // Get WxhazardsSpdb2Symprod singleton instance

  static WxhazardsSpdb2Symprod *Inst(int argc, char **argv);
  static WxhazardsSpdb2Symprod *Inst();
  
  // Run the program.

  void run();
  
  // Flag indicating whether the program status is currently okay.

  bool okay;
  
  // Retrieves the program parameters

  Params *getParams(void)
  {
    return(_params);
  }
  
 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static WxhazardsSpdb2Symprod *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Server object.

  SpdbServer *_spdbServer;
  
  /////////////////////
  // Private methods //
  /////////////////////

  // Constructor -- private because this is a singleton object

  WxhazardsSpdb2Symprod(int argc, char **argv);
  
  // Add the given convective region to the Symprod buffer.

  static void _addConvRegionToBuffer(symprod_product_t *prod,
				     ConvRegionHazard *hazard,
				     Params *params);
  
  // Convert the data from the SPDB database to symprod format.

  static void *_convertToSymprod(spdb_chunk_ref_t *spdb_hdr,
				 void *spdb_data,
				 int spdb_len,
				 int *symprod_len);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("WxhazardsSpdb2Symprod");
  }
  
};


#endif

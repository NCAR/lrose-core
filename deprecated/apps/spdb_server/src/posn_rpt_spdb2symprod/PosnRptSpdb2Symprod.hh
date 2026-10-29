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
 *   $Id: PosnRptSpdb2Symprod.hh,v 1.2 2016/03/07 18:41:15 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * PosnRptSpdb2Symprod.hh : header file for the PosnRptSpdb2Symprod
 *                            program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef PosnRptSpdb2Symprod_HH
#define PosnRptSpdb2Symprod_HH

/*
 **************************** includes **********************************
 */

#include <sys/time.h>

#include <spdbFormats/PosnRpt.hh>
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

class PosnRptSpdb2Symprod
{
 public:

  // Destructor

  ~PosnRptSpdb2Symprod(void);
  
  // Get PosnRptSpdb2Symprod singleton instance

  static PosnRptSpdb2Symprod *Inst(int argc, char **argv);
  static PosnRptSpdb2Symprod *Inst();
  
  // Run the program.

  void run();
  
  // Flag indicating whether the program status is currently okay.

  bool okay;
  
  ////////////////////
  // Access methods //
  ////////////////////

  Params *getParams(void) const
  {
    return _params;
  }
  
  ui08 *getCurrentPosIcon(void) const
  {
    return _currentPosIcon;
  }
  
  ui08 *getWayPt0Icon(void) const
  {
    return _wayPt0Icon;
  }
  
  ui08 *getWayPt1Icon(void) const
  {
    return _wayPt1Icon;
  }
  
  ui08 *getWayPt2Icon(void) const
  {
    return _wayPt2Icon;
  }
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static PosnRptSpdb2Symprod *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Server object.

  SpdbServer *_spdbServer;
  
  // Icons for displaying

  ui08 *_currentPosIcon;
  ui08 *_wayPt0Icon;
  ui08 *_wayPt1Icon;
  ui08 *_wayPt2Icon;
  

  /////////////////////
  // Private methods //
  /////////////////////

  // Constructor -- private because this is a singleton object

  PosnRptSpdb2Symprod(int argc, char **argv);
  
  // Add the current position to the Symprod buffer.

  static void _addCurrentPositionToBuffer(symprod_product_t *prod,
					  const PosnRpt& posn_rpt,
					  const Params& params);
  
  // Add the given way point to the Symprod buffer.

  static void _addWayPtToBuffer(symprod_product_t *prod,
				const WayPoint& way_point,
				const char *color,
				const ui08 *icon);
  
  // Add the way point line to the Symprod buffer.

  static void _addWayPtLineToBuffer(symprod_product_t *prod,
				    const PosnRpt& posn_rpt,
				    const Params& params);
  
  // Convert the data from the SPDB database to symprod format.

  static void *_convertToSymprod(spdb_chunk_ref_t *spdb_hdr,
				 void *spdb_data,
				 int spdb_len,
				 int *symprod_len);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("PosnRptSpdb2Symprod");
  }
  
};


#endif

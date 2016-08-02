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
 *   $Date: 2016/03/07 01:39:55 $
 *   $Id: GenPolyStats2Ascii.hh,v 1.4 2016/03/07 01:39:55 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * GenPolyStats2Ascii: GenPolyStats2Ascii program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2009
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef GenPolyStats2Ascii_HH
#define GenPolyStats2Ascii_HH

#include <sys/time.h>

#include <rapformats/GenPolyStats.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


class GenPolyStats2Ascii
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  // Flag indicating whether the program status is currently okay.

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Destructor
   */

  ~GenPolyStats2Ascii(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static GenPolyStats2Ascii *Inst(int argc, char **argv);
  static GenPolyStats2Ascii *Inst();
  

  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /*********************************************************************
   * run() - run the program.
   */

  void run();
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static GenPolyStats2Ascii *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  GenPolyStats2Ascii(int argc, char **argv);
  

  /*********************************************************************
   * _processData() - Process the data for the given time.
   */

  bool _processData(const DateTime &start_time,
		    const DateTime &end_time);
  

  /*********************************************************************
   * _writeFieldListValues() - Write all of the fields whose field names
   *                           begin with the given string to cout.
   */

  void _writeFieldListValues(const GenPolyStats &polygon,
			     const string &field_prefix) const;
  

  /*********************************************************************
   * _writeFieldValue() - Write the given field value to cout
   */

  void _writeFieldValue(const GenPolyStats &polygon,
			const string &field_name,
			const string &field_label = "") const;
  

  /*********************************************************************
   * _writePolygon() - Write the information for this polygon to cout.
   */

  void _writePolygon(const GenPolyStats &polygon) const;
  

  /*********************************************************************
   * _writeScanType() - Write the scan type from the given polygon
   */

  void _writeScanType(const GenPolyStats &polygon) const;
  

};


#endif

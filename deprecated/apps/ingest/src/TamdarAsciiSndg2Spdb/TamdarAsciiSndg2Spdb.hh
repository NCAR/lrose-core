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
 *   $Date: 2016/03/07 01:23:06 $
 *   $Id: TamdarAsciiSndg2Spdb.hh,v 1.3 2016/03/07 01:23:06 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * TamdarAsciiSndg2Spdb: TamdarAsciiSndg2Spdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef TamdarAsciiSndg2Spdb_HH
#define TamdarAsciiSndg2Spdb_HH

#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <dsdata/TriggerInfo.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

#include "AsciiReader.hh"
#include "SpdbWriter.hh"

using namespace std;


class TamdarAsciiSndg2Spdb
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

  ~TamdarAsciiSndg2Spdb(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static TamdarAsciiSndg2Spdb *Inst(int argc, char **argv);
  static TamdarAsciiSndg2Spdb *Inst();
  

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

  static TamdarAsciiSndg2Spdb *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Input trigger object

  DsTrigger *_dataTrigger;

  // Input/output objects

  AsciiReader *_reader;
  SpdbWriter *_writer;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  TamdarAsciiSndg2Spdb(int argc, char **argv);
  

  /*********************************************************************
   * _processData() - Process the data for the given time.
   */

  bool _processData(const string &input_file_path);
  

  /*********************************************************************
   * _setSoundingLocation() - Set the overall sounding location using the
   *                          method indicated by the user.
   */

  void _setSoundingLocation(Sndg &sounding) const;
  

  /*********************************************************************
   * _sortPoints() - Sort the points in the sounding by pressure.
   */

  void _sortPoints(Sndg &sounding) const;
  

};


#endif

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
 *   $Date: 2016/03/06 23:53:39 $
 *   $Id: Binet2Dsr.hh,v 1.7 2016/03/06 23:53:39 dixon Exp $
 *   $Revision: 1.7 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Binet2Dsr: Binet2Dsr program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2003
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Binet2Dsr_HH
#define Binet2Dsr_HH


#include <cstdio>
#include <string>

#include <Fmq/DsFmq.hh>

#include "Args.hh"
#include "Params.hh"

#include "BeamWriter.hh"
#include "BinetBeamMsg.hh"
#include "BinetRadarMsg.hh"
#include "BinetReader.hh"

using namespace std;


class Binet2Dsr
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

  ~Binet2Dsr(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static Binet2Dsr *Inst(int argc, char **argv);
  static Binet2Dsr *Inst();
  

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

  static Binet2Dsr *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Data reader objects

  BinetReader *_reader;
  
  // Objects for writing the beam data

  BeamWriter _beamWriter;
  
  // Current messages since we generally need information from both
  // message types at the same time.

  BinetBeamMsg *_currentBeamMsg;
  BinetRadarMsg *_currentRadarMsg;


  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  Binet2Dsr(int argc, char **argv);
  

  /*********************************************************************
   * _processData() - Process data for the given time.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(void);
  

};


#endif

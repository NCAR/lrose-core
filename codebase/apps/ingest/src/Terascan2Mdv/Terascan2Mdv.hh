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
 *   $Id: Terascan2Mdv.hh,v 1.5 2016/03/07 01:23:06 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Terascan2Mdv.hh : header file for the Terascan2Mdv program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Terascan2Mdv_HH
#define Terascan2Mdv_HH

/*
 **************************** includes **********************************
 */

#include <ctime>
#include <cctype>
//#include <sys/time.h>
//#include <sys/types.h>

// Include the needed Terascan include files.  Note that they were
// written assuming that people would only write in C, not in C++.

#ifdef __cplusplus
extern "C" {
#endif

#include <gp.h>

#ifdef __cplusplus
}
#endif

#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;

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

class Terascan2Mdv
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

  ~Terascan2Mdv(void);
  
  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static Terascan2Mdv *Inst(int argc, char **argv);
  static Terascan2Mdv *Inst();
  

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

  static Terascan2Mdv *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  pid_t _pid;
   
  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  Terascan2Mdv(int argc, char **argv);
  

  /*********************************************************************
   * _getDataTimeFromFilename() - Get the data time from the input
   *                              filename.
   */

  DateTime _getDataTimeFromFilename(const char *input_filename) const;
  

  /*********************************************************************
   * _getDataTimeFromFile() - Get the data time from the satellite file.
   */

  static DateTime _getDataTimeFromDataset(SETP dataset);
  

  /*********************************************************************
   * _processDataset() - Process the given dataset.
   */

  bool _processDataset(const string &input_filename,
		       const string &output_filename = "");
  

  /*********************************************************************
   * _hildsPlay() - wrapper for processing data in a child process.
   */

  bool _childsPlay(const string &input_filename,
		   const string &output_filename);
  

  /*********************************************************************
   * _runArchiveMode() - Run in archive mode.
   */

  void _runArchiveMode();
  

  /*********************************************************************
   * _runRealtimeMode() - Run in realtime mode.
   */

  void _runRealtimeMode();
  

  /*********************************************************************
   * _waitForStableFile() - Wait for the given file to become stable
   *                        (i.e. for the file writing to finish).
   */

  static void _waitForStableFile(const string &filepath);
  
};


#endif

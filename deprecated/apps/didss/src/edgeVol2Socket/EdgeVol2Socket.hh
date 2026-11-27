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
 *   $Id: EdgeVol2Socket.hh,v 1.6 2016/03/06 23:53:42 dixon Exp $
 *   $Revision: 1.6 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * EdgeVol2Socket : header file for the EdgeVol2Socket program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef EdgeVol2Socket_HH
#define EdgeVol2Socket_HH


#include <string>
#include <vector>

#include <edge/vol.h>

#include "EdgeComm.hh"
#include "EdgeStatusMsg.hh"
using namespace std;


class EdgeVol2Socket
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

  ~EdgeVol2Socket(void);
  
  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static EdgeVol2Socket *Inst(int argc, char **argv);
  static EdgeVol2Socket *Inst();
  

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

  static EdgeVol2Socket *_instance;
  
  // Program parameters.

  char *_progName;
  
  bool _debugFlag;
  bool _printRadarInfo;
  
  bool _simulateRealtime;
  
  double _azimuthCorrection;
  
  // Object for communicating with client

  EdgeComm *_communicator;
  
  // List of input volume files to process

  vector< string > _fileList;
  
  // Members controlling how ofther the different messages are sent

  unsigned int _beamSleepMsecs;
  unsigned int _beamsBeforeStatusMsg;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  EdgeVol2Socket(int argc, char **argv);
  

  /*********************************************************************
   * _createFileList() - Create the list of files to process from the given
   *                     input directory.
   *
   * Returns true if the file list was successfully created, false
   * otherwise.
   *
   * The global member _fileList is updated to include all of the appropriate
   * files from the given directory when this method returns.
   */

  bool _createFileList(const string &directory,
		       const string &ext);
  

  /*********************************************************************
   * _loadStatusSweepInfo() - Load the status message information that is
   *                          constant throughout the sweep.
   */

  void _loadStatusSweepInfo(EdgeStatusMsg &status_msg,
			    const sweep_struct &sweep_info,
			    const int version_num);
  

  /*********************************************************************
   * _loadStatusVolInfo() - Load the status message information that is
   *                        constant throughout the volume.
   */
  
  void _loadStatusVolInfo(EdgeStatusMsg &status_msg,
			  const vol_struct &vol_info);
  
};


#endif

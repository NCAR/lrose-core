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
/**
 *
 * @file ApRemoval.hh
 *
 * @class ApRemoval
 *
 * ApRemoval is the top level application class.
 *  
 * @date 9/18/2002
 *
 */

#ifndef ApRemoval_HH
#define ApRemoval_HH

using namespace std;

#include <string>

#include <math.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/MsgLog.hh>

#include "DataMgr.hh"
#include "Params.hh"


/** 
 * @class ApRemoval
 */

class ApRemoval
{
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /** 
   * @brief Constructor
   */

  ApRemoval();

  /**
   * @brief Destructor
   */

  ~ApRemoval();
   
  /**
   * @brief Initialize the object.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   */

  int init(int argc, char** argv);

  /**
   * @brief Get the name of this application.
   *
   * @return Returns the name of the application.
   */

  inline const string &getProgramName() { return program.getFile(); }

  /**
   * @brief Get the message log object.
   *
   * @return Returns the message log object.
   */

  inline MsgLog& getMsgLog() { return msgLog; }
   
  /**
   * @brief Run the application.
   *
   * @return Returns the application return code.
   */

  int run();

private:

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief The path of the program executable.
   */

  Path program;
   
  /**
   * @brief The path of the parameter file.
   */

  char *paramsPath;

  /**
   * @brief The parameters read in from the parameter file.
   */

  Params *params;

  /**
   * @brief The message log being used by this application.
   */

  MsgLog msgLog;

  /**
   * @brief The object that manages the data for the application and that
   *        controls processing.
   */

  DataMgr dataMgr;
   

  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Print out the program usage.
   */

  void usage();

  /**
   * @brief Process the command like arguments.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   * @param[out] override The TDRP parameter overrides specified on the
   *                      command line.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int processArgs(int argc, char **argv,
		  tdrp_override_t& override);

  /**
   * @brief Initialize the data manager object using the input parameters.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int processParams();

  /**
   * @brief Initialize message logging.
   */

  void initLog();

};

//
// Make one instance global
//
#ifdef _APMAIN_
         ApRemoval *apRemoval;
#else
  extern ApRemoval *apRemoval;
#endif

//
// Macro for easy access to application name
//
#define PROGRAM_NAME apRemoval->getProgramName().c_str()

//
// Macros for message logging
//
#define POSTMSG       apRemoval->getMsgLog().postMsg
#define DEBUG_ENABLED apRemoval->getMsgLog().isEnabled( DEBUG )
#define INFO_ENABLED  apRemoval->getMsgLog().isEnabled( INFO )

//
// Prototypes for asyncrhronous handlers
//
void dieGracefully( int signal );

#endif


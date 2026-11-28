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
 * @file DeTectHorizXml2Spdb.hh
 *
 * @class DeTectHorizXml2Spdb
 *
 * DeTectHorizXml2Spdb program object.
 *  
 * @date 9/6/2011
 *
 */

#ifndef DeTectHorizXml2Spdb_HH
#define DeTectHorizXml2Spdb_HH

#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"

using namespace std;

/** 
 * @class DeTectHorizXml2Spdb
 */

class DeTectHorizXml2Spdb
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief Flag indicating whether the program status is currently okay.
   */

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Destructor
   */

  ~DeTectHorizXml2Spdb(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @return Returns a pointer to the DeTectHorizXml2Spdb instance.
   */

  static DeTectHorizXml2Spdb *Inst(int argc, char **argv);


  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the DeTectHorizXml2Spdb instance.
   */

  static DeTectHorizXml2Spdb *Inst();
  

  /**
   * @brief Initialize the local data.
   *
   * @return Returns true if the initialization was successful,
   *         false otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /**
   * @brief Run the program.
   */

  void run();
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static DeTectHorizXml2Spdb *_instance;
  
  /**
   * @brief Program name.
   */

  char *_progName;

  /**
   * @brief Command line arguments.
   */

  Args *_args;

  /**
   * @brief Parameter file parameters.
   */

  Params *_params;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @note Private because this is a singleton object.
   */

  DeTectHorizXml2Spdb(int argc, char **argv);
  

  /**
   * @brief Process the given file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processFiles();
  

};


#endif

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
 * @file Smos2Spdb.hh
 *
 * @class Smos2Spdb
 *
 * Smos2Spdb program object.
 *  
 * @date 10/2/2009
 *
 */

#ifndef Smos2Spdb_HH
#define Smos2Spdb_HH

#include <string>
#include <sys/time.h>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;

/** 
 * @class Smos2Spdb
 */

class Smos2Spdb
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

  ~Smos2Spdb(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @return Returns a pointer to the Smos2Spdb instance.
   */

  static Smos2Spdb *Inst(int argc, char **argv);


  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the Smos2Spdb instance.
   */

  static Smos2Spdb *Inst();
  

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

  static Smos2Spdb *_instance;
  
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
  
  /**
   * @brief Data triggering object.
   */

  DsTrigger *_dataTrigger;
  
  /**
   * @brief List of values used to indicate missing data in the input file.
   */

  vector< double > _inputMissingValues;
  

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

  Smos2Spdb(int argc, char **argv);
  

  /**
   * @brief Initialize the data trigger.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initTrigger();
  

  /**
   * @brief Process the given file.
   *
   * @param[in] file_path Path for the input file to process.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processFile(const string &file_path);
  

};


#endif

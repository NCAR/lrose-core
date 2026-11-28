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
 * @file MdvCopyMatchingProj.hh
 *
 * @class MdvCopyMatchingProj
 *
 * MdvCopyMatchingProj program object.
 *  
 * @date 6/29/2012
 *
 */

#ifndef MdvCopyMatchingProj_HH
#define MdvCopyMatchingProj_HH

#include <string>
#include <sys/time.h>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <Mdv/MdvxPjg.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;

/** 
 * @class MdvCopyMatchingProj
 */

class MdvCopyMatchingProj
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

  ~MdvCopyMatchingProj(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @return Returns a pointer to the class singleton instance.
   */

  static MdvCopyMatchingProj *Inst(int argc, char **argv);


  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the class singleton instance.
   */

  static MdvCopyMatchingProj *Inst();
  

  /**
   * @brief Initialize the local data.
   *
   * @return Returns true if the initialization was successful, false
   *         otherwise.
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

  ///////////////////////
  // Private constants //
  ///////////////////////

  /**
   * @brief Tolerance for comparing float values.
   */

  static const double TOLERANCE;
  

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static MdvCopyMatchingProj *_instance;
  
  /**
   * @brief Programname.
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
   * @brief The projection to look for.
   */

  MdvxPjg _proj;
  

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

  MdvCopyMatchingProj(int argc, char **argv);
  

  /**
   * @brief Initialize the desired projection.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initProj();
  

  /**
   * @brief Initialize the data trigger.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initTrigger();
  

  /**
   * @brief Process data for the given time.
   *
   * @param[in] trigger_time Trigger time.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processData(const DateTime &trigger_time);
  

};


#endif

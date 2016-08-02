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
 * @file UpdateMdvStartEndTimes.hh
 *
 * @class UpdateMdvStartEndTimes
 *
 * UpdateMdvStartEndTimes is the top level application class.
 *  
 * @date 10/7/2002
 *
 */

#ifndef UpdateMdvStartEndTimes_HH
#define UpdateMdvStartEndTimes_HH

#include <string>
#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <Mdv/Mdvx.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


/** 
 * @class UpdateMdvStartEndTimes
 */

class UpdateMdvStartEndTimes
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

  ~UpdateMdvStartEndTimes(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   */

  static UpdateMdvStartEndTimes *Inst(int argc, char **argv);

  /**
   * @brief Retrieve the singleton instance of this class.
   */

  static UpdateMdvStartEndTimes *Inst();
  

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
   * @brief Singleton instance pointer
   */

  static UpdateMdvStartEndTimes *_instance;
  
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
   * @brief Triggering object
   */

  DsTrigger *_dataTrigger;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @note The constructor is private because this is a singleton object
   */

  UpdateMdvStartEndTimes(int argc, char **argv);
  

  /**
   * @brief Get the data time of the next MDV volume.
   *
   * @param[in] curr_vol_time Current volume time.
   *
   * @return Returns the data time of the next volume on success,
   *         DateTime::NEVER on error.
   */

  DateTime _getNextVolTime(const DateTime &curr_vol_time) const;
  

  /**
   * @brief Get the data time of the previous MDV volume.
   *
   * @param[in] curr_vol_time Current volume time.
   *
   * @return Returns the data time of the previous volume on success,
   *         DateTime::NEVER on error.
   */

  DateTime _getPrevVolTime(const DateTime &curr_vol_time) const;
  

  /**
   * @brief Initialize the data trigger.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initTrigger(void);
  

  /**
   * @brief Process data for the given trigger time.
   *
   * @param[in] trigger_time Processing trigger time.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processData(const DateTime &trigger_time);
  

  /**
   * @brief Read the indicated input file.
   *
   * @param[out] input_file Input file that was read.
   * @param[in] trigger_time Trigger time for finding input file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readInputFile(Mdvx &input_file,
		      const DateTime &trigger_time) const;
  

};


#endif

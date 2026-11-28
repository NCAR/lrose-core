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
 * @file QPESum2Dsr.hh
 *
 * @class QPESum2Dsr
 *
 * QPESum2Dsr is the top level application class.
 *  
 * @date 7/27/2010
 *
 */

#ifndef QPESum2Dsr_HH
#define QPESum2Dsr_HH

#include <cstdio>
#include <string>

#include <dsdata/DsTrigger.hh>
#include <Fmq/DsRadarQueue.hh>
#include <rapformats/DsRadarMsg.hh>

#include "Args.hh"
#include "Params.hh"
#include "RadarFile.hh"

using namespace std;


/** 
 * @class QPESum2Dsr
 */

class QPESum2Dsr
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

  ~QPESum2Dsr(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   */

  static QPESum2Dsr *Inst(int argc, char **argv);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   */

  static QPESum2Dsr *Inst();
  

  /**
   * @brief Initialize the local data.
   *
   * @return Returns true if the initialization was successful, false otherwise.
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
   * @brief Value used to signify missing data in the scaled data buffer used
   *        for the radar message.
   */

  static const ui08 SCALED_MISSING_DATA_VALUE;
  

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static QPESum2Dsr *_instance;
  
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
   * @brief The data trigger object.
   */

  DsTrigger *_dataTrigger;
  
  /**
   * @brief The current volume number.  The volume number field in the
   *        radar file is always set to 0 so we need to keep track of this
   *        ourselves.
   */

  int _volNum;
  
  /**
   * @brief The output dsRadar FMQ.
   */

  DsRadarQueue _radarQueue;
  
  /**
   * @brief The radar message.  We keep a global copy since this message
   *        contains the radar parameter and field information.
   */

  DsRadarMsg _radarMsg;

  /**
   * @brief The issue time.  We keep a global copy so we know
   *        when a new volume is coming in.
   */

  time_t _lastIssueTime;

  /**
   * @brief Tilts in the volume.
   */

  int _numberOfTilts;

  /**
   * @brief Tilts in the volume.
   */

  int *_tilts;

  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   *
   * @note The constructor is private because this is a singleton object.
   */

  QPESum2Dsr(int argc, char **argv);
  

  /**
   * @brief Create the count field colorscale file.
   */
  
  void _createCountColorscale() const;
  

  /**
   * @brief Initialize the field params in the radar message.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initFieldParams();
  

  /**
   * @brief Initialize the radar params in the radar message.
   *
   * @params[in] radar_file The current radar file.
   */

  void _initRadarParams(const RadarFile &radar_file);
  

  /**
   * @brief Initialize the data trigger object.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initTrigger();
  

  /**
   * @brief Process data for the given time.
   *
   * @param[in] file_path The full path of the input file to process.
   * @param[in] issue_time The issue time of the input data.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processData(const string &file_path, const time_t &issue_time);
  

};


#endif

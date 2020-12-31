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
 * @file ComputeRefract.hh
 *
 * @class ComputeRefract
 *
 * ComputeRefract program object.
 *  
 * @date 12/1/2008
 *
 */

#ifndef ComputeRefract_HH
#define ComputeRefract_HH

#include "Processor.hh"
#include "Params.hh"
#include "CalibDayNight.hh"
#include <Refract/RefParms.hh>

class RefractInput;
class DsMdvx;

/** 
 * @class ComputeRefract
 */

class ComputeRefract
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

  virtual ~ComputeRefract(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @return Returns a pointer to the program instance.
   */

  static ComputeRefract *Inst(int argc, char **argv);

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the program instance.
   */

  static ComputeRefract *Inst();
  

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

  static ComputeRefract *_instance;
  
  /**
   * @brief Program name.
   */

  char *_progName;

  /**
   * @brief Parameter file paramters.
   */
  Params _params;
  RefParms _refparms;
  
  /**
   * @brief Triggering object
   */

  DsTrigger *_dataTrigger;
  
  /**
   * @brief The calibration files.
   */

  CalibDayNight _calib;
  
  /**
   * @brief Refractivity processor object
   */

  Processor _processor;
  
  // Global objects

  RefractInput *_inputHandler;
  

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

  ComputeRefract(int argc, char **argv);
  

  /**
   * @brief Process data for the given trigger time.
   *
   * @param trigger_time The current trigger time.  Data for this time
   *                     should be processed.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const DateTime &trigger_time);
  

  bool _readInputFile(DsMdvx &mdvx, const DateTime &data_time);
};


#endif

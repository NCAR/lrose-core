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
 * @file BuoyObs2Spdb.hh
 *
 * @class BuoyObs2Spdb
 *
 * BuoyObs2Spdb program object.
 *  
 * @date 7/25/2011
 *
 */

#ifndef BuoyObs2Spdb_HH
#define BuoyObs2Spdb_HH

#include <map>
#include <string>
#include <sys/time.h>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <rapformats/station_reports.h>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;

/** 
 * @class BuoyObs2Spdb
 */

class BuoyObs2Spdb
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

  ~BuoyObs2Spdb(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @return Returns a pointer to the BuoyObs2Spdb instance.
   */

  static BuoyObs2Spdb *Inst(int argc, char **argv);


  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the BuoyObs2Spdb instance.
   */

  static BuoyObs2Spdb *Inst();
  

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

  ///////////////////
  // Private types //
  ///////////////////

  typedef enum
  {
    QC_NO_CHECK = 0,
    QC_CORRECT = 1,
    QC_ERROR = 3,
    QC_MISSING = 9
  } qc_indicator_t;
  
    
  ///////////////////////
  // Private constants //
  ///////////////////////

  /**
   * @brief The size of the input buffer to use.
   */

  static const size_t INPUT_BUFFER_SIZE;
  
  /**
   * @brief The number of bytes in each line of the buoy observations.
   */

  static const size_t BUOY_OBS_RECORD_LEN;
  

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static BuoyObs2Spdb *_instance;
  
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
   * @brief List of processed buoys and their locations.
   */

  map< string, pair< double, double > > _buoyLocations;
  

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

  BuoyObs2Spdb(int argc, char **argv);
  

  /**
   * @brief Create the buoy locations map file.
   */

  void _createBuoyLocationsMapFile() const;
  

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
  

  /**
   * @brief Process the first line of the buoy observation.
   *
   * @param[in] buffer     The line read from the input file.
   * @param[in] line_num   The line number of the line, used for error messages.
   * @param[out] report    The updated station report information.
   *
   * @return Returns the number of records in the observation.
   */

  int _processObsLine1(const string &buffer,
		       const size_t line_num,
		       station_report_t &report) const;
  

  /**
   * @brief Process the second line of the buoy observation.
   *
   * @param[in] buffer     The line read from the input file.
   * @param[in] line_num   The line number of the line, used for error messages.
   * @param[out] report    The updated station report information.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processObsLine2(const string &buffer,
			const size_t line_num,
			station_report_t &report) const;
  

  /**
   * @brief Process the third line of the buoy observation.
   *
   * @param[in] buffer     The line read from the input file.
   * @param[in] line_num   The line number of the line, used for error messages.
   * @param[out] report    The updated station report information.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processObsLine3(const string &buffer,
			const size_t line_num,
			station_report_t &report) const;
  

  /**
   * @brief Process the fourth line of the buoy observation.
   *
   * @param[in] buffer     The line read from the input file.
   * @param[in] line_num   The line number of the line, used for error messages.
   * @param[out] report    The updated station report information.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processObsLine4(const string &buffer,
			const size_t line_num,
			station_report_t &report) const;
  

};


#endif

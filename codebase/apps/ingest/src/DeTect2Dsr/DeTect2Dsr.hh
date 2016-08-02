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
 * @file DeTect2Dsr.hh
 *
 * @class DeTect2Dsr
 *
 * DeTect2Dsr is the top level application class.
 *  
 * @date 7/7/2010
 *
 */

#ifndef DeTect2Dsr_HH
#define DeTect2Dsr_HH

#include <cstdio>
#include <string>

#include <DeTect/DataObject.hh>
#include <dsdata/DsTrigger.hh>
#include <Fmq/DsRadarQueue.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


/** 
 * @class DeTect2Dsr
 */

class DeTect2Dsr
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

  ~DeTect2Dsr();
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   */

  static DeTect2Dsr *Inst(int argc, char **argv);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   */

  static DeTect2Dsr *Inst();
  

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
   * @brief The maximum valid count value.
   */

  static const int MAX_COUNT;
  

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static DeTect2Dsr *_instance;
  
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
   * @brief List of times to process.  These times are expected to be specified
   *        by the user in chronological order so we will operate using that
   *        expectation.
   */

  vector< pair< DateTime, DateTime> > _timeList;
  
  /**
   * @brief Index of the current time range in effect.
   */

  int _currentTimeRangeIndex;
  
  /**
   * @brief Look-up table for converting the count values to power.
   */

  int *_countToPower;

  /**
   * @brief The scale used to scale the power value.  The scaling of the values
   *        is done in the _countToPower table.
   */

  double _powerScale;
  
  /**
   * @brief The bias used to scale the power value.  The scaling of the values
   *        is done in the _countToPower table.
   */

  double _powerBias;
  
  /**
   * @brief The current volume number.  There is no volume number included
   *        in the data, so we will increment the volume number every time
   *        we get back to the 0 degree azimuth.
   */

  int _volumeNum;
  
  /**
   * @brief The azimuth angle of the previous beam.  We need to keep track
   *        of this so we can trigger an end-of-volume/end-of-tilt whenever
   *        we pass the 0 degree azimuth.
   */

  double _prevAzimuth;
  
  /**
   * @brief The output dsRadar FMQ.
   */

  DsRadarQueue _radarQueue;
  
  /**
   * @brief The time for the first beam processed.
   */

  DateTime _firstBeamTime;
  
  /**
   * @brief The time for the last beam processed.
   */

  DateTime _lastBeamTime;
  

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

  DeTect2Dsr(int argc, char **argv);
  

  /**
   * @brief Create the count field colorscale file.
   */
  
  void _createCountColorscale() const;
  

  /**
   * @brief Initialize the count-to-power look-up table.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initCountToPower();
  

  /**
   * @brief Initialize the time list.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initTimeList();
  

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
   *
   * @return Returns true on success, false on failure.
   */

  bool _processData(const string &file_path);
  

  void _processDataObject(const DataObject &data_object);
  

};


#endif

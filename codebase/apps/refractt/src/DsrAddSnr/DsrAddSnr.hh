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
 * @file DsrAddSnr.hh
 *
 * @class DsrAddSnr
 *
 * DsrAddSnr is the top level application class.
 *  
 * @date 3/15/2010
 *
 */

#ifndef DsrAddSnr_HH
#define DsrAddSnr_HH

#include <string>
#include <sys/time.h>
#include <vector>

#include <Fmq/DsRadarQueue.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


/** 
 * @class DsrAddSnr
 */

class DsrAddSnr
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

  ~DsrAddSnr(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   */

  static DsrAddSnr *Inst(int argc, char **argv);


  /**
   * @brief Retrieve the singleton instance of this class.
   */

  static DsrAddSnr *Inst();
  

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
   * @brief The SNR field name.
   */

  static const string SNR_FIELD_NAME;
  
  /**
   * @brief The SNR field units.
   */

  static const string SNR_UNITS;
  
  /**
   * @brief Minimum SNR value.
   */

  static const float MIN_SNR_VALUE;
  
  /**
   * @brief Maximum SNR value.
   */

  static const float MAX_SNR_VALUE;
  
  /**
   * @brief Value to use to signal missing data in the output file.
   */

  static const double MISSING_DATA_VALUE;
  

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer
   */

  static DsrAddSnr *_instance;
  
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
   * @brief The input FMQ.
   */

  DsRadarQueue _inputQueue;
  
  /**
   * @brief The output FMQ.
   */

  DsRadarQueue _outputQueue;
  
  /**
   * @brief The index of the power field in the input queue.
   */

  int _dmFieldIndex;

  int _snrNumBytes;
  float _snrScale;
  float _snrBias;
  int _snrScaledMissingValue;
  
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

  DsrAddSnr(int argc, char **argv);
  

  /**
   * @brief Add the SNR field to the field params in the message.
   *
   * @param[in,out] msg The message.
   *
   * @return Returns true on success, false on failure.
   */

  bool _addSnrFieldParams(DsRadarMsg &msg);
  

  /**
   * @breif Add the SNR field to the beam data in the message.
   *
   * @param[in,out] msg The message
   *
   * @return Returns true on success, false on failure.
   */

  bool _addSnrField(DsRadarMsg &msg) const;
  

  /**
   * @brief Calculate the SNR value.
   *
   * @param[in] dm_data The power data.
   * @param[out] snr_data The calculated SNR data.
   * @param[in] num_gates The number of gates in the data arrays.
   */

  void _calcSnr(const float *dm_data, float *snr_data,
		const int num_gates) const;
  

};


#endif

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
 * @file MdvAddSnr.hh
 *
 * @class MdvAddSnr
 *
 * MdvAddSnr is the top level application class.
 *  
 * @date 3/11/2010
 *
 */

#ifndef MdvAddSnr_HH
#define MdvAddSnr_HH

#include <string>
#include <sys/time.h>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


/** 
 * @class MdvAddSnr
 */

class MdvAddSnr
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

  ~MdvAddSnr(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   */

  static MdvAddSnr *Inst(int argc, char **argv);


  /**
   * @brief Retrieve the singleton instance of this class.
   */

  static MdvAddSnr *Inst();
  

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
   * @brief Value to use to signal missing data in the output file.
   */

  static const double MISSING_DATA_VALUE;
  
  /**
   * @brief Very large value.
   */

  static const double VERY_LARGE;
  

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer
   */

  static MdvAddSnr *_instance;
  
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
   * @param[in] argv Command line arguments.
   *
   * @note The constructor is private because this is a singleton object.
   */

  MdvAddSnr(int argc, char **argv);
  

  /**
   * @brief Add the SNR field to the given file.
   *
   * @param[in,out] mdvx The file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _addSnrField(DsMdvx &mdvx) const;
  

  /**
   * @brief Calculate SNR from the given fields.
   *
   * @param[in] dm_field The power field.
   * @param[out] snr_field The calculated SNR field.
   *
   * @return Returns true on success, false on failure.
   */

  bool _calcSnr(const MdvxField &dm_field,
		MdvxField &snr_field) const;
  

  /**
   * @brief Create the blank SNR field.
   *
   * @param[in] base_field_hdr Base field header.
   * @param[in] base_vlevel_hdr Base vlevel header.
   * @param[in] field_name The SNR field name.
   *
   * @return Returns a pointer to the SNR field on success,
   *         0 on failure.
   */

  MdvxField *_createBlankSnrField(const Mdvx::field_header_t base_field_hdr,
				  const Mdvx::vlevel_header_t base_vlevel_hdr,
				  const string &field_name) const;
  

  /**
   * @brief Initialize the data trigger.
   *
   * Returns true on success, false on failure.
   */

  bool _initTrigger(void);
  

  /**
   * @brief Process data for the given trigger time.
   *
   * @param trigger_time The current trigger time.  Data for this time
   *                     should be processed.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const DateTime &trigger_time);
  

  /**
   * @brief Read the input file.
   *
   * @param[out] mdvx The input file.
   * @param[in] data_time The desired data time.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readInputFile(DsMdvx & mdvx,
		      const DateTime &data_time) const;
  

  /**
   * @brief Update the master header in the output file.
   *
   * @param[in,out] mdvx Output MDV file.
   */

  void _updateMasterHeader(DsMdvx &mdvx) const;
  
};


#endif

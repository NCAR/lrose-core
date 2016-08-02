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
 * @file MdvRemapNormalize.hh
 *
 * @class MdvRemapNormalize
 *
 * MdvRemapNormalize is the top level application class.
 *  
 * @date 11/20/2002
 *
 */

#ifndef MdvRemapNormalize_HH
#define MdvRemapNormalize_HH

#include <string>
#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


/** 
 * @class MdvRemapNormalize
 */

class MdvRemapNormalize
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

  ~MdvRemapNormalize(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   */

  static MdvRemapNormalize *Inst(int argc, char **argv);

  /**
   * @brief Retrieve the singleton instance of this class.
   */

  static MdvRemapNormalize *Inst();
  

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

  static MdvRemapNormalize *_instance;
  
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
  

  /**
   * @brief Output projection.
   */

  MdvxPjg _outputProj;
  
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

  MdvRemapNormalize(int argc, char **argv);
  

  /**
   * @brief Initialize the output trigger
   *
   * @return Returns true on success, false on failure.
   */

  bool _initOutputProj(void);
  

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
  

  /**
   * @brief Remap the data in the input file and store it in the output
   *        file.
   *
   * @param[in] input_file The input MDV file.
   * @param[out] output_file The output MDV file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _remapData(const DsMdvx &input_file,
		  DsMdvx &output_file) const;
  

  /**
   * @brief Create a new MDV field by remapping the input field using the
   *        previously defined output projection.
   *
   * @param[in] input_field The input field.
   *
   * @return Returns a pointer to the remapped field on success, 0 on failure.
   */

  MdvxField *_remapField(const MdvxField &input_field) const;
  

  /**
   * @brief Set the master header in the output file based on field values
   *        in the input file master header.
   *
   * @param[in,out] output_file Output file.
   * @param[in] input_master_hdr Input file master header.
   */

  void _setMasterHeader(DsMdvx &output_file,
			const Mdvx::master_header_t &input_master_hdr) const;
  

};


#endif

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
 * @file Hrrr15minNcf2Mdv.hh
 *
 * @class Hrrr15minNcf2Mdv
 *
 * Hrrr15minNcf2Mdv program object.
 *  
 * @date 3/30/2009
 *
 */

#ifndef Hrrr15minNcf2Mdv_HH
#define Hrrr15minNcf2Mdv_HH

#include <string>
#include <sys/time.h>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>

#include "Args.hh"
#include "Params.hh"
#include "NetcdfField.hh"
#include "NetcdfFile.hh"

using namespace std;

/** 
 * @class Hrrr15minNcf2Mdv
 */

class Hrrr15minNcf2Mdv
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

  ~Hrrr15minNcf2Mdv(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @return Returns a pointer to the class singleton instance.
   */

  static Hrrr15minNcf2Mdv *Inst(int argc, char **argv);


  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the class singleton instance.
   */

  static Hrrr15minNcf2Mdv *Inst();
  

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

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static Hrrr15minNcf2Mdv *_instance;
  
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
   * @brief List of fields to process.
   */

  vector< NetcdfField* > _fieldList;
  
  /**
   * @brief Type of compression to use for the MDV fields when writing
   *        them to the output files.
   */

  Mdvx::compression_type_t _outputCompressionType;
  
  /**
   * @brief Type of scaling to use for the MDV fields when writing
   *        them to the output files.
   */

  Mdvx::scaling_type_t _outputScalingType;
  

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

  Hrrr15minNcf2Mdv(int argc, char **argv);
  

  /**
   * @brief Initialize the field list.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initFieldList();
  

  /**
   * @brief Initialize the output compression type.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initOutputCompressionType();
  

  /**
   * @brief Initialize the output scaling type.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initOutputScalingType();
  

  /**
   * @brief Initialize the data trigger.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initTrigger();
  

  /**
   * @brief Process the given file.
   *
   * @param[in] input_file_path Path of input file to process.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processFile(const string &input_file_path);
  

};


#endif

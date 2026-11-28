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
 * @file JmaMtSatHRIT2Mdv.hh
 *
 * @class JmaMtSatHRIT2Mdv
 *
 * JmaMtSatHRIT2Mdv is the top level application class.
 *  
 * @date 1/30/2012
 *
 */

#ifndef JmaMtSatHRIT2Mdv_HH
#define JmaMtSatHRIT2Mdv_HH

#include <cstdio>
#include <string>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <hrit/HRITFile.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


/** 
 * @class JmaMtSatHRIT2Mdv
 */

class JmaMtSatHRIT2Mdv
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

  ~JmaMtSatHRIT2Mdv(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   */

  static JmaMtSatHRIT2Mdv *Inst(int argc, char **argv);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   */

  static JmaMtSatHRIT2Mdv *Inst();
  

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

  ///////////////////
  // Private types //
  ///////////////////

  typedef struct
  {
    string url;
    MdvxProj proj;
  } domain_info_t;
  
    
  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static JmaMtSatHRIT2Mdv *_instance;
  
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
   * @brief List of domains we need to process.
   */

  vector< domain_info_t > _outputDomains;
  
  /**
   * @brief The vertical level type to use in the output file.
   */

  Mdvx::vlevel_type_t _outputVlevelType;
  

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

  JmaMtSatHRIT2Mdv(int argc, char **argv);
  

  /**
   * @brief Convert the units of the data in the given field using the
   *        conversion specified in the parameter file.
   *
   * @param[in,out] field    The field to convert.
   */

  void _convertUnits(MdvxField &field) const;


  /**
   * @brief Create a blank field using the given parameters.
   *
   * @param[in] field_name     The name of the field.
   * @param[in] units          The units for the field.
   * @param[in] proj           The field projection.
   *
   * @return Returns a pointer to the new field on success, 0 on failure.
   */

  MdvxField *_createField(const string &field_name,
			  const string &units,
			  const MdvxProj &proj);
  

  /**
   * @brief Initialize the list of output domains.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initDomains();
  

  /**
   * @brief Initialize the output vertical level type.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initOutputVlevelType();
  

  /**
   * @brief Initialize the program trigger object.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initTrigger();
  

  /**
   * @brief Process data for the given trigger.
   *
   * @param[in] trigger_info    The trigger information.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processData(const TriggerInfo &trigger_info);
  

  /**
   * @brief Read the HRIT files associated with the trigger file and combine
   *        them into a single HRIT file.
   *
   * @param[in]  trigger_file_path     The trigger file path.
   * @param[out] combined_hrit_file    The combined HRIT file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readHritFiles(const string &trigger_file_path,
		      HRITFile &combined_hrit_file) const;
  
  /**
   * @brief Update the master header in the output file.
   *
   * @param[in,out] mdvx      The output MDV file.
   * @param[in]     hrif_file The HRIF file we are processing.
   */

  void _updateMasterHeader(DsMdvx &mdvx, const HRITFile &hrif_file);
  
};


#endif

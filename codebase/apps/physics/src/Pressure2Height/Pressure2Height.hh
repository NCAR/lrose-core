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
 * @file Pressure2Height.hh
 *
 * @class Pressure2Height
 *
 * Pressure2Height is the top level application class.
 *  
 * @date 11/30/2010
 *
 */

#ifndef Pressure2Height_HH
#define Pressure2Height_HH

#include <string>
#include <sys/time.h>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


/** 
 * @class Pressure2Height
 */

class Pressure2Height
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

  ~Pressure2Height(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   */

  static Pressure2Height *Inst(int argc, char **argv);


  /**
   * @brief Retrieve the singleton instance of this class.
   */

  static Pressure2Height *Inst();
  

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
   * @brief Value used to flag missing heights.
   */

  static const double MISSING_HEIGHT_VALUE;
  

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer
   */

  static Pressure2Height *_instance;
  
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

  Pressure2Height(int argc, char **argv);
  

  /**
   * @brief Convert the given input field from pressure to height.
   *
   * @param[in,out] input_field The input field to be converted.
   * @param[in] temp_field The 3D temperature field used for the conversion.
   * @param[in] output_field_name The field name for the converted field.
   * @param[in] output_field_name_long The long field name for the converted
   *                                   field.
   *
   * @return Returns true on success, false on failure.
   */

  bool _convertField(MdvxField &input_field,
		     const MdvxField &temp_field,
		     const string &output_field_name,
		     const string &output_field_name_long) const;
  

  /**
   * @brief Initialize the data trigger.
   *
   * Returns true on success, false on failure.
   */

  bool _initTrigger(void);
  

  /**
   * @brief Convert the given pressure value to the appropriate height value
   *        using hypsometric interpolation.
   *
   * @param[in] pres Pressure value in mb.
   * @param[in] temp Temperature value in K.
   *
   * @return Returns the associated height value in Kft on success,
   *         MISSING_HEIGHT_VALUE on failure.
   */

  double _pressure2Height(const double pres,
			  const double temp) const;
  

  /**
   * @brief Process data for the given trigger time.
   *
   * @param trigger_time The current trigger time.  Data for this time
   *                     should be processed.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const TriggerInfo &trigger_info);
  

  /**
   * @brief Read the indicated field.
   *
   * @param[in] field_info Information describing the field.
   * @param[in] data_time Desired data time.
   * @param[in] proj Desired input projection.
   *
   * @return Returns a pointer to the read field on success, 0 on failure.
   */

  MdvxField *_readModelField(Params::model_field_info_t &field_info,
			     const DateTime &data_time,
			     const DateTime &fcst_time,
			     const MdvxProj &proj) const;
  

  /**
   * @brief Read the input file.
   *
   * @param[out] mdvx Input file.
   * @param[in] data_time Desired data time.
   *
   * @return Returns true on sucess, false on failure.
   */

  bool _readInputFile(DsMdvx &mdvx,
		      const DateTime &data_time,
		      const DateTime &fcst_time) const;
  

  /**
   * @brief Update the master header in the output file.
   *
   * @param[in,out] mdvx Output MDV file.
   */

  void _updateMasterHeader(DsMdvx &mdvx) const;

  
  /**
   * @brief Write the output file.
   *
   * @param[in] mdvx The output file.
   * @param[in] url The output url.
   *
   * @return Returns true on success, false on failure.
   */

  bool _writeFile(DsMdvx &mdvx,
		  const string &url) const;
  

};


#endif

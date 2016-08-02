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
 * @file Sigmet2Mdv.hh
 *
 * @class Sigmet2Mdv
 *
 * Sigmet2Mdv program object.
 *  
 * @date 6/7/2010
 *
 */

#ifndef Sigmet2Mdv_HH
#define Sigmet2Mdv_HH

#include <string>
#include <sys/time.h>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <rapformats/SigAirMet.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;

/** 
 * @class Sigmet2Mdv
 */

class Sigmet2Mdv
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

  ~Sigmet2Mdv(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @return Returns a pointer to the class singleton instance.
   */

  static Sigmet2Mdv *Inst(int argc, char **argv);


  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the class singleton instance.
   */

  static Sigmet2Mdv *Inst();
  

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

  static Sigmet2Mdv *_instance;
  
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
   * @note Private because this is a singleton object.
   */

  Sigmet2Mdv(int argc, char **argv);
  

  /**
   * @brief Add the given SIGMET to the grid.
   *
   * @param[in] sigmet The SIGMET to add.
   * @param[in,out] grid The grid.
   */

  void _addSigmetToGrid(const SigAirMet &sigmet,
			ui08 *grid) const;
  

  /**
   * @brief Create an blank MDV SIGMET field.
   *
   * @return Returns a pointer to the MDV field on success, 0 on failure.
   */

  MdvxField *_createBlankSigmetField() const;
  

  /**
   * @brief Create an MDV field containing the given list of sigmets.
   *
   * @param[in] sigmets The list of sigmets to include in the gridded field.
   *
   * @return Returns a pointer to the MDV field on success, 0 on failure.
   */

  MdvxField *_createSigmetField(const vector< SigAirMet > &sigmets) const;
  

  /**
   * @brief Initialize the output projection.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initOutputProj();
  

  /**
   * @brief Initialize the data trigger.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initTrigger();
  

  /**
   * @brief Process data for the given time.
   *
   * @param[in] trigger_time Trigger time.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processData(const DateTime &trigger_time);
  

  /**
   * @brief Read the SIGMETs from the database.
   *
   * @param[in] data_time Desired data time.
   * @param[out] sigmets The sigmets from the database.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readSigmets(const DateTime &data_time,
		    vector< SigAirMet > & sigmets) const;
  

  /**
   * @brief Write the given SIGMET field to the output file.
   *
   * @param[in] sigmet_field The SIGMET field.  This pointer is no longer
   *                         valid when this method returns.
   * @param[in] data_time The time for the data in the field.
   *
   * @return Returns true on success, false on failure.
   */

  bool _writeOutputFile(MdvxField *sigmet_field,
			const DateTime &data_time) const;
  

};


#endif

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
 * @file MdvExtractPoint.hh
 *
 * @class MdvExtractPoint
 *
 * MdvExtractPoint program object.
 *  
 * @date 8/4/2009
 *
 */

#ifndef MdvExtractPoint_HH
#define MdvExtractPoint_HH

#include <string>
#include <sys/time.h>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include "Args.hh"
#include "Params.hh"
#include "Output.hh"

using namespace std;

/** 
 * @class MdvExtractPoint
 */

class MdvExtractPoint
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

  ~MdvExtractPoint(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @return Returns a pointer to the class singleton instance.
   */

  static MdvExtractPoint *Inst(int argc, char **argv);


  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the class singleton instance.
   */

  static MdvExtractPoint *Inst();
  

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

  static MdvExtractPoint *_instance;
  
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
   * @brief Output handler object.
   */

  Output *_outputHandler;
  

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

  MdvExtractPoint(int argc, char **argv);
  

  /**
   * @brief Initialize the output handler object.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initOutputHandler();
  

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
   * @brief Process the list of points in the given field.
   *
   * @param[in] field Data field.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processPoints(const MdvxField &field) const;
  

  /**
   * @brief Read the input field for the specified time.
   *
   * @param[in] data_time Data time for input field.
   *
   * @return Returns a pointer to the input field on success, 0 on failure.
   */

  MdvxField *_readInputField(const DateTime &data_time) const;
  

  ////////////
  // Macros //
  ////////////

  static double _setValue(const fl32 *data,
			  const fl32 missing_value, const fl32 bad_value,
			  const int x_index, const int y_index,
			  const int z_index,
			  const int nx, const int ny, const int nz)
  {
    if (x_index < 0 || x_index >= nx ||
	y_index < 0 || y_index >= ny)
      return Output::MISSING_VALUE;
    
    int index = x_index + (y_index * nx) + (z_index * nx * ny);
    
    if (data[index] == missing_value ||
	data[index] == bad_value)
      return Output::MISSING_VALUE;
    
    return data[index];
  }
  
};


#endif

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
 * @file QPESumBinary2Mdv.hh
 *
 * @class QPESumBinary2Mdv
 *
 * QPESumBinary2Mdv is the top level application class.
 *  
 * @date 5/07/2012
 *
 */

#ifndef QPESumBinary2Mdv_HH
#define QPESumBinary2Mdv_HH

#include <dataport/port_types.h>
#include <dsdata/DsTrigger.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


class QPESumBinary2Mdv
{
 public:

  /**
   * @brief Destructior
   */
  
  ~QPESumBinary2Mdv(void);
  
  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] arc Number of command line arguments
   * @param[in] arv Command line arguments
   */

  static QPESumBinary2Mdv *Inst(int argc, char **argv);

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   */

  static QPESumBinary2Mdv *Inst();

  /**
   * @brief Initialize the local data.
   *
   * @return Returns true if the initialization was successful, false otherwise.
   */

  bool init();

  
  /**
   * @brief Run the program.
   */

  void run();
  
  // Flag indicating whether the program status is currently okay.


  /**
   * @brief Flag indicating whether the program status is currently okay.
   */

  bool okay;
  
 private:

  /**
   * @brief Constructor
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   *
   * @note The constructor is private because this is a singleton object.
   */

  QPESumBinary2Mdv(int argc, char **argv);

  /**
   * @brief Singleton instance pointer
   */

  static QPESumBinary2Mdv *_instance;
  
  // Define private null copy constructor and assignment operator so the
  // default ones are not accidentally used.

  QPESumBinary2Mdv(const QPESumBinary2Mdv & other);
  QPESumBinary2Mdv & operator= (const QPESumBinary2Mdv & other);

  char _varname[20];
  char _varunit[6];
  string _short_varname;
  string _units;
  string _field_name;
  string _field_name_long;
  
  float _minx, _miny,_minz, _dx, _dy, _dz, _missingDataValue;
  int  _nx, _ny, _nz, _year, _month, _day, _hour, _minute, _second;
  
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
   * @brief Writes out MDV data file
   *
   * @param[in] input_file_name Name of input file used to provide the data.
   * @param[in] data Passing in a pointer to the data.
   *
   * @return Return 0 on successful, -1 if not.
   */

  int _writeMDV(const string &input_file_name, fl32 *data);


  /**
   * @brief Processes the input file
   *
   * @param[in] file_path Input file path
   *
   * @return Returns true on success, and false otherwise
   */

  bool _processData(const string &file_path);

  /**
   * @brief Reads the input file
   *
   * @param[in] filename Input file name
   * @param[in] data Passing in pointer to data
   *
   * @return Returns true on success, and false otherwise
   */

  bool _readFile(const string &filename, fl32 *data);

  /**
   * @brief Gets the data and time from the file name
   *
   * @param[in] filename Input file name
   *
   * @return Returns a DateTime object.
   */

  DateTime _getDataTime(const string &input_file_name);
   
  
};

#endif

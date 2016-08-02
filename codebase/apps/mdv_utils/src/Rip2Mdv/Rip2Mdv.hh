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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/04 02:22:13 $
 *   $Id: Rip2Mdv.hh,v 1.3 2016/03/04 02:22:13 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Rip2Mdv: Rip2Mdv program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Rip2Mdv_HH
#define Rip2Mdv_HH

#include <string>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <Mdv/MdvxField.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


class Rip2Mdv
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  // Flag indicating whether the program status is currently okay.

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Destructor
   */

  ~Rip2Mdv(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static Rip2Mdv *Inst(int argc, char **argv);
  static Rip2Mdv *Inst();
  

  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /*********************************************************************
   * run() - run the program.
   */

  void run();
  

 private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  static const size_t VAR_DESC_LEN;
  static const size_t UNITS_LEN;
  static const size_t CHRIP_LEN;
  static const size_t CHRIP_NUM_ELEMENTS;
  

  ///////////////////
  // Private types //
  ///////////////////

  typedef struct
  {
    si32 map_projection;
    si32 coarse_ny;
    si32 coarse_nx;
    si32 ny;
    si32 nx;
    si32 dimensions;
    si32 grid_domain;
    si32 unknown;
    si32 nz;
    si32 mdateb;
    si32 mdate;
    si32 ice_physics_type;
    si32 vert_coord_type;
    si32 landuse_dataset;
    si32 spare[18];
  } ihrip_t;
  

  typedef struct
  {
    fl32 first_true_lat;
    fl32 second_true_lat;
    fl32 coarse_central_lat;
    fl32 coarse_central_lon;
    fl32 coarse_dxy_km;
    fl32 dxy_km;
    fl32 coarse_ll_y;
    fl32 coarse_ll_x;
    fl32 unknown[4];
    fl32 rhourb;
    fl32 rhour;
    fl32 xtime;
    fl32 spare[17];
  } rhrip_t;

  
  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static Rip2Mdv *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Triggering object

  DsTrigger *_dataTrigger;
  
  // MDV field containing RIP file information

  MdvxField *_field;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  Rip2Mdv(int argc, char **argv);
  

  /*********************************************************************
   * _getUtime() - Convert the given RIP time value to UNIX time.
   *
   * Returns the UNIX time equivalent for the given RIP time.
   */

  inline static time_t _getUtime(const int rip_time)
  {
    int year = rip_time / 1000000;
    int month = (rip_time % 1000000) / 10000;
    int day = (rip_time % 10000) / 100;
    int hour = rip_time % 100;
    
    if (year > 69) year += 1900;
    else year += 2000;
    
    DateTime time_obj(year, month, day, hour);
    
    return time_obj.utime();
  }
  

  /*********************************************************************
   * _initTrigger() - Initialize the data trigger.
   *
   * Returns true on success, false on failure.
   */

  bool _initTrigger(void);
  

  /*********************************************************************
   * _processData() - Process the data in the given file.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const string &file_path);
  

  /*********************************************************************
   * _readRipData() - Read the RIP data from the given file.  Update the
   *                  data in the MDV field with the data values read in.
   *
   * Returns true on success, false on failure.
   */

  bool _readRipData(FILE *rip_file, const string &rip_file_path);
  

  /*********************************************************************
   * _readRipHeader() - Read the RIP header information from the given
   *                    file.  Create the MDV field based on this information.
   *
   * Returns true on success, false on failure.
   */

  bool _readRipHeader(FILE *rip_file, const string &rip_file_path);
  

  /*********************************************************************
   * _writeMdvFile() - Write the MDV file.
   *
   * Returns true on success, false on failure.
   */

  bool _writeMdvFile(const string &rip_file_path);
  

};


#endif

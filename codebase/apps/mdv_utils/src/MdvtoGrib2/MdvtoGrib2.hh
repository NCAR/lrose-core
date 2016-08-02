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
 *   $Date: 2016/03/04 02:22:13 $
 *   $Revision: 1.6 $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MdvtoGrib2: MdvtoGrib2 program object.
 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Sept 2006
 *
 ************************************************************************/

#ifndef MdvtoGrib2_HH
#define MdvtoGrib2_HH

#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <dsdata/TriggerInfo.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/DateTime.hh>
#include <grib2/Grib2File.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


class MdvtoGrib2
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

  ~MdvtoGrib2(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static MdvtoGrib2 *Inst(int argc, char **argv);
  static MdvtoGrib2 *Inst();
  

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

  int run();
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static MdvtoGrib2 *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Input trigger object

  DsTrigger *_dataTrigger;

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  MdvtoGrib2(int argc, char **argv);
  

  /*********************************************************************
   * _convertMdvtoGrib2Level() - Convert the given MDV vertical level info
   *                           to the equivalent GRIB level info.
   */

  static bool _convertMdvtoGrib2LevelType(const int mdv_level_type,
					  const double mdv_level_value,
					  int &firstSurfaceType,
					  int &secondSurfaceType,
					  double &firstValue,
					  double &secondValue);  

  /*********************************************************************
   * _scaleFactorValue() - Convert the given value into a integer with
   *                        a scale factor.
   */

  void _scaleFactorValue(double &value, int &scaleFactor);


  /*********************************************************************
   * _processData() - Process the data for the given time.
   */

  bool _processData(TriggerInfo &trigger_info);
  

  /*********************************************************************
   * _readMdvFile() - Read the MDV file for the given time.
   */

  bool _readMdvFile(DsMdvx &input_mdv,
		    TriggerInfo &trigger_info) const;

  /*********************************************************************
   * _createGDSTemplate() - Greates a GribProjection based on the MdvxField
   *                - sets gribProj == NULL if this projection will
   *                  be the exact same as the lastGribProj.
   *                - Returns GribProjectionTemplate number.
   */

  int _createGDSTemplate(MdvxField *field, int lastGridDefNum, Grib2::GribProj *lastGribProj,
			 Grib2::GribProj **gribProj);

  /*********************************************************************
   * _createPDSTemplate() - Greates a ProductDefTemplate based on the MdvxField
   *                        and information passed through params file.
   *                        Returns ProductDefTemplate number.
   */

  int _createPDSTemplate(MdvxField *field, int field_num, int z, time_t reference_time, 
			 int file_data_type, Grib2::ProdDefTemp **prodDefTemplate);

};


#endif

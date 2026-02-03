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
 *   $Date: 2016/03/07 18:17:26 $
 *   $Id: CalcMoisture.hh,v 1.3 2016/03/07 18:17:26 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * CalcMoisture: CalcMoisture program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2006
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef CalcMoisture_HH
#define CalcMoisture_HH

#include <string>
#include <Mdv/MdvxField.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"
class DsInputPath;

using namespace std;

class CalcMoisture
{
 public:

  // Flag indicating whether the program status is currently okay.

  bool okay;

  // Constructors/Destructor
  
  CalcMoisture(int argc, char **argv);
  virtual ~CalcMoisture();

  // Run the app
  
  int run();
  
private:

  string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;
  
  // data trigger

  DsInputPath *_input;

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * _processData() - Process data for the given trigger time.
   *
   * Returns 0 on success, -1 on failure.
   */
  
  int _processData(const string filePath,
                   const DateTime &fileTime);
  
  /*********************************************************************
   * _calcDewPointField() - Calculate the dew point field from the given
   *                        data.
   *
   * Returns a pointer to the dew point field on success, 0 on failure.
   */

  MdvxField *_calcDewPointField(const MdvxField &e_field);

  /*********************************************************************
   * _calcDewPointField() - Calculate the relative humidity field from
   *                        the given dewpoint data.
   *
   * Returns a pointer to the RH field on success, 0 on failure.
   */

  MdvxField *_calcRelativeHumidityField(const MdvxField &dp_field,
                                        const double temp_k);

  /*********************************************************************
   * _calcWaterVaporField() - Calculate the water vapor field from the
   *                          given data.
   *
   * Returns a pointer to the water vapor field on success, 0 on failure.
   */

  MdvxField *_calcWaterVaporField(const MdvxField &n_field,
				  const double temp_k,
				  const double press_mb);

  /*********************************************************************
   * _getTempPress() - Get the mean temperature (in K) and pressure (in mb)
   *                   values using the stations specifiec in the parameter
   *                   file.
   *
   * Returns 0 on success, -1 on failure.
   */

  int _getTempPress(const DateTime &data_time,
                    double &temp_k, double &press_mb);
  

};


#endif

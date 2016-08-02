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
 *   $Date: 2016/03/03 19:23:21 $
 *   $Id: AdiabatTempLookupTable.hh,v 1.5 2016/03/03 19:23:21 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * AdiabatTempLookupTable : Class controlling the lookup table for
 *                          obtaining temperature in K on a pseudoadiabat,
 *                          given pressure in hPa and theta_e in K.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2001
 *
 * Nancy Rehak
 * Based on RIP code from the MM5 model.
 *
 ************************************************************************/

#ifndef AdiabatTempLookupTable_HH
#define AdiabatTempLookupTable_HH



#include <string>
using namespace std;


class AdiabatTempLookupTable
{
 public:

  //////////////////////
  // Public constants //
  //////////////////////

  static const double MISSING_DATA_VALUE;
  

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructor
   */

  AdiabatTempLookupTable(const string &lookup_filename = "");
  

  /*********************************************************************
   * Destructor
   */

  ~AdiabatTempLookupTable(void);
  

  /*********************************************************************
   * setFilename() - Sets the lookup table filename to the given value.
   */

  void setFilename(const string &lookup_filename)
  {
    _lookupFilename = lookup_filename;
    _lookupGenerated = false;
  }
  

  /*********************************************************************
   * getTemperature() - Retrieves the temperature on a pseudoadiabat
   *                    given pressure in hPa and theta_e in K.
   *
   * Returns the calculated temperature value from the table on success.
   * Returns MISSING_DATA_VALUE if there is an error calculating the value.
   */

  double getTemperature(const double pressure,
			const double theta_e);
  

 private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  static const int NUM_COMMENT_LINES;
  

  /////////////////////
  // Private members //
  /////////////////////

  string _lookupFilename;
  
  bool _lookupGenerated;
  
  unsigned int _numPressureLevels;
  unsigned int _numThetaELevels;
  
  double *_pressureLevels;
  double *_thetaELevels;
  double *_lookupTable;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * _calcIndex() - Calculate the index in the lookup table based on
   *                the pressure level index and the theta_e level index.
   */

  inline int _calcIndex(const unsigned int pressure_index,
			const unsigned int theta_e_index)
  {
    if (pressure_index >= _numPressureLevels ||
	theta_e_index >= _numThetaELevels)
      return -1;
    
    return (pressure_index * _numThetaELevels) + theta_e_index;
  }
  

  /*********************************************************************
   * _loadTableFromFile() - Load the lookup table from the file.
   *
   * Returns true if the table was loaded successfully, false otherwise.
   */

  bool _loadTableFromFile();

  void _freeArrays();
  

};


#endif

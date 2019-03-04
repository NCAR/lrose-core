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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2019/03/04 00:13:38 $
//   $Id: InputVariable.cc,v 1.3 2019/03/04 00:13:38 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * InputVariable.cc: class implementing InputVariables used with netCDF files
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2003
 *
 * Kay Levesque
 *
 *********************************************************************/

#include <rapformats/station_reports.h>
#include "InputVariable.hh"

using namespace std;


/**********************************************************************
 * Constructors
 */

InputVariable::InputVariable(const string &variableName, const bool debug):
  _debug(debug),
  _checkForMissing(false),
  _missingValue(0.0),
  _totalNumValues(0.0),
  _variableName(variableName),
  _missingValueAttName(""),
  _ncVarPtr(0),
  _converter(new Converter()),
  _isInitialized(false)
{
  
}


/**********************************************************************
 * Destructor
 */

InputVariable::~InputVariable(void)
{
  delete _converter;
}
  

/**********************************************************************
 * init() - initialize the InputVariable object
 */

bool InputVariable::init(const Nc3File &fileObject)
{
  if(_debug)
    cerr << "InputVariable::init(): Initializing variable: " 
	 << _variableName.c_str() << endl;
  _isInitialized = false;

  if ((_ncVarPtr = fileObject.get_var(_variableName.c_str())) == 0)
    {
      cerr << "WARNING! Variable not found in netCDF file: " 
	   << _variableName << endl;
      return false;
    }

  _checkForMissing = false;

  if (_missingValueAttName != "")
    {
      cerr << "missing valAttName: " << _missingValueAttName.c_str() << endl;

      _missingValue = _ncVarPtr->get_att(_missingValueAttName.c_str())->as_float(0);
      if(_debug)
	cerr << "   Setting missing data value to: " << _missingValue << endl;
      _checkForMissing = true;
    }

  //initialize the value of _totalNumValues, using the NcVar pointer's num_vals() method
  _totalNumValues = _ncVarPtr->num_vals();

  _isInitialized = true;
  if(_debug)
    cerr << "    Variable initialized." << endl;
  
  return true;
}
  

/**********************************************************************
 * getValue() - take the recordNumber, return the ncVar value, doing any
 * units conversion with the Converter object
 */

double InputVariable::getValue(const int recordNumber)
{
  if (!_isInitialized)
    {
      cerr << "WARNING! Variable not initialized: " << _variableName << endl;
      return STATION_NAN;
    }

  if (recordNumber >= _totalNumValues)
    {
      cerr << "WARNING! Index of requested value is beyond the range of the total "
	   << "number of values for this variable." << endl;
      cerr << "      Requesting: <" << recordNumber << ">" << endl;
      cerr << "      Available number of values: <" << _totalNumValues << ">" << endl;
      return STATION_NAN;
    }

  double value = _ncVarPtr->as_float(recordNumber);

  if (_checkForMissing)
    if (value == _missingValue)
      return STATION_NAN;

  //return the result of convertValue() which does units conversion

  return _converter->convertValue(value);
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

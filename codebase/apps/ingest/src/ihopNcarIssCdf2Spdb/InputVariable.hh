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
 *   $Author: jcraig $
 *   $Locker:  $
 *   $Date: 2018/01/26 20:17:22 $
 *   $Id: InputVariable.hh,v 1.3 2018/01/26 20:17:22 jcraig Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * InputVariable.hh: class implementing InputVariables used with netCDF files
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2003
 *
 * Kay Levesque
 *
 ************************************************************************/

#ifndef InputVariable_HH
#define InputVariable_HH

#include <string>
#include <Ncxx/Nc3File.hh>
#include "Converter.hh"

using namespace std;


class InputVariable
{
 public:

  /*****************************************************************
   * Constructor
   */

  InputVariable(const string &variableName="", const bool debug = false);

  
  /*****************************************************************
   * Destructor
   */

  virtual ~InputVariable(void);
  
  /******************************************************************
   * Access methods
   */

  string getVariableName(void) const
  {
    return _variableName;
  }

  void setVariableName(const string &variableName)
  {
    _variableName = variableName;
  }

  void setMissingValueAttName(const string &attName)
  {
    _missingValueAttName = attName;
  }

  void setDebug(const bool debug)
  {
    _debug = debug;
  }

  //InputVariable object takes control of the converter pointer.
  //Calling method should not delete this pointer or do anything else
  //with this pointer after calling this method.

  void setConverter(Converter *converter)
  {
    delete _converter;
    _converter = converter;
  }

public:

  bool init(const Nc3File &fileObject);
  double getValue(const int recordNumber);

protected:

  bool _debug;
  bool _checkForMissing;
  double _missingValue;
  double _totalNumValues;
  string _variableName;
  string _missingValueAttName;
  Nc3Var *_ncVarPtr;
  Converter *_converter;

private:

  bool _isInitialized;
};


#endif

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
 *   $Date: 2016/03/04 02:22:10 $
 *   $Id: BackgroundField.hh,v 1.2 2016/03/04 02:22:10 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * BackgroundField: Class containing information for a background field.
 *
 * RAP, NCAR, Boulder CO
 *
 * Dec 2006
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef BackgroundField_H
#define BackgroundField_H

#include <iostream>
#include <string>

using namespace std;


class BackgroundField
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  BackgroundField(const string &url = "",
		  const string &field_name = "",
		  const int level_num = 0,
		  const bool debug_flag = false);
  
  /*********************************************************************
   * Destructor
   */

  ~BackgroundField();


  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * getUrl() - Retrieve the URL for this background field.
   */

  inline string getUrl() const
  {
    return _url;
  }


  /*********************************************************************
   * getFieldName() - Retrieve the field name for this background field.
   */

  inline string getFieldName() const
  {
    return _fieldName;
  }


  /*********************************************************************
   * getLevelNum() - Retrieve the level number for this background field.
   */

  inline int getLevelNum() const
  {
    return _levelNum;
  }


  /*********************************************************************
   * setUrl() - Set the URL for this background field.
   */

  inline void setUrl(const string &url)
  {
    _url = url;
  }


  /*********************************************************************
   * setFieldName() - Set the field name for this background field.
   */

  inline void setFieldName(const string &field_name)
  {
    _fieldName = field_name;
  }


  /*********************************************************************
   * setLevelNum() - Set the level number for this background field.
   */

  inline void setLevelNum(const int level_num)
  {
    _levelNum = level_num;
  }


  //////////////////////////
  // Input/output methods //
  //////////////////////////

  /*********************************************************************
   * print() - Write the object information to the indicated stream.
   */

  void print(ostream &out) const;


protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  
  string _url;
  string _fieldName;
  int _levelNum;
  
};

#endif

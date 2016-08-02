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
 *   $Id: VertProfile.hh,v 1.2 2016/03/04 02:22:13 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * VertProfile: Class containing the information for a single vertical
 *              profile.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2008
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef VertProfile_HH
#define VertProfile_HH

#include <string>
#include <vector>

using namespace std;

class VertProfile
{
 public:

  /*********************************************************************
   * Constructor
   */

  VertProfile(const string &field_name,
	      const bool debug_flag = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~VertProfile(void);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * addValue() - Add the given value to the profile.
   */

  void addValue(const double &value)
  {
    _values.push_back(value);
  }
  

  /*********************************************************************
   * getFieldName() - Get the field name for the profile.
   */

  string getFieldName() const
  {
    return _fieldName;
  }
  

  /*********************************************************************
   * getValues() - Get the profile values.
   */

  const vector< double > &getValues() const
  {
    return _values;
  }
  

 protected:

  bool _debug;
  
  string _fieldName;
  vector< double > _values;
  
};


#endif

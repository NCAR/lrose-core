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
 *   $Date: 2016/03/04 02:29:42 $
 *   $Id: IconDef.hh,v 1.2 2016/03/04 02:29:42 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * IconDef.hh : IconDef methods.  This class represents a stroked
 *              icon definition for a Symprod object.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2000
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef IconDef_HH
#define IconDef_HH

/*
 **************************** includes **********************************
 */

#include <string>
#include <vector>

#include <euclid/GridPoint.hh>
#include <Spdb/Symprod.hh>
using namespace std;

/*
 ******************************* defines ********************************
 */

/*
 ******************************* structures *****************************
 */

/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */


class IconDef
{      
public:

  // Constructors

  IconDef(const string &icon_name,
	  vector< GridPoint > &icon_points);

  IconDef(const IconDef &rhs);

  // Destructors

  ~IconDef();


  ////////////////////
  // Access methods //
  ////////////////////

  inline int getNumPoints(void) const
  {
    return _numPoints;
  }
  
  inline Symprod::ppt_t *getPointList(void) const
  {
    return _points;
  }
  

private:

  // Members

  string _iconName;
  int _numPoints;
  Symprod::ppt_t *_points;
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("IconDef");
  }
  
};

# endif     /* IconDef_HH */

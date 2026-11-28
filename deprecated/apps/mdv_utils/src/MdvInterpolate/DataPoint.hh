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
 *   $Date: 2016/03/04 02:22:11 $
 *   $Id: DataPoint.hh,v 1.2 2016/03/04 02:22:11 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DataPoint: Class representing a data point.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2004
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef DataPoint_HH
#define DataPoint_HH

#include <vector>

#include <toolsa/DateTime.hh>

#include "DataPoint.hh"

using namespace std;


class DataPoint
{
 public:

  /////////////////////////////
  // Constructors/destructor //
  /////////////////////////////

  /**********************************************************************
   * Constructor
   */

  DataPoint(const bool debug_flag = false);
  

  /**********************************************************************
   * Destructor
   */

  virtual ~DataPoint(void);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**********************************************************************
   * set() - Set the point information
   */

  void set(const double lat, const double lon, const double value)
  {
    _lat = lat;
    _lon = lon;
    _value = value;
  }
  

  /**********************************************************************
   * setLocation() - Set the point location
   */

  void setLocation(const double lat, const double lon)
  {
    _lat = lat;
    _lon = lon;
  }
  

  /**********************************************************************
   * getLocation() - Retrieve the location of the data point
   */

  void getLocation(float &lat, float &lon) const
  {
    lat = _lat;
    lon = _lon;
  }
  
  void getLocation(double &lat, double &lon) const
  {
    lat = _lat;
    lon = _lon;
  }
  

  /**********************************************************************
   * getLatitude() - Retrieve the latitude location of the data point.
   */

  double getLatitude() const
  {
    return _lat;
  }
  

  /**********************************************************************
   * getLongitude() - Retrieve the longitude location of the data point.
   */

  double getLongitude() const
  {
    return _lon;
  }
  

  /**********************************************************************
   * setValue() - Set the value of the data point.
   */

  void setValue(const double value)
  {
    _value = value;
  }
  

  /**********************************************************************
   * getValue() - Retrieve the value of the data point.
   */

  double getValue() const
  {
    return _value;
  }
  

protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;

  double _lat;
  double _lon;
  double _value;
  
};


#endif

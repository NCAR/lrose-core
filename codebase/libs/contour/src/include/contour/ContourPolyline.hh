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
/////////////////////////////////////////////////////////////
// ContourPolyline.hh
//
// Class representing a single line segment in a contour
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 2000
//
///////////////////////////////////////////////////////////////

#ifndef ContourPolyline_H
#define ContourPolyline_H

#include <iostream>
#include <vector>

#include <contour/ContourPoint.hh>

using namespace std;

class ContourPolyline
{
  
public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  // constructor

  ContourPolyline(const bool& debug_flag = false);

  // destructor
  
  ~ContourPolyline();


  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * clearPoints() - Delete all of the points in the polyline.
   */

  void clearPoints()
  {
    _points.erase(_points.begin(), _points.end());
  }
  

  /*********************************************************************
   * addPoint() - Add the given point to the polyline.
   */

  void addPoint(const ContourPoint &point)
  {
    _points.push_back(point);
  }
  

  void addPoint(const double x, const double y)
  {
    ContourPoint point(x, y);
    addPoint(point);
  }
  

  /*********************************************************************
   * getPoints() - Retrieve the vector of points in the polyline.
   *
   * NOTE: This method returns a pointer to the internal point vector
   * which should not be changed by the client.
   */

  const vector< ContourPoint >& getPoints() const
  {
    return _points;
  }
  

  ////////////////////
  // Output methods //
  ////////////////////

  /*********************************************************************
   * print() - Print the contents of the polyline object to the given
   *           stream.
   */

  void print(ostream &out) const;
  

  //////////////////////////
  // Comparison operators //
  //////////////////////////

  inline bool operator==(const ContourPolyline& rhs) const
  {
    if (this->_points.size() != rhs._points.size())
      return false;
    
    vector< ContourPoint >::const_iterator this_iter =
      this->_points.begin();
    vector< ContourPoint >::const_iterator rhs_iter =
      rhs._points.begin();
    
    for (;
	 this_iter != this->_points.end();
	 ++this_iter, ++rhs_iter)
    {
      if (*this_iter != *rhs_iter)
	return false;
    } /* endfor - this_iter, rhs_iter */
    
    return true;
  }
  

  inline bool operator<(const ContourPolyline& rhs) const
  {
    vector< ContourPoint >::const_iterator this_iter =
      this->_points.begin();
    vector< ContourPoint >::const_iterator rhs_iter =
      rhs._points.begin();
    
    for (;
	 this_iter != this->_points.end();
	 ++this_iter, ++rhs_iter)
    {
      if (rhs_iter == rhs._points.end())
	return false;
      
      if (*this_iter < *rhs_iter)
	return true;
      
      if (*this_iter > *rhs_iter)
	return false;
      
    } /* endfor - this_iter, rhs_iter */
    
    return false;
  }
  

  inline bool operator<=(const ContourPolyline& rhs) const
  {
    vector< ContourPoint >::const_iterator this_iter =
      this->_points.begin();
    vector< ContourPoint >::const_iterator rhs_iter =
      rhs._points.begin();
    
    for (;
	 this_iter != this->_points.end();
	 ++this_iter, ++rhs_iter)
    {
      if (rhs_iter == rhs._points.end())
	return false;
      
      if (*this_iter < *rhs_iter)
	return true;
      
      if (*this_iter > *rhs_iter)
	return false;
      
    } /* endfor - this_iter, rhs_iter */
    
    if (rhs_iter == rhs._points.end())
      return true;
    
    return false;
  }
  

  inline bool operator>(const ContourPolyline& rhs) const
  {
    return !(*this <= rhs);
  }
  

  inline bool operator>=(const ContourPolyline& rhs) const
  {
    return !(*this < rhs);
  }
  

protected:
  
private:

  /////////////////////
  // Private members //
  /////////////////////

  bool _debugFlag;
  
  vector< ContourPoint > _points;
  

  /////////////////////
  // Private methods //
  /////////////////////

};

#endif

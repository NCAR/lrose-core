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
// Basin.hh
//
// Class representing a watershed basin.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2000
//
///////////////////////////////////////////////////////////////

#ifndef Basin_H
#define Basin_H

#include <iostream>
#include <vector>

#include <euclid/point.h>
#include <euclid/WorldPoint2D.hh>
#include <hydro/BasinField.hh>
#include <Mdv/MdvxProj.hh>
#include <shapelib/shapefil.h>
using namespace std;


class Basin
{
  
public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  // constructor

  Basin(const bool debug_flag = false);

  // destructor
  
  ~Basin();

  // Load the basin information from the given shape file
  //
  // Returns true if the basin information was successfully loaded, false
  // otherwise.

  bool loadShapeInfo(const string shape_file_base,
		     const int shape_number,
		     const string basin_id_field = "");
  
  bool loadShapeInfo(const SHPHandle shape_handle,
		     const DBFHandle dbf_handle,
		     const int shape_number,
		     const string basin_id_field);
  

  /////////////////////
  // Utility methods //
  /////////////////////

  // Create a gridded mask representing the position of this basin on
  // the given projection.
  //
  // If successful, allocates space for the gridded mask and returns
  // a pointer to that mask.  The pointer must be freed by the caller
  // using ufree().  The grid will contain 0's for grid squares outside
  // of the basin and non-0 values for squares within the basin.
  //
  // Also returns the limits of the location of the basin within the
  // grid in X/Y indices so that you can speed things up by just operating
  // on the area of the grid covered by the basin.
  //
  // If not successful, returns 0.

  unsigned char *createMask(const MdvxProj &projection,
			    int &min_x, int &max_x,
			    int &min_y, int &max_y) const;
  

  // Determines whether the given point lies within the basin.
  //
  // Returns true if the point is within the basin, false otherwise.

  bool pointInBasin(const double lat, const double lon) const;
  

  // Determine the basin ID field name from the shape file base.

  static string getIdFieldFromShapeBase(const string shape_base);
  

  //////////////////////////
  // Input/output methods //
  //////////////////////////

  // Print the current basin information to the given stream for
  // debugging purposes.

  void print(ostream &stream,
	     const bool print_vertices = true) const;
  

  ////////////////////
  // Access methods //
  ////////////////////

  // Get the unique basin identifier

  int getId(void) const
  {
    return _id;
  }
  

  // Get a pointer to the perimeter of the basin.

  WorldPoint2D getOutflowLocation(void) const
  {
    return _perimeter[0];
  }
  

  // Get a pointer to the perimeter array used by the euclid library
  // routines.  Note that this returns a pointer to the actual member
  // so it shouldn't be changed or deleted by the caller.

  const Point_d *getEuclidPerimeter(void) const
  {
    return _euclidPerimeter;
  }
  
  // Get the number of vertices in the basin perimeter

  int getNumVertices(void) const
  {
    return _perimeter.size();
  }
  
  // Get a pointer to the field information for the basin.  This
  // pointer points directly to the member of this object so should
  // not be changed by the calling routine.

  const vector< BasinField > *getFieldList(void) const
  {
    return &_dbfInfo;
  }
  

protected:
  
private:

  /////////////////////
  // Private members //
  /////////////////////

  bool _debugFlag;
  
  // Flag indicating if the basin information was loaded successfully

  bool _infoLoaded;
  
  // Basin identifier.  This must be unique among the basins.

  int _id;
  
  // Basin perimeter information

  vector< WorldPoint2D > _perimeter;
  
  double _minLat;
  double _minLon;
  double _maxLat;
  double _maxLon;
  
  // Basin perimeter used for polygon calculations using the euclid library.
  // Note that these point values are in lat/lon.

  Point_d *_euclidPerimeter;
  
  // All of the database information for the basin

  vector< BasinField > _dbfInfo;
  

  /////////////////////
  // Private methods //
  /////////////////////

  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("Basin");
  }
  
};

#endif
